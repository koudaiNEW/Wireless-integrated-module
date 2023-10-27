#include "lpuart.h"
#include "gpio.h"
#include "sysctrl.h"
#include "uart_user.h"

const unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40
};

const  unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
0x41, 0x81, 0x80, 0x40
};

extern Timely_Data System_Data;

uint8_t LpUart_4G_Buffer[64] = {0x00};
uint8_t LpUart_BLE_Buffer[64] = {0x00};
uint8_t Valid_Data[64] = {0x00};  //服务器返回数据


//串口接收超时超时
void LpUartTimeOut(void)
{
    if(System_Data.LpUart_4G_Timeout>0)System_Data.LpUart_4G_Timeout--;   //接收时间超时
    else 
    {
        System_Data.LpUart_4G_Count = 0;                       //接收缓存地址偏移恢复
        System_Data.Valid_Data_Flag = 0;
        System_Data.Valid_Data_Count = 2;
    }
    if(System_Data.LpUart_BLE_Timeout>0)System_Data.LpUart_BLE_Timeout--;
    else System_Data.LpUart_BLE_Count=0;
}

//LPUART0中断函数，蓝牙
void LpUart0_IRQHandler(void)
{
    LPUart_ClrStatus(M0P_LPUART0, LPUartRC);         //<清接收中断请求
    System_Data.LpUart_BLE_Timeout = 2;   //接收超时20ms
    System_Data.LpUart_BLE_Busy = 1;         //串口忙碌置位
    System_Data.BLE_Idle_Timeout = 0;      //清蓝牙空闲计数
    LpUart_BLE_Buffer[System_Data.LpUart_BLE_Count] = LPUart_ReceiveData(M0P_LPUART0);  //读取数据
    System_Data.LpUart_BLE_Count++; 
    if(System_Data.LpUart_BLE_Count>63)System_Data.LpUart_BLE_Count = 0;
}

//LPUART1中断函数，4G
void LpUart1_IRQHandler(void)
{
    LPUart_ClrStatus(M0P_LPUART1, LPUartRC);         //<清接收中断请求
    System_Data.LpUart_4G_Timeout = 2;   //接收超时20ms
    // LpUart_4G_Buffer[LpUart_4G_Count] = LPUart_ReceiveData(M0P_LPUART0);  //读取数据
    // LpUart_4G_Count++;

    //服务器返回有效
    if(System_Data.Valid_Data_Flag == 1)  
    {
        Valid_Data[System_Data.Valid_Data_Count] = LPUart_ReceiveData(M0P_LPUART1);
        System_Data.Valid_Data_Count++;
        //判断帧结尾
        if((Valid_Data[System_Data.Valid_Data_Count - 2] == 0x55) && (Valid_Data[System_Data.Valid_Data_Count - 1] == 0xAA))
        {
            System_Data.Valid_Data_Ready = 1;  //返回数据接收完毕
            System_Data.Valid_Data_Flag = 0;
            System_Data.Valid_Data_Count = 2;
        }
        // if(Valid_Data_Count>=42)Valid_Data_Flag = 0;
    }
    else
    {
        LpUart_4G_Buffer[System_Data.LpUart_4G_Count] = LPUart_ReceiveData(M0P_LPUART1);  //读取数据
        System_Data.LpUart_4G_Count++;
        //判断是否为服务器返回数据
        if(LpUart_4G_Buffer[0]==0xA5&&LpUart_4G_Buffer[1]==0x5A)
        {
            System_Data.Valid_Data_Flag = 1;  //返回数据接收中
            System_Data.LpUart_4G_Count = 0;
            LpUart_4G_Buffer[0] = 0;
        }
    }

    if(System_Data.Valid_Data_Count>63)System_Data.Valid_Data_Count = 0;
    if(System_Data.LpUart_4G_Count>63)System_Data.LpUart_4G_Count = 0;
}

//lpuart1端口配置，4G串口
void App_LpUart1PortCfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    ///<TX
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin0,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin0,GpioAf2); //配置PA00为LPUART1_TX
    //<RX
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin1,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin1,GpioAf2); //配置PA01为LPUART1_RX
    //4G pwr
    stcGpioCfg.enDir = GpioDirOut;
    stcGpioCfg.bOutputVal = TRUE;
    Gpio_Init(GpioPortA,GpioPin2,&stcGpioCfg); 
}

//lpuart0端口配置，蓝牙串口
void App_LpUart0PortCfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    ///<TX
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortB,GpioPin10,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortB,GpioPin10,GpioAf4); //配置PB10为LPUART0_TX
    //<RX
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortB,GpioPin11,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortB,GpioPin11,GpioAf3); //配置PB11为LPUART0_RX
}

//lpuart1端口配置，4G串口去初始化
void App_LpUart1DeInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enPd = GpioPdEnable;  //下拉

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    //4G pwr
    Gpio_Init(GpioPortA,GpioPin2,&stcGpioCfg); 
    ///<TX
    Gpio_Init(GpioPortA,GpioPin0,&stcGpioCfg);
    //<RX
    stcGpioCfg.enPu = GpioPuEnable;  //上拉
    stcGpioCfg.enPd = GpioPdDisable;  //无下拉
    Gpio_Init(GpioPortA,GpioPin1,&stcGpioCfg);
}

///< LPUART1配置,4G
void App_LpUart1Cfg(unsigned char LpMode)
{
    stc_lpuart_cfg_t stcCfg;
    DDL_ZERO_STRUCT(stcCfg);
    ///<外设模块时钟使能
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart1,TRUE);    
    ///<LPUART 初始化
    stcCfg.enStopBit = LPUart1bit;                   ///<1停止位    
    stcCfg.enMmdorCk = LPUartDataOrAddr;                   ///<偶校验
    if(LpMode == LPMODE_POWER)
    {
        stcCfg.stcBaud.enSclkSel = LPUartMskPclk;        ///<传输时钟源
        stcCfg.stcBaud.u32Sclk = Sysctrl_GetPClkFreq();  ///<PCLK获取
        stcCfg.stcBaud.u32Baud = 115200;                   ///<波特率
    }
    else 
    {
        stcCfg.stcBaud.enSclkSel = LPUartMskRcl;        ///<传输时钟源
        stcCfg.stcBaud.u32Sclk = 38400;
        stcCfg.stcBaud.u32Baud = 9600;                   ///<波特率
    }
    stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;     ///<采样分频
    stcCfg.enRunMode = LPUartMskMode1;               ///<工作模式
    LPUart_Init(M0P_LPUART1, &stcCfg);
    ///<LPUART 中断使能
    LPUart_ClrStatus(M0P_LPUART1,LPUartRC);          ///<清接收中断请求
    LPUart_ClrStatus(M0P_LPUART1,LPUartTC);          ///<清发送中断请求
    LPUart_EnableIrq(M0P_LPUART1,LPUartRxIrq);       ///<使能接收中断
    LPUart_DisableIrq(M0P_LPUART1,LPUartTxIrq);      ///<禁止发送中断
    EnableNvic(LPUART1_IRQn,IrqLevel2,TRUE);         ///<系统中断使能

    Valid_Data[0] = 0xA5;
    Valid_Data[1] = 0x5A;
}

///< LPUART0配置，蓝牙
void App_LpUart0Cfg(unsigned char LpMode)
{
    stc_lpuart_cfg_t stcCfg;
    DDL_ZERO_STRUCT(stcCfg);
    ///<外设模块时钟使能
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpUart0,TRUE);    

    ///<LPUART 初始化
    stcCfg.enStopBit = LPUart1bit;                   ///<1停止位    
    stcCfg.enMmdorCk = LPUartDataOrAddr;                   ///<偶校验
    if(LpMode == LPMODE_POWER)
    {
        stcCfg.stcBaud.enSclkSel = LPUartMskPclk;        ///<传输时钟源
        stcCfg.stcBaud.u32Sclk = Sysctrl_GetPClkFreq();  ///<PCLK获取
        stcCfg.stcBaud.u32Baud = 57600;                   ///<波特率
    }
    else 
    {
        stcCfg.stcBaud.enSclkSel = LPUartMskRcl;        ///<传输时钟源
        stcCfg.stcBaud.u32Sclk = 38400;
        stcCfg.stcBaud.u32Baud = 9600;                   ///<波特率
    }
    stcCfg.stcBaud.enSclkDiv = LPUartMsk4Or8Div;     ///<采样分频
    stcCfg.enRunMode = LPUartMskMode1;               ///<工作模式
    LPUart_Init(M0P_LPUART0, &stcCfg);

    ///<LPUART 中断使能
    LPUart_ClrStatus(M0P_LPUART0,LPUartRC);          ///<清接收中断请求
    LPUart_ClrStatus(M0P_LPUART0,LPUartTC);          ///<清发送中断请求
    LPUart_EnableIrq(M0P_LPUART0,LPUartRxIrq);       ///<使能接收中断
    LPUart_DisableIrq(M0P_LPUART0,LPUartTxIrq);      ///<禁止发送中断
    EnableNvic(LPUART0_IRQn,IrqLevel1,TRUE);         ///<系统中断使能
}

//蓝牙4G串口初始化
void BleAnd4GLpUartInit(unsigned char LpUartNumber,unsigned char LpMode)
{
    if(LpUartNumber)
    {
        App_LpUart1PortCfg();
        App_LpUart1Cfg(LpMode);
    }
    else 
    {
        App_LpUart0PortCfg();
        App_LpUart0Cfg(LpMode);
    }
}

//CRC16 校验
unsigned short CRC16( unsigned char *puchMsg,  unsigned short usDataLen) 
{   
    unsigned char uchCRCHi = 0xFF ;  
    unsigned char uchCRCLo = 0xFF ; 
    unsigned char uIndex ; 
    while (usDataLen--) 
    {     
        uIndex = uchCRCHi ^ *puchMsg++ ; 
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;     
        uchCRCLo = auchCRCLo[uIndex] ;   
    }   
    return (uchCRCHi << 8 | uchCRCLo) ; 
}


