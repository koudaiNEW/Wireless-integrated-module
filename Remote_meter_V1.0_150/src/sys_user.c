#include "sysctrl.h"
#include "sys_user.h"
#include "lcd_user.h"
#include "gpio.h"

user_Data External_Data;
Timely_Data System_Data;
// unsigned char userTest1 = 0;

///< 干簧管中断服务函数
void PortB_IRQHandler(void)
{
    if(Gpio_GetIrqStatus(GpioPortB, GpioPin15))
    {
        Gpio_ClearIrq(GpioPortB, GpioPin15);  
        System_Data.LCD_Count = External_Data.LCD_WakeUp_Time;  //LCD显示定时
        if(!System_Data.LCD_Busy)
        {
            System_Data.LCD_Busy = 2;
        }
    }
    if(Gpio_GetIrqStatus(GpioPortB, GpioPin13))
    {
        Gpio_ClearIrq(GpioPortB, GpioPin13);  
        System_Data.LCD_Count = External_Data.LCD_WakeUp_Time;  //LCD显示定时
        if(!System_Data.LCD_Busy)
        {
            System_Data.LCD_Busy = 2;
        }
    }
}    

static void App_SysClkInit(void)
{
    stc_sysctrl_clk_cfg_t  stcClkCfg;

    //CLK INIT
    stcClkCfg.enClkSrc  = SysctrlClkRCH;
    stcClkCfg.enHClkDiv = SysctrlHclkDiv1;
    stcClkCfg.enPClkDiv = SysctrlPclkDiv1;
    Sysctrl_ClkInit(&stcClkCfg);

    //使能RCL
    Sysctrl_SetRCLTrim(SysctrlRclFreq38400);
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
}

//干簧管、按键初始化
static void App_SwitchInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    
    ///< 打开GPIO外设时钟门控
    // Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
    ///< 端口方向配置->输入
    stcGpioCfg.enDir = GpioDirIn;
    ///< 端口驱动能力配置->驱动能力
    stcGpioCfg.enDrv = GpioDrvL;
    ///< 端口上下拉配置->无
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdDisable;
    ///< 端口开漏输出配置->开漏输出关闭
    stcGpioCfg.enOD = GpioOdDisable;
    ///< 端口输入/输出值寄存器总线控制模式配置->AHB
    stcGpioCfg.enCtrlMode = GpioAHB;
    ///< GPIO IO USER KEY初始化
    Gpio_Init(GpioPortB, GpioPin15, &stcGpioCfg);    
    
    //干簧管IO设置，上升沿触发
    Gpio_EnableIrq(GpioPortB, GpioPin15, GpioIrqRising);
    //按键
    Gpio_EnableIrq(GpioPortB, GpioPin13, GpioIrqRising);
    EnableNvic(PORTB_IRQn, IrqLevel2, TRUE);
}

//按键初始化
// static void App_ButtonInit(void)
// {
//     stc_gpio_cfg_t stcGpioCfg;
//     DDL_ZERO_STRUCT(stcGpioCfg);
    
//     ///< 打开GPIO外设时钟门控
//     // Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
    
//     ///< 端口方向配置->输入
//     stcGpioCfg.enDir = GpioDirIn;
//     ///< 端口驱动能力配置->驱动能力
//     stcGpioCfg.enDrv = GpioDrvL;
//     ///< 端口上下拉配置->无
//     stcGpioCfg.enPu = GpioPuDisable;
//     stcGpioCfg.enPd = GpioPdDisable;
//     ///< 端口开漏输出配置->开漏输出关闭
//     stcGpioCfg.enOD = GpioOdDisable;
//     ///< 端口输入/输出值寄存器总线控制模式配置->AHB
//     stcGpioCfg.enCtrlMode = GpioAHB;
//     ///< GPIO IO USER KEY初始化
//     Gpio_Init(GpioPortB, GpioPin13, &stcGpioCfg);    
    
//     //按键IO设置，上升沿触发
//     Gpio_EnableIrq(GpioPortB, GpioPin13, GpioIrqRising);
//     EnableNvic(PORTB_IRQn, IrqLevel2, TRUE);
// }


void SysInit(void)
{
    App_SysClkInit();

    // Sysctrl_SetRCHTrim(SysctrlRchFreq4MHz);
    // while(!M0P_SYSCTRL->RCH_CR_f.STABLE);  //RCH稳定

    // Sysctrl_SetRCLTrim(SysctrlRclFreq38400);
    // M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;
    // M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    // M0P_SYSCTRL->SYSCTRL0_f.RCL_EN = 1;    //RCL使能
    // while(!M0P_SYSCTRL->RCL_CR_f.STABLE);  //RCL稳定
    // Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
    // userTest1 = sizeof(External_Data);

    ///< 打开GPIO外设时钟门控
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    //设置SWD端口为IO口
    // Sysctrl_SetFunc(SysctrlSWDUseIOEn, TRUE);
    ///< 配置为数字端口
    M0P_GPIO->PAADS = 0;
    M0P_GPIO->PBADS = 0;
    M0P_GPIO->PCADS = 0;
    M0P_GPIO->PDADS = 0;
    M0P_GPIO->PEADS = 0;
    M0P_GPIO->PFADS = 0;

    ///< 配置为端口输入
    M0P_GPIO->PADIR = 0XFFFF;
    M0P_GPIO->PBDIR = 0XFFFF;
    M0P_GPIO->PCDIR = 0XFFFF;
    M0P_GPIO->PDDIR = 0XFFFF;
    M0P_GPIO->PEDIR = 0XFFFF;
    M0P_GPIO->PFDIR = 0XFFFF;

    ///< 输入下拉
    M0P_GPIO->PAPD = 0xFFFF;
    M0P_GPIO->PBPD = 0xFFFF;
    M0P_GPIO->PCPD = 0xFFFF;
    M0P_GPIO->PDPD = 0xFFFF;
    M0P_GPIO->PEPD = 0xFFFF;
    M0P_GPIO->PFPD = 0xFFFF;

    //干簧管IO设置，上升沿触发
    App_SwitchInit();
    // Gpio_EnableIrq(GpioPortA, GpioPin11, GpioIrqHigh);
    // EnableNvic(PORTA_IRQn, IrqLevel2, TRUE);

    ///< 输入上拉
    // M0P_GPIO->PAPU = 0xFFFF;
    // M0P_GPIO->PBPU = 0xFFFF;
    // M0P_GPIO->PCPU = 0xFFFF;
    // M0P_GPIO->PDPU = 0xFFFF;
    // M0P_GPIO->PEPU = 0xFFFF;
    // M0P_GPIO->PFPU = 0xFFFF;
}
