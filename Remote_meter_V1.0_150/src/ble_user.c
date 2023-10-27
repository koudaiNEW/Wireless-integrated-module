#include "lpuart.h"
#include "gpio.h"
#include "adc.h"
#include "bgr.h"
#include "uart_user.h"
#include "sensor_user.h"
#include "flash_user.h"
#include "adc_user.h"
#include "iic_user.h"
#include "timer_user.h"

extern uint8_t LpUart_BLE_Buffer[];
extern user_Data External_Data;
extern Timely_Data System_Data;

unsigned char BLE_SendDataBuffer[128] = {0x00};

//蓝牙连接超时
void BLE_Link_Timeout(void)
{
    if(System_Data.BLE_Link_Flag)System_Data.BLE_Idle_Timeout++;
}

//蓝牙状态LED控制
static void BLE_LED_State(unsigned char state)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);

    if(state)
    {
        stcGpioCfg.enDir = GpioDirOut;
        stcGpioCfg.bOutputVal = TRUE;
    }
    else 
    {
        stcGpioCfg.enDir = GpioDirIn;
        stcGpioCfg.enPd = GpioPdEnable;  //下拉
    }
    Gpio_Init(GpioPortB,GpioPin14,&stcGpioCfg); //led
}

//唤醒BLE
void BLE_WAKE(void)
{
    Gpio_ClrIO(GpioPortB,GpioPin2); //wake
    delay1ms(1100);
    Gpio_SetIO(GpioPortB,GpioPin2); //wake
}

//清空缓存
static void BLE_Clear_Buffer(void)
{
    unsigned char i = 0;
    for(i=0;i<64;i++)LpUart_BLE_Buffer[i] = '\0';
}

//发送数据
static void BLE_SendData(unsigned char total,unsigned char *ble_cmd)
{
    unsigned char i = 0;
    // BLE_LED_Data(1);
    for(i=0;i<total;i++)LPUart_SendData(M0P_LPUART0,ble_cmd[i]);
    // BLE_LED_Data(0);
}

//返回标准判断
static void BLE_Judgment(unsigned short total, unsigned char *ble_cmd)
{
    unsigned char i = 0;
    unsigned char tryCount = 0;
    
    while(tryCount<2)
    {
        BLE_SendData(total,ble_cmd);  //指令
        delay1ms(300);
        while(i<62)
        {
            if(LpUart_BLE_Buffer[i]=='O'&&LpUart_BLE_Buffer[i+1]=='K')
            {
                goto clear;
            }
            i++;
        }
        i = 0;
        tryCount++;
    }

    clear:
    BLE_Clear_Buffer();
}

//BLE指令 切换命令行
void BLE_Start(void)
{
    unsigned char BLE_CMD_START[4] = "+++a";

    BLE_Judgment(4,BLE_CMD_START);  //切换到命令行
}

//BLE指令 重启
static void BLE_Restart(void)
{
    unsigned char j = 0;
    unsigned char tryCount = 0;
    
    unsigned char BLE_CMD_RE[6] = "AT+Z\r\n";

    while(tryCount<2)
    {
        BLE_SendData(6,BLE_CMD_RE);   //重启BLE模块
        delay1ms(500);
        while(j<62)
        {
            if(LpUart_BLE_Buffer[j]=='W'&&LpUart_BLE_Buffer[j+1]=='H')
            {
                goto clear;
            }
            j++;
        }
        j = 0;
        tryCount++;
    }

    clear:
    BLE_Clear_Buffer();
}

//BLE指令 更改波特率 重启
static void BLE_Uart(void)
{
    unsigned char BLE_CMD_UART[20] = "AT+UART=9600,8,0,0\r\n";

    BLE_Judgment(20,BLE_CMD_UART);   //设置串口9600

    BLE_Restart();   //重启BLE
}

//BLE指令 超低功耗模式
void BLE_DEEPSLEEP(void)
{
    unsigned char BLE_CMD_DEEPSLEEP[14] = "AT+DEEPSLEEP\r\n";

    BLE_Judgment(14,BLE_CMD_DEEPSLEEP);  //超低功耗模式
}

//BLE指令 模块名称更改 唤醒-命令模式-重启
void BLE_ID_Set(void)
{
	  unsigned char i = 0;
	
    unsigned char BLE_CMD_NAME[20] = "AT+NAME=";

    //唤醒BLE
    BLE_WAKE();

    //切换命令行
    BLE_Start();

    //Set BLE ID
    for(i=0;i<10;i++)
    {
        if(External_Data.ID[i] == 0x00)
        {
            BLE_CMD_NAME[i+8] = 0x30;
        }
        else
        {
            BLE_CMD_NAME[i+8] = External_Data.ID[i];
        }
    }
    BLE_CMD_NAME[18] = '\r';
    BLE_CMD_NAME[19] = '\n';

    BLE_Judgment(20,BLE_CMD_NAME);   //设置模块名称

    BLE_Restart();   //重启BLE
}

//BLE指令 更改从机模式 自动重启
static void BLE_SMode(void)
{
    unsigned char j = 0;
    unsigned char tryCount = 0;
    
    unsigned char BLE_CMD_MODE[11] = "AT+MODE=S\r\n";

    while(tryCount<2)
    {
        BLE_SendData(11,BLE_CMD_MODE);   //切换从机模式
        delay1ms(500);
        while(j<62)
        {
            if(LpUart_BLE_Buffer[j]=='W'&&LpUart_BLE_Buffer[j+1]=='H')
            {
                goto clear;
            }
            j++;
        }
        j = 0;
        tryCount++;
    }

    clear:
    BLE_Clear_Buffer();
}

//BLE指令 关闭密码配对
static void BLE_Passen(void)
{
    unsigned char BLE_CMD_PASSDIS[15] = "AT+PASSEN=OFF\r\n";

    BLE_Judgment(15,BLE_CMD_PASSDIS);   //关闭密码配对
}

//BLE指令 设置广播间隔5s 重启
static void BLE_ADPTIM(void)
{
    // unsigned char BLE_CMD_ADPTIM[15] = "AT+ADPTIM=500\r\n";
    unsigned char BLE_CMD_ADPTIM[14] = "AT+ADPTIM=10\r\n";

    BLE_Judgment(14,BLE_CMD_ADPTIM);   //设置广播间隔5s

    BLE_Restart();   //重启BLE
}

//BLE指令 退出命令模式
static void BLE_OutAT(void)
{
    unsigned char BLE_CMD_ENTM[9] = "AT+ENTM\r\n";

    BLE_Judgment(9,BLE_CMD_ENTM);   //退出命令模式
}

//BLE指令 查询模块连接状态
static unsigned char BLE_Link(void)
{
    unsigned char j = 0;
    unsigned char tryCount = 0;
    unsigned char link_status = 0;
    
    unsigned char BLE_CMD_LINK[10] = "AT+LINK?\r\n";

    while(tryCount<2)
    {
        BLE_SendData(10,BLE_CMD_LINK);   //查询连接
        delay1ms(200);
        while(j<62)
        {
            if(LpUart_BLE_Buffer[j]=='L'&&LpUart_BLE_Buffer[j+1]=='i'&&LpUart_BLE_Buffer[j+2]=='n'&&LpUart_BLE_Buffer[j+3]=='k')
            {
                tryCount = 2;
                break;
            }
            j++;
        }
        if(LpUart_BLE_Buffer[j+5]=='O'&&LpUart_BLE_Buffer[j+6]=='n')link_status = 1;
        else link_status = 0;
        j = 0;
        tryCount++;
    }
    BLE_Clear_Buffer();
    return link_status;
}

//BLE指令 设置模块发射功率 
static void BLE_TPL(void)
{
    unsigned char BLE_CMD_TPL[10] = "AT+TPL=9\r\n";

    BLE_Judgment(10,BLE_CMD_TPL);   //设置发射功率
}


//BLE初始化
void BLE_Init(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
    stcGpioCfg.bOutputVal = TRUE;
    Gpio_Init(GpioPortB,GpioPin2,&stcGpioCfg); //wake脚置高

    //LED开启
    BLE_LED_State(1);

    delay1ms(1000);
    // Gpio_Init(GpioPortA,GpioPin6,&stcGpioCfg); //nRst
    // Gpio_Init(GpioPortA,GpioPin3,&stcGpioCfg); //nReload

    //切换命令行
    BLE_Start();

    //更改波特率
    BLE_Uart();

    //BLE串口重新初始化
    BleAnd4GLpUartInit(LP_BLE,LPMODE_USUAL);            

    //切换命令行
    BLE_Start();

    //从机模式
    BLE_SMode();

    //更改名称
    BLE_ID_Set();

    //切换命令行
    BLE_Start();

    //关闭密码配对
    BLE_Passen();

    //设置发射功率
    BLE_TPL();

    //广播间隔10s
    BLE_ADPTIM();

    //切换命令行
    BLE_Start();

    //低功耗模式
    BLE_DEEPSLEEP();

    // delay1ms(500);

    //LED关闭
    BLE_LED_State(0);

    System_Data.LpUart_BLE_Busy = 0;
}

//BLE读设备信息
static void BLE_SysData_Read(void)
{
    unsigned char i = 0;

    System_Data.ADBusy = 1;
    //采集电池电压
    ADC_User_Start();  ///< 启动单次转换采样
    //帧头
    BLE_SendDataBuffer[0] = 0xA5;
    BLE_SendDataBuffer[1] = 0x5A;
    //数据类型
    BLE_SendDataBuffer[2] = 0x01;
    //软硬件版本号
    BLE_SendDataBuffer[3] = External_Data.Version_Software;
    BLE_SendDataBuffer[4] = External_Data.Version_Hardware;
    //ID与IMEI、IMSI
    for(i=0;i<10;i++)BLE_SendDataBuffer[5+i] = External_Data.ID[i];
    for(i=0;i<15;i++)BLE_SendDataBuffer[15+i] = External_Data.IMEI[i];
    for(i=0;i<15;i++)BLE_SendDataBuffer[30+i] = External_Data.IMSI[i];
    //唤醒屏幕上传有效标志位
    BLE_SendDataBuffer[45] = External_Data.LCD_UpData_Flag;
    //屏幕唤醒时间
    BLE_SendDataBuffer[46] = External_Data.LCD_WakeUp_Time >> 8;
    BLE_SendDataBuffer[47] = External_Data.LCD_WakeUp_Time;
    //记录起始UTC
    BLE_SendDataBuffer[49] = External_Data.UTC>>24;
    BLE_SendDataBuffer[50] = External_Data.UTC>>16;
    BLE_SendDataBuffer[51] = External_Data.UTC>>8;
    BLE_SendDataBuffer[52] = External_Data.UTC;
    //记录间隔
    BLE_SendDataBuffer[53] = External_Data.Sensor_Interval>>8;
    BLE_SendDataBuffer[54] = External_Data.Sensor_Interval;
    //目标记录次数
    BLE_SendDataBuffer[55] = External_Data.Sensor_ScheduledTimes>>8;
    BLE_SendDataBuffer[56] = External_Data.Sensor_ScheduledTimes;
    //传感器类型
    BLE_SendDataBuffer[57] = External_Data.Sensor_Type;
    //量程下限
    BLE_SendDataBuffer[58] = External_Data.Sensor_Min>>8;
    BLE_SendDataBuffer[59] = External_Data.Sensor_Min;
    //量程上限
    BLE_SendDataBuffer[60] = External_Data.Sensor_Max>>8;
    BLE_SendDataBuffer[61] = External_Data.Sensor_Max;
    //IP地址1
    for(i=0;i<4;i++)BLE_SendDataBuffer[62+i] = External_Data.Address_IP1[i];
    BLE_SendDataBuffer[66] = External_Data.port1>>8;
    BLE_SendDataBuffer[67] = External_Data.port1;
    //IP地址2
    for(i=0;i<4;i++)BLE_SendDataBuffer[68+i] = External_Data.Address_IP2[i];
    BLE_SendDataBuffer[72] = External_Data.port2>>8;
    BLE_SendDataBuffer[73] = External_Data.port2;
    //帧尾
    BLE_SendDataBuffer[74] = 0x55;
    BLE_SendDataBuffer[75] = 0xAA;
    //等待电池电压采集
    i = 0;
    while(i<30)
    {
        if(System_Data.ADBusy==0)break;
        i++;
    }
    BLE_SendDataBuffer[48] = External_Data.Battery_Level;

    BLE_SendData(76,BLE_SendDataBuffer);
}

//BLE读压力
static void BLE_PData_Read(void)
{
    SensorIICInit();
    BLE_SendDataBuffer[0] = 0xA5;
    BLE_SendDataBuffer[1] = 0x5A;
    BLE_SendDataBuffer[2] = 0x02;
    System_Data.BLE_PData.i16 = GetPTValue(PMODE); 
    SensorIICDeInit();
    BLE_SendDataBuffer[3] = System_Data.BLE_PData.u16>>8;
    BLE_SendDataBuffer[4] = System_Data.BLE_PData.u16;
    BLE_SendDataBuffer[5] = 0x55;
    BLE_SendDataBuffer[6] = 0xAA;

    BLE_SendData(7,BLE_SendDataBuffer);
}

//BLE写设备ID
static void BLE_ID_Write(void)
{
    unsigned char i = 0;

    for(i=0;i<10;i++)External_Data.ID[i] = LpUart_BLE_Buffer[3+i];
    External_Data.ID[10] = 1;  //用户修改ID标志
    BLE_ID_Set();
    Flash_User_Write();
}

//BLE写数据记录间隔、记录次数
static void BLE_AutoTime_Write(void)
{
    External_Data.Sensor_Interval = (unsigned short)LpUart_BLE_Buffer[3]<<8;
    External_Data.Sensor_Interval |= LpUart_BLE_Buffer[4];
    External_Data.Sensor_ScheduledTimes = (unsigned short)LpUart_BLE_Buffer[5]<<8;
    External_Data.Sensor_ScheduledTimes |= LpUart_BLE_Buffer[6];
    Flash_User_Write();
}

//BLE写IP1与端口
static void BLE_IP1_Write(void)
{
    unsigned char i = 0;

    for(i=0;i<4;i++)External_Data.Address_IP1[i] = LpUart_BLE_Buffer[3+i];
    External_Data.port1 = ((unsigned short)LpUart_BLE_Buffer[7]<<8)|LpUart_BLE_Buffer[8];
    Flash_User_Write();
}

//BLE写IP2与端口
static void BLE_IP2_Write(void)
{
    unsigned char i = 0;

    for(i=0;i<4;i++)External_Data.Address_IP2[i] = LpUart_BLE_Buffer[3+i];
    External_Data.port2 = ((unsigned short)LpUart_BLE_Buffer[7]<<8)|LpUart_BLE_Buffer[8];
    Flash_User_Write();
}

//BLE写传感器类型、量程
static void BLE_Sensor_Write(void)
{
    External_Data.Sensor_Type = LpUart_BLE_Buffer[3];
    External_Data.Sensor_Min = ((unsigned short)LpUart_BLE_Buffer[4]<<8)|LpUart_BLE_Buffer[5];
    External_Data.Sensor_Max = ((unsigned short)LpUart_BLE_Buffer[6]<<8)|LpUart_BLE_Buffer[7];
    Flash_User_Write();
}

//BLE写清零偏置
static void BLE_ZeroOffset_Write(void)
{
    External_Data.Zero_Offset += System_Data.BLE_PData.i16;
    Flash_User_Write();
}

//BLE写唤醒屏幕上传标志
static void BLE_LCD_Flag_Write(void)
{
    External_Data.LCD_UpData_Flag = LpUart_BLE_Buffer[3];
    Flash_User_Write();
}

//BLE写屏幕唤醒时间
static void BLE_LCD_WakeUpTime_Write(void)
{
    unsigned short temp = 0;

    temp = (unsigned short)LpUart_BLE_Buffer[3] << 8 | LpUart_BLE_Buffer[4];
    if(temp < 3)
    {
        temp = 3;
    }
    External_Data.LCD_WakeUp_Time = temp;
    Flash_User_Write();
}

//BLE返回指令类型错误
static void BLE_ERROR_Return(void)
{
    unsigned char BLE_ERROR[13] = "BLE_CMD_ERROR";

    BLE_SendData(13,BLE_ERROR);
}


//蓝牙数据处理
void BLE_Analysis(void)
{
    if(System_Data.LpUart_BLE_Busy==1 && System_Data.LpUart_BLE_Timeout==0)
    {
        //蓝牙连接标志
        System_Data.BLE_Link_Flag = 1;
        //LED开启
        BLE_LED_State(1);
        //非协议内容
        if(!(LpUart_BLE_Buffer[0]==0xA5&&LpUart_BLE_Buffer[1]==0x5A))
        {
            System_Data.LpUart_BLE_Busy = 0;
            BLE_Clear_Buffer();
            return;
        }
        //解析
        switch (LpUart_BLE_Buffer[2])
        {
        case 1:  //读取设备信息
            BLE_SysData_Read();
            break;

        case 2:  //读压力
            BLE_PData_Read();
            break;

        case 3:  //写设备ID
            BLE_ID_Write();
            break;

        case 4:  //写数据记录间隔、记录次数
            BLE_AutoTime_Write();
            break;

        case 5:  //写IP1与端口
            BLE_IP1_Write();
            break;

        case 6:  //写IP2与端口
            BLE_IP2_Write();
            break;

        case 7:  //写传感器信息
            BLE_Sensor_Write();
            break;

        case 8:  //关闭蓝牙连接
            delay1ms(200);
            //切换命令行
            BLE_Start();
            //低功耗模式
            BLE_DEEPSLEEP();
            //LED关闭
            BLE_LED_State(0);
            System_Data.BLE_Link_Flag = 0;
            System_Data.NET_Send_Type = 2;  //蓝牙关闭发送
            break;

        case 9:  //写清零偏置
            BLE_ZeroOffset_Write();
            break;

        case 10:  //恢复出厂设置
            Default_Setting(0);
            //设置ID
            BLE_ID_Set();
            //切换命令行
            BLE_Start();
            //低功耗模式
            BLE_DEEPSLEEP();
            //LED关闭
            BLE_LED_State(0);
            System_Data.BLE_Link_Flag = 0;
            break;

        case 11:  //写唤醒屏幕上传标志
            BLE_LCD_Flag_Write();
            break;

        case 12:  //写屏幕唤醒时间
            BLE_LCD_WakeUpTime_Write();
            break;

        default:  //未能解析帧数据类型，返回类型错误
            BLE_ERROR_Return();
            break;
        }

        System_Data.LpUart_BLE_Busy = 0;

        BLE_Clear_Buffer();
    }
    //蓝牙空闲超时,180s
    if(System_Data.BLE_Link_Flag)
    {
        if(System_Data.BLE_Idle_Timeout>18)
        {
            //切换命令行
            BLE_Start();
            if(BLE_Link())            //查询蓝牙是否保持连接
            {
                BLE_OutAT();
            }
            else 
            {
                BLE_DEEPSLEEP();      //蓝牙未连接进入低功耗模式
                //LED关闭
                BLE_LED_State(0);
                System_Data.BLE_Link_Flag = 0;
                System_Data.NET_Send_Type = 2;  //蓝牙关闭发送
            }
            System_Data.LpUart_BLE_Busy = 0;
        }
    }
}

