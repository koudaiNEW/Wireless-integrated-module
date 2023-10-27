#include "flash.h"
#include "spi.h"
#include "gpio.h"
#include "reset.h"
#include "flash_user.h"
#include "sensor_user.h"

extern user_Data External_Data;
extern Timely_Data System_Data;

// //初始化SPI1引脚
// static void App_GpioInit(void)
// {
//     stc_gpio_cfg_t GpioInitStruct;
//     DDL_ZERO_STRUCT(GpioInitStruct);
    
//     Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    
//     ///< SPI1引脚配置:主机
//     GpioInitStruct.enDrv = GpioDrvH;
//     GpioInitStruct.enDir = GpioDirOut;   

//     Gpio_Init(STK_SPI1_CS_PORT, STK_SPI1_CS_PIN, &GpioInitStruct);
//     Gpio_SetAfMode(STK_SPI1_CS_PORT, STK_SPI1_CS_PIN, GpioAf1);             ///<配置SPI1_CS
                                                               
//     Gpio_Init(STK_SPI1_SCK_PORT, STK_SPI1_SCK_PIN, &GpioInitStruct);            
//     Gpio_SetAfMode(STK_SPI1_SCK_PORT, STK_SPI1_SCK_PIN, GpioAf1);           ///<配置SPI1_SCK
                                                               
//     Gpio_Init(STK_SPI1_MOSI_PORT, STK_SPI1_MOSI_PIN, &GpioInitStruct);           
//     Gpio_SetAfMode(STK_SPI1_MOSI_PORT, STK_SPI1_MOSI_PIN, GpioAf1);         ///<配置SPI1_MOSI
                                                               
//     GpioInitStruct.enDir = GpioDirIn;                          
//     Gpio_Init(STK_SPI1_MISO_PORT, STK_SPI1_MISO_PIN, &GpioInitStruct);            
//     Gpio_SetAfMode(STK_SPI1_MISO_PORT, STK_SPI1_MISO_PIN, GpioAf1);         ///<配置SPI1_MISO
// }

// //初始化SPI1
// static void App_SPIInit(void)
// {
//     stc_spi_cfg_t  SpiInitStruct;    
    
//     ///< 打开外设时钟
//     Sysctrl_SetPeripheralGate(SysctrlPeripheralSpi0,TRUE);
    
//     ///<复位模块
//     Reset_RstPeripheral0(ResetMskSpi1);
    
//     //SPI0模块配置：主机
//     SpiInitStruct.enSpiMode = SpiMskMaster;     //配置位主机模式
//     SpiInitStruct.enPclkDiv = SpiClkMskDiv128;    //波特率：PCLK/2
//     SpiInitStruct.enCPHA    = SpiMskCphafirst;  //第一边沿采样
//     SpiInitStruct.enCPOL    = SpiMskcpollow;    //极性为低
//     Spi_Init(M0P_SPI1, &SpiInitStruct);
// }

// //Flash SPI初始化
// void Flash_SPI_Init(void)
// {
//     App_GpioInit();
//     App_SPIInit();
//     // Spi_SetCS(M0P_SPI1, FALSE);   //片选，开始
//     Spi_SetCS(M0P_SPI1, TRUE);    //片选，结束
// }

//写相关
void Flash_User_Write(void)
{
    unsigned char i = 0;
    unsigned long flash_addr = Flash_Address_UserData;
    temp_Flash user_ToFlash;

    user_ToFlash.won = External_Data;
    Flash_SectorErase(Flash_Address_UserData);
    for(i=0;i<21;i++)
    {
        Flash_WriteWord(flash_addr,user_ToFlash.toFlash[i]);
        flash_addr += 4;
    }
}

//读相关
void Flash_User_Read(void)
{
    unsigned char i = 0;
    unsigned long flash_addr = Flash_Address_UserData;

    temp_Flash user_ToFlash;
    for(i=0;i<21;i++)
    {
        user_ToFlash.toFlash[i] = *((volatile unsigned long *)flash_addr);
        flash_addr += 4;
    }
    External_Data = user_ToFlash.won;
    External_Data.Sensor_ScanCount = 0;  //扫描次数清零
}

//写默认设置
void Default_Setting(unsigned char mode)
{
    unsigned char i = 0;

    External_Data.Address_IP1[0] = 0;  //默认服务器IP
    External_Data.Address_IP1[1] = 0;
    External_Data.Address_IP1[2] = 0;
    External_Data.Address_IP1[3] = 0;
    External_Data.port1 = 0;           //默认端口

    External_Data.Address_IP2[0] = 0;  //默认服务器IP
    External_Data.Address_IP2[1] = 0;
    External_Data.Address_IP2[2] = 0;
    External_Data.Address_IP2[3] = 0;
    External_Data.port2 = 0;           //默认端口

    External_Data.Sensor_ScanCount = 0;    //默认采集次数0
    External_Data.Sensor_Interval = 300;    //默认扫描间隔300s
    External_Data.Sensor_ScheduledTimes = 72;    //默认6h传输一次
    //TEST
    // External_Data.Sensor_Interval = 60;    //默认扫描间隔60s
    // External_Data.Sensor_ScheduledTimes = 3;    //默认5min传输一次


    // External_Data.Version_Software = 1;          //默认软件版本号1
    // External_Data.Version_Software = 2;          //默认软件版本号2
    External_Data.Version_Software = 3;          //默认软件版本号3
    External_Data.Version_Hardware = 1;          //默认硬件版本号1

    External_Data.Sensor_Type = 1;         //传感器型号

    External_Data.LCD_UpData_Flag = 1;     //默认熄灭LCD上传开启

    External_Data.LCD_WakeUp_Time = 3;    //默认LCD点亮时间30s

    //量程150kPa
    External_Data.Sensor_Min = 0;          //量程下限
    External_Data.Sensor_Max = 150 * 100;      //量程上限

    // //量程2500kPa
    // External_Data.Sensor_Min = 0;          //量程下限
    // External_Data.Sensor_Max = 2500;      //量程上限

    GetK();

    External_Data.Zero_Offset = 0;         //默认偏移量0

    if(mode)
    {
        for(i=0;i<15;i++)External_Data.IMEI[i] = External_Data.IMSI[i] = '0';  //默认IMEI与IMSI
    }

    External_Data.ID[0] = 'J';                   //默认ID
    External_Data.ID[1] = 'B';
    External_Data.ID[2] = 'W';
    External_Data.ID[3] = 'L';
    for(i=4;i<10;i++)
    {
        External_Data.ID[i] = External_Data.IMEI[i + 5];
    }
    External_Data.ID[10] = 0;  //默认用户修改ID标志

    Flash_User_Write();
    Flash_SectorErase(Flash_Address_PowerOn);
    Flash_WriteByte(Flash_Address_PowerOn,171u);
}

//上电读Flash
void Flash_Power_Init(void)
{
    //初始化Flash
    while(Ok != Flash_Init(1, TRUE));
    //上电读上电标志位
    if((*((volatile unsigned char *)Flash_Address_PowerOn))!=171)
    {
        Default_Setting(1);
    }
    //读用户数据
    Flash_User_Read();
    //读上一次IP连接标志
    // if((*((volatile unsigned char *)Flash_Address_Ethernet))==171)
    // {
    //     External_Data.Sensor_ScanCount = External_Data.Sensor_ScheduledTimes;
    //     System_Data.ADBusy = 1;
    //     // Flash_SectorErase(Flash_Address_Ethernet);
    // }
}

//写压力数据Flash，16Bit，记满256次换一个扇区
void Flash_PData_Write(unsigned short PData)
{
    if(External_Data.Sensor_ScanCount > 32768)return;
    if(External_Data.Sensor_ScanCount % 256 == 0 || External_Data.Sensor_ScanCount == 1)Flash_SectorErase(Flash_Address_ScanData + External_Data.Sensor_ScanCount / 256 * 512);
    Flash_WriteHalfWord(Flash_Address_ScanData + (External_Data.Sensor_ScanCount - 1) * 2,PData);
}

//读压力数据
unsigned short Flash_PData_Read(unsigned short number)
{
    unsigned short PData = *((volatile unsigned short *)(Flash_Address_ScanData + (number - 1) * 2));
    if(number > 32768)return 0xFFFF;
    return PData;
}

