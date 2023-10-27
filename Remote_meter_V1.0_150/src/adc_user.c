#include "adc.h"
#include "gpio.h"
#include "bgr.h"
#include "lpuart.h"

extern user_Data External_Data;
extern Timely_Data System_Data;

unsigned long batteryAD[10] = {0x00};

static void App_AdcPortDeInit(void);

 ///< ADC 中断服务程序
void Adc_IRQHandler(void)
{    
    unsigned char i = 0;
    unsigned long batteryAD_OK = 0;

    Adc_ClrIrqStatus(AdcMskIrqSgl);       ///< 清除中断标志位
    batteryAD[System_Data.ADCount] = Adc_GetSglResult();   ///< 获取采样值
    System_Data.ADCount++;
    if(System_Data.ADCount>9)
    {
        System_Data.ADCount = 0;
        for(i=0;i<10;i++)batteryAD_OK += batteryAD[i];
        batteryAD_OK /= 10; 
        External_Data.Battery_Level = (unsigned long)(batteryAD_OK/4095.0*1.5*100);
        System_Data.ADBusy = 0;
        Adc_SGL_Stop();                       ///< ADC 单次转换停止
        Bgr_BgrDisable();   //BGR关闭
        Adc_Disable();    //ADC关闭
        // App_AdcPortDeInit(); //ADC引脚去初始化
        // ADBusy = 1;
        // Adc_SGL_Start();
    }
    else Adc_SGL_Start();       ///< 启动单次转换采样
}

///< ADC 采样端口初始化
void App_AdcPortInit(void)
{    
    ///< 开启 GPIO外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
    Gpio_SetAnalogMode(GpioPortA, GpioPin4);        //PA04 (AIN4)
}

///< ADC 采样端口去初始化
static void App_AdcPortDeInit(void)
{    
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    ///< 开启 GPIO外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    //设置为数字端口
    M0P_GPIO->PAADS = 0;

    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enPd = GpioPdEnable;  //下拉
    // stcGpioCfg.enPu = GpioPuEnable;  //上拉

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    //ADC
    Gpio_Init(GpioPortA,GpioPin4,&stcGpioCfg);
}

///< ADC模块 初始化
void App_AdcInit(void)
{
    stc_adc_cfg_t              stcAdcCfg;

    DDL_ZERO_STRUCT(stcAdcCfg);
    
    ///< 开启ADC/BGR 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE); 
    
    Bgr_BgrEnable();        ///< 开启BGR
    Adc_Enable();
    delay1ms(10);
    
    ///< ADC 初始化配置
    stcAdcCfg.enAdcMode         = AdcSglMode;               ///<采样模式-单次
    stcAdcCfg.enAdcClkDiv       = AdcMskClkDiv1;            ///<采样分频-1
    stcAdcCfg.enAdcSampCycleSel = AdcMskSampCycle12Clk;     ///<采样周期数-12
    stcAdcCfg.enAdcRefVolSel    = AdcMskRefVolSelInBgr1p5;  ///<参考电压选择-内部1.5V
    stcAdcCfg.enAdcOpBuf        = AdcMskBufDisable;         ///<OP BUF配置-关
    stcAdcCfg.enInRef           = AdcMskInRefEnable;        ///<内部参考电压使能-开
    stcAdcCfg.enAdcAlign        = AdcAlignRight;            ///<转换结果对齐方式-右
    Adc_Init(&stcAdcCfg);
}

///< ADC 单次扫描模式 配置
void App_AdcSglCfg(void)
{
    ///< ADC 采样通道配置
    Adc_CfgSglChannel(AdcExInputCH4);
    
    ///< ADC 中断使能
    Adc_EnableIrq();
    EnableNvic(ADC_DAC_IRQn, IrqLevel3, TRUE);
    
    ///< 启动单次转换采样
    // Adc_SGL_Start();   

}

//电池电量初始化
void Battery_Init(void)
{
    App_AdcPortInit();
    App_AdcInit();
    App_AdcSglCfg();
    System_Data.ADBusy = 1;
}

//ADC开始扫描
void ADC_User_Start(void)
{
    // App_AdcPortInit();
    Bgr_BgrEnable();
    Adc_Enable();
    delay1ms(50);
    Adc_SGL_Start();       ///< 启动单次转换采样
}

//电池电量扫描
void Battery_Scan(void)
{
    if(System_Data.LCD_Busy)
    {
        System_Data.ADBusy = 1;
    }
    // ADBusy = 0;  //Test
    if(System_Data.ADBusy)
    {
        ADC_User_Start();
    }
}

