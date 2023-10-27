#include "lpuart.h"
#include "gpio.h"
#include "ethernet_user.h"
#include "uart_user.h"
#include "ble_user.h"
#include "flash_user.h"
#include "timer_user.h"
#include "flash.h"
#include "adc_user.h"
#include "sensor_user.h"
#include "lcd_user.h"

extern user_Data External_Data;
extern Timely_Data System_Data;
extern uint8_t Valid_Data[];
extern uint8_t LpUart_4G_Buffer[];

unsigned char Ethernet_SendData[1024] = {0x00};  //4G发送缓存

//4G LED控制
static void Ethernet_LED_State(unsigned char state)
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
    Gpio_Init(GpioPortB,GpioPin8,&stcGpioCfg); //led
}

//整型数转字符串
static unsigned char Number_To_Char(unsigned short data, unsigned char* address)
{
    unsigned char length = 0;
    if(data<10)
    {
        length = 1;
        *address = 0x30+data;
    }
    else if(data<100)
    {
        length = 2;
        *address = 0x30+data/10;
        *(address+1) = 0x30+data%10;
    }
    else if(data<1000)
    {
        length = 3;
        *address = 0x30+data/100;
        *(address+1) = 0x30+data/10%10;
        *(address+2) = 0x30+data%10;
    }
    else if(data<10000)
    {
        length = 4;
        *address = 0x30+data/1000;
        *(address+1) = 0x30+data/100%10;
        *(address+2) = 0x30+data/10%10;
        *(address+3) = 0x30+data%10;
    }
    else
    {
        length = 5;
        *address = 0x30+data/10000;
        *(address+1) = 0x30+data/1000%10;
        *(address+2) = 0x30+data/100%10;
        *(address+3) = 0x30+data/10%10;
        *(address+4) = 0x30+data%10;
    }
    return length;
}

//发送数据
static void Ethernet_SendDataFun(unsigned short total,unsigned char *Ethernet_cmd)
{
    unsigned short i = 0;

    while(i<total)
    {
        LPUart_SendData(M0P_LPUART1,Ethernet_cmd[i]);
        i++;
    }
}

//清空缓存
static void Ethernet_Clear_Buffer(void)
{
    unsigned char i = 0;
    for(i=0;i<64;i++)LpUart_4G_Buffer[i] = 0;
}

//返回标准判断
static void Ethernet_Judgment(unsigned short total, unsigned char *Ethernet_cmd)
{
    unsigned char i = 0;
    // unsigned char tryCount = 0;
    
    // while(tryCount<2)
    // {
    //     Ethernet_SendDataFun(total,Ethernet_cmd);  //指令
    //     delay1ms(300);
    //     while(i<62)
    //     {
    //         if(LpUart_4G_Buffer[i]=='O'&&LpUart_4G_Buffer[i+1]=='K')
    //         {
    //             goto clear;
    //         }
    //         i++;
    //     }
    //     i = 0;
    //     tryCount++;
    // }
    // clear:
    // Ethernet_Clear_Buffer();

    if(!System_Data.Delay_Flag)
    {
        Ethernet_SendDataFun(total,Ethernet_cmd);  //指令
    }
    System_Data.Delay_Flag = 1;

    if(System_Data.User_Delay_Count / 500)
    {
        System_Data.User_Delay_Count -= 500;
        System_Data.Delay_Through++;
        while(i<62)
        {
            if(LpUart_4G_Buffer[i]=='O'&&LpUart_4G_Buffer[i+1]=='K')
            {
                System_Data.Delay_Through = 2;
                break;
            }
            i++;
        }

        if(System_Data.Delay_Through > 1)
        {
            System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
        }
        else
        {
            Ethernet_SendDataFun(total,Ethernet_cmd);  //指令
        }
    }
}

//NET指令 等待模块开机
static void Ethernet_Power_On(void)
{
    unsigned char i = 0;
    // unsigned char tryCount = 0;

    // while(tryCount<20)
    // {
    //     delay1ms(100);
    //     while(i<61)
    //     {
    //         if(LpUart_4G_Buffer[i]=='R'&&LpUart_4G_Buffer[i+1]=='D'&&LpUart_4G_Buffer[i+2]=='Y')
    //         {
    //             goto clear;
    //         }
    //         i++;
    //     }
    //     i = 0;
    //     tryCount++;
    // }
    // clear:
    // Ethernet_Clear_Buffer();

    System_Data.Delay_Flag = 1;   
    if(System_Data.User_Delay_Count / 100)
    {
        System_Data.User_Delay_Count -= 100;
        System_Data.Delay_Through++;
        while(i<61)
        {
            if((LpUart_4G_Buffer[i] == 'R' && LpUart_4G_Buffer[i+1] == 'D' && LpUart_4G_Buffer[i+2] == 'Y') || System_Data.Delay_Through > 30)
            {
                System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
                break;
            }
            i++;
        }
    }

}

//NET指令 更改波特率9600
static void Ethernet_Uart(void)
{
    unsigned char Ethernet_CMD_BPS[13] = "AT+IPR=9600\r\n";

    Ethernet_Judgment(13,Ethernet_CMD_BPS);  //更改波特率9600
}

//NET指令 查询IMEI号
static void Ethernet_IMEI(void)
{
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char tryCount = 0;

    unsigned char Ethernet_CMD_IMEI[11] = "AT+CGSN=1\r\n";

    while(tryCount<2)
    {
        Ethernet_SendDataFun(11,Ethernet_CMD_IMEI);  //查询IMEI号
        delay1ms(300);
        while(j<62)
        {
            if(LpUart_4G_Buffer[j]=='O'&&LpUart_4G_Buffer[j+1]=='K')
            {
                i = j;
                for(j=0;j<15;j++)External_Data.IMEI[j] = LpUart_4G_Buffer[i-20+j];
                if(External_Data.ID[10]!=1)   //用户修改ID标志位
                {
                    //修改ID
                    for(i=0;i<6;i++)External_Data.ID[i+4] = External_Data.IMEI[i+9];
                }
                //写入Flash
                Flash_User_Write();
                goto clear;
            }
            j++;
        }
        j = 0;
        tryCount++;
    }

    clear:
    Ethernet_Clear_Buffer();
}

//NET指令 查询IMSI号
static void Ethernet_IMSI(void)
{
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char tryCount = 0;

    unsigned char Ethernet_CMD_IMSI[9] = "AT+CIMI\r\n";

    while(tryCount<2)
    {
        Ethernet_SendDataFun(9,Ethernet_CMD_IMSI);  //查询IMEI号
        delay1ms(300);
        while(j<62)
        {
            if(LpUart_4G_Buffer[j]=='O'&&LpUart_4G_Buffer[j+1]=='K')
            {
                i = j;
                for(j=0;j<15;j++)External_Data.IMSI[j] = LpUart_4G_Buffer[i-19+j];
                //写入Flash
                Flash_User_Write();
                goto clear;
            }
            j++;
        }
        j = 0;
        tryCount++;
    }
    
    clear:
    Ethernet_Clear_Buffer();
}

//NET指令 Socket创建状态判断
static void Ethernet_Socket_Create(void)
{
    unsigned char i = 0;
    // unsigned char tryCount = 0;

    // while(tryCount<80)
    // {
    //     delay1ms(100);
    //     while(i<62)
    //     {
    //         if(LpUart_4G_Buffer[i]=='C'&&LpUart_4G_Buffer[i+1]=='O'&&LpUart_4G_Buffer[i+2]=='N'&&LpUart_4G_Buffer[i+3]=='N')
    //         {
    //             return;
    //         }
    //         i++;
    //     }
    //     i = 0;
    //     tryCount++;
    // }

    System_Data.Delay_Flag = 1;   
    if(System_Data.User_Delay_Count / 100)
    {
        System_Data.User_Delay_Count -= 100;
        System_Data.Delay_Through++;
        while(i<62)
        {
            if((LpUart_4G_Buffer[i]=='C'&&LpUart_4G_Buffer[i+1]=='O'&&LpUart_4G_Buffer[i+2]=='N'&&LpUart_4G_Buffer[i+3]=='N') || System_Data.Delay_Through > 80)
            {
                System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
                break;
            }
            i++;
        }
    }
}



/*NET指令 设置Socket,透传模式,Socket固定1
ip: IP地址数组
port: 端口号
*/
static void Ethernet_Set_Socket(unsigned char *ip,unsigned short port)
{
    unsigned char i = 21;
    unsigned char Ethernet_CMD_SOCKET[49] = "AT+QIOPEN=1,1,\"TCP\",\"";

    if(!System_Data.Delay_Flag)
    {
        i += Number_To_Char(*ip,&Ethernet_CMD_SOCKET[i]);
        Ethernet_CMD_SOCKET[i] = '.';
        i++;
        i += Number_To_Char(*(ip+1),&Ethernet_CMD_SOCKET[i]);
        Ethernet_CMD_SOCKET[i] = '.';
        i++;
        i += Number_To_Char(*(ip+2),&Ethernet_CMD_SOCKET[i]);
        Ethernet_CMD_SOCKET[i] = '.';
        i++;
        i += Number_To_Char(*(ip+3),&Ethernet_CMD_SOCKET[i]);
        Ethernet_CMD_SOCKET[i] = '\"';
        i++;
        Ethernet_CMD_SOCKET[i] = ',';
        i++;
        i += Number_To_Char(port,&Ethernet_CMD_SOCKET[i]);
        Ethernet_CMD_SOCKET[i] = ',';
        Ethernet_CMD_SOCKET[i+1] = '0';
        Ethernet_CMD_SOCKET[i+2] = ',';
        Ethernet_CMD_SOCKET[i+3] = '2';  //透传
        Ethernet_CMD_SOCKET[i+4] = '\r';
        Ethernet_CMD_SOCKET[i+5] = '\n';

        Ethernet_SendDataFun(i+6,Ethernet_CMD_SOCKET);  //创建Socket指令
    }
    Ethernet_Socket_Create();  //判断创建状态
}

/*
NET指令 关闭Socket
Socket_Number: Socket编号
*/
static void Ethernet_Close_Socket(unsigned char Socket_Number)
{
    unsigned char i = 11;

    unsigned char Ethernet_CMD_QICLOSE[14] = "AT+QICLOSE=";

    i += Number_To_Char(Socket_Number,&Ethernet_CMD_QICLOSE[i]);
    Ethernet_CMD_QICLOSE[i] = '\r';
    Ethernet_CMD_QICLOSE[i+1] = '\n';

    Ethernet_Judgment(14,Ethernet_CMD_QICLOSE);  //关闭Socket
}

//NET指令 退出透传
static void Ethernet_TTMode_Exit(void)
{
    unsigned char Ethernet_CMD_Exit[3] = "+++";

    System_Data.Delay_Flag = 1;   

    Ethernet_Judgment(3,Ethernet_CMD_Exit);  //退出透传模式
}

//NET指令 自动进入休眠模式，默认串口无数据5s后
static void Ethernet_SleepAuto(void)
{
    unsigned char Ethernet_CMD_Sleep[12] = "AT+QSCLK=2\r\n";

    Ethernet_Judgment(12,Ethernet_CMD_Sleep);  //自动休眠
}

//NET指令 最小功能模式
static void Ethernet_Mini_Mode(void)
{
    unsigned char Ethernet_CMD_Mini[11] = "AT+CFUN=0\r\n";

    Ethernet_Judgment(11,Ethernet_CMD_Mini);  //最小功能
}

//NET指令 最全功能模式
static void Ethernet_Func_Mode(void)
{
    unsigned char Ethernet_CMD_Func[11] = "AT+CFUN=1\r\n";

    Ethernet_Judgment(11,Ethernet_CMD_Func);  //最全功能
    Ethernet_Clear_Buffer();
}

//NET指令 查询信号强度
static void Ethernet_ATCSQ(void)
{
    unsigned char i = 0;
    // unsigned char tryCount = 0;

    unsigned char Ethernet_CMD_CSQ[8] = "AT+CSQ\r\n";

    System_Data.Ethernet_CSQ = 99;

    // while(tryCount<2)
    // {
    //     Ethernet_SendDataFun(8,Ethernet_CMD_CSQ);  //查询信号强度
    //     delay1ms(300);
    //     while(j<62)
    //     {
    //         if(LpUart_4G_Buffer[j]==0x20)
    //         {
    //             System_Data.Ethernet_CSQ = LpUart_4G_Buffer[j+1] - 0x30;
    //             if(LpUart_4G_Buffer[j+2] != 0x2C)System_Data.Ethernet_CSQ = System_Data.Ethernet_CSQ * 10 + (LpUart_4G_Buffer[j+2]-0x30);
    //             goto clear;
    //         }
    //         j++;
    //     }
    //     j = 0;
    //     tryCount++;
    // }

    // clear:
    // Ethernet_Clear_Buffer();

    if(!System_Data.Delay_Flag)
    {
        Ethernet_SendDataFun(8,Ethernet_CMD_CSQ);  //查询信号强度
    }
    System_Data.Delay_Flag = 1;

    if(System_Data.User_Delay_Count / 300)
    {
        System_Data.User_Delay_Count -= 300;
        System_Data.Delay_Through++;
        while(i<62)
        {
            if(LpUart_4G_Buffer[i]==0x20)
            {
                System_Data.Delay_Through = 2;
                System_Data.Ethernet_CSQ = LpUart_4G_Buffer[i+1] - 0x30;
                if(LpUart_4G_Buffer[i+2] != 0x2C)
                {
                    System_Data.Ethernet_CSQ = System_Data.Ethernet_CSQ * 10 + (LpUart_4G_Buffer[i+2]-0x30);
                }
                break;
            }
            i++;
        }

        if(System_Data.Delay_Through > 1)
        {
            System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
        }
        else
        {
            Ethernet_SendDataFun(8,Ethernet_CMD_CSQ);  //查询信号强度
        }
    }
    
}

//NET指令 查询网络附着状态
static void Ethernet_CGATT(void)
{
    unsigned char i = 0;
    // unsigned char tryCount = 0;

    unsigned char Ethernet_CMD_CGATT[11] = "AT+CGATT?\r\n";

    // while(tryCount<20)
    // {
    //     Ethernet_SendDataFun(11,Ethernet_CMD_CGATT);  //查询网络附着
    //     delay1ms(500);
    //     while(i<62)
    //     {
    //         if(LpUart_4G_Buffer[i]==0x20 && LpUart_4G_Buffer[i+1] == 0x31)
    //         {
    //             goto clear;
    //         }
    //         i++;
    //     }
    //     i = 0;
    //     tryCount++;
    // }

    // clear:
    // Ethernet_Clear_Buffer();

    if(!System_Data.Delay_Flag)
    {
        Ethernet_SendDataFun(11,Ethernet_CMD_CGATT);  //查询网络附着
    }
    System_Data.Delay_Flag = 1;

    if(System_Data.User_Delay_Count / 500)
    {
        System_Data.User_Delay_Count -= 500;
        System_Data.Delay_Through++;
        while(i<62)
        {
            if(LpUart_4G_Buffer[i] == 0x20 && LpUart_4G_Buffer[i+1] == 0x31)
            {
                System_Data.Delay_Through = 21;
                break;
            }
            i++;
        }

        if(System_Data.Delay_Through > 20)
        {
            System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
        }
        else
        {
            Ethernet_SendDataFun(11,Ethernet_CMD_CGATT);  //查询网络附着
        }
    }
}

//NET指令 创建Socket是否成功
static unsigned char Ethernet_Senddata_Success(void)
{
    unsigned char i = 0;
    unsigned char flag = 0;

    while(i < 60)
    {
        if(LpUart_4G_Buffer[i] == 'E' && LpUart_4G_Buffer[i+1] == 'R' && LpUart_4G_Buffer[i+2] == 'R' && LpUart_4G_Buffer[i+3] == 'O' && LpUart_4G_Buffer[i+4] == 'R')
        {
            flag = 1;
            break;
        }
        i++;
    }
    Ethernet_Clear_Buffer();
    return flag;
}

//NET指令 等待服务器返回数据
static void Ethernet_Server_Ready(void)
{
    //unsigned char i = 0;

    // while(tryCount < 80)
    // {
    //     delay1ms(100);
    //     if(System_Data.Valid_Data_Ready == 1)
    //     {
    //         break;
    //     }
    //     tryCount++;
    // }

    System_Data.Delay_Flag = 1;   
    if(System_Data.User_Delay_Count / 100)
    {
        System_Data.User_Delay_Count -= 100;
        System_Data.Delay_Through++;
        if(System_Data.Valid_Data_Ready == 1 || System_Data.Delay_Through > 80)
        {
            System_Data.User_Delay_Count = System_Data.Delay_Flag = System_Data.Delay_Through = 0;
        }
    }
}


//4G模块初始化
void Ethernet_Init(void)
{
    //LED On
    Ethernet_LED_State(1);

    while (1)
    {
        switch(System_Data.Ethernet_Send_State)
        {
            case 0 :
                //等待模块开机
                Ethernet_Power_On();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 1 :
                //更改波特率9600
                Ethernet_Uart();
                if(!System_Data.Delay_Flag)
                {
                    //串口重新初始化
                    BleAnd4GLpUartInit(LP_NET,LPMODE_USUAL);
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 2 :
                //等待网络附着
                Ethernet_CGATT();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 3 :
                //查询信号强度
                Ethernet_ATCSQ();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State = 0;
                    goto Ethernet_continue;
                }
                break;

            default:
                break;
        }
    }

    Ethernet_continue:
    //查询IMEI
    Ethernet_IMEI();

    //查询IMSI
    Ethernet_IMSI();

    //4G下电
    App_LpUart1DeInit();

    //LED Off
    Ethernet_LED_State(0);
}

//4G数据处理
void Ethernet_Analysis(void)
{
    unsigned char i = 0;

    if((System_Data.LpUart_4G_Timeout == 0) && (System_Data.Valid_Data_Ready == 1))
    {
        // if(((unsigned short)Valid_Data[31]<<8)|Valid_Data[32] != CRC16(Valid_Data,31))   //CRC16 校验
        // {
        //     Ethernet_Errorcode |= 0x00010000;
        //     LpUart_4G_Busy = 0;
        //     return;
        // }
        // External_Data.Version_Software = Valid_Data[4];      //获取软件版本号
        // External_Data.Version_Hardware = Valid_Data[5];      //获取硬件版本号

        //获取唤醒屏幕上传有效标志位
        External_Data.LCD_UpData_Flag = Valid_Data[7];

        if((Valid_Data[6]&0x01) == 0x01)              //UTC有效
        {
            External_Data.UTC = ((unsigned long)Valid_Data[8]<<24)|((unsigned long)Valid_Data[9]<<16)|((unsigned short)Valid_Data[10]<<8)|Valid_Data[11];    //获取UTC
        }
        if((Valid_Data[6]&0x02) == 0x02)              //获取IP 1地址
        {
            External_Data.Address_IP1[0] = Valid_Data[12];
            External_Data.Address_IP1[1] = Valid_Data[13];
            External_Data.Address_IP1[2] = Valid_Data[14];
            External_Data.Address_IP1[3] = Valid_Data[15];
            External_Data.port1 = ((unsigned short)Valid_Data[16]<<8)|Valid_Data[17];
        }
        if((Valid_Data[6]&0x04) == 0x04)              //获取时间间隔、采集次数
        {
            External_Data.Sensor_Interval = ((unsigned short)Valid_Data[18]<<8)|Valid_Data[19];
            External_Data.Sensor_ScheduledTimes = ((unsigned short)Valid_Data[20]<<8)|Valid_Data[21];
        }
        if((Valid_Data[6]&0x10) == 0x10)              //获取ID
        {
            for(i=0;i<10;i++)External_Data.ID[0+i] = Valid_Data[22+i];
            External_Data.ID[10] = 1;  //用户修改ID标志
            BLE_ID_Set();
            BLE_Start();
            BLE_DEEPSLEEP();
        }

        Flash_User_Write();

        Valid_Data[0] = 0xA5;
        Valid_Data[1] = 0x5A;
        for(i=2;i<64;i++)Valid_Data[i] = 0;

        System_Data.Valid_Data_Ready = 0;  //置为无数据
    }
}

//扫描次数到达4G发送
void Ethernet_Send_Time(void)
{
    unsigned long UTC = 0;
    unsigned short i = 0;
    unsigned short j = 0;
    unsigned short temp = 0;
    unsigned char NET_Send = 0;
    unsigned char NET_Count = 0;
    unsigned char Temp_Flag = 0;
    unsigned char IP_Count = 0;


    //到达上传次数、蓝牙关闭、屏幕熄灭
    if(External_Data.Sensor_ScanCount >= External_Data.Sensor_ScheduledTimes || System_Data.NET_Send_Type > 0 || System_Data.Ethernet_Send_State != 0)
    {
        //根据4G状态执行流程
        switch(System_Data.Ethernet_Send_State)
        {
            case 0 :
                //4G上电
                BleAnd4GLpUartInit(LP_NET,LPMODE_POWER);
                //LED On
                Ethernet_LED_State(1);
                //4G进行状态下一阶段
                System_Data.Ethernet_Send_State++;
                break;

            case 1 :
                //等待模块开机
                Ethernet_Power_On();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 2 :
                //更改波特率9600
                Ethernet_Uart();
                if(!System_Data.Delay_Flag)
                {
                    //串口重新初始化
                    BleAnd4GLpUartInit(LP_NET,LPMODE_USUAL);
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 3 :
                //等待网络附着
                Ethernet_CGATT();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            case 4 :
                //查询信号强度
                Ethernet_ATCSQ();
                if(!System_Data.Delay_Flag)
                {
                    //4G进行状态下一阶段
                    System_Data.Ethernet_Send_State++;
                }
                break;

            default:
                break;
        }
        //初始化流程未完成，重出重入
        if(System_Data.Ethernet_Send_State < 5)
        {
            return;
        }
        //发送进行状态清零
        System_Data.Ethernet_Send_State = 0;

        //检查RAM数据是否异常
        if(External_Data.IMEI[0] == 0 || External_Data.IMSI[0] == 0)
        {
            Ethernet_IMEI();  //查询IMEI号
            Ethernet_IMSI();
        }
        //计算分包次数
        NET_Send = External_Data.Sensor_ScanCount / 477;
        if(External_Data.Sensor_ScanCount % 477 != 0)
        {
            NET_Send++;
        }

        //帧头
        Ethernet_SendData[0] = 0xA5;
        Ethernet_SendData[1] = 0x5A;
        //包总数
        Ethernet_SendData[2] = NET_Send;
        //软硬件版本号
        Ethernet_SendData[6] = External_Data.Version_Software;
        Ethernet_SendData[7] = External_Data.Version_Hardware;
        //上传类型
        //是否存在蓝牙连接
        if(System_Data.BLE_Link_Flag)
        {
            //同时是否为屏幕熄灭上传
            if(System_Data.NET_Send_Type == 5);
            else
            {
                System_Data.NET_Send_Type = 4;  //存在蓝牙连接
            }
        }
        Ethernet_SendData[8] = System_Data.NET_Send_Type;
        //ID与IMEI IMSI
        for(i=0;i<10;i++)Ethernet_SendData[9 + i] = External_Data.ID[i];
        for(i=0;i<15;i++)Ethernet_SendData[19 + i] = External_Data.IMEI[i];
        for(i=0;i<15;i++)Ethernet_SendData[34 + i] = External_Data.IMSI[i];
        //唤醒屏幕上传有效标志位
        Ethernet_SendData[49] = External_Data.LCD_UpData_Flag;
        //等待电池电压采集
        i = 0;
        while(i<30)
        {
            if(System_Data.ADBusy==0)break;
            delay1ms(1);
            i++;
        }
        //电池电压
        Ethernet_SendData[50] = External_Data.Battery_Level;
        //信号强度
        Ethernet_SendData[51] = System_Data.Ethernet_CSQ;
        //传感器状态
        Ethernet_SendData[52] = System_Data.Sensor_State;
        //传感器类型
        Ethernet_SendData[53] = External_Data.Sensor_Type;
        //量程下限
        Ethernet_SendData[54] = External_Data.Sensor_Min >> 8;
        Ethernet_SendData[55] = External_Data.Sensor_Min;
        //量程上限
        Ethernet_SendData[56] = External_Data.Sensor_Max >> 8;
        Ethernet_SendData[57] = External_Data.Sensor_Max;
        //记录起始UTC
        Ethernet_SendData[58] = External_Data.UTC >> 24;
        Ethernet_SendData[59] = External_Data.UTC >> 16;
        Ethernet_SendData[60] = External_Data.UTC >> 8;
        Ethernet_SendData[61] = External_Data.UTC;
        //记录间隔
        Ethernet_SendData[62] = External_Data.Sensor_Interval >> 8;
        Ethernet_SendData[63] = External_Data.Sensor_Interval;
        //记录次数
        Ethernet_SendData[64] = External_Data.Sensor_ScheduledTimes >> 8;
        Ethernet_SendData[65] = External_Data.Sensor_ScheduledTimes;
        //临时发送
        if(System_Data.NET_Send_Type > 0)
        {
            Temp_Flag = 1;
        }

        while(NET_Count < NET_Send || Temp_Flag)
        {
            Temp_Flag = 0;
            NET_Count++;
            //包次序
            Ethernet_SendData[3] = NET_Count;
            //压力数据
            for(i=1; i + 477 * (NET_Count - 1) < External_Data.Sensor_ScanCount + 1; i++)
            {
                temp = Flash_PData_Read(i + 477 * (NET_Count - 1));
                Ethernet_SendData[66 + j] = temp>>8;
                j++;
                Ethernet_SendData[66 + j] = temp;
                j++;
                if(i == 477)
                {
                    i++;
                    break;
                }
            }
            if(NET_Count > 1)
            {
                UTC = External_Data.UTC + 477 * (NET_Count - 1) * External_Data.Sensor_Interval;
                //记录起始UTC
                Ethernet_SendData[58] = UTC >> 24;
                Ethernet_SendData[59] = UTC >> 16;
                Ethernet_SendData[60] = UTC >> 8;
                Ethernet_SendData[61] = UTC;
            }
            //帧长度
            i = (i - 1) * 2;
            temp = 70 + i;
            Ethernet_SendData[4] = temp >> 8;
            Ethernet_SendData[5] = temp;
            //CRC校验
            temp = CRC16(Ethernet_SendData, temp);
            Ethernet_SendData[66 + i] = temp >> 8;
            Ethernet_SendData[67 + i] = temp;
            //帧尾
            Ethernet_SendData[68 + i] = 0x55;
            Ethernet_SendData[69 + i] = 0xAA;

            temp = 0;

            // //创建socket to IP1
            // Ethernet_Set_Socket(External_Data.Address_IP1,External_Data.port1);
            // //是否连接成功
            // temp |= Ethernet_Senddata_Success();
            // //发送数据
            // Ethernet_SendDataFun(70 + i, Ethernet_SendData);
            // //等待数据返回
            // Ethernet_Server_Ready();
            // //退出透传
            // Ethernet_TTMode_Exit();
            // //关闭socket
            // Ethernet_Close_Socket(1);
            // //数据处理
            // Ethernet_Analysis();

            // //创建socket to IP2
            // Ethernet_Set_Socket(External_Data.Address_IP2,External_Data.port2);
            // //是否连接成功
            // temp = temp << 1 | Ethernet_Senddata_Success();
            // //发送数据
            // Ethernet_SendDataFun(70 + i, Ethernet_SendData);
            // //等待数据返回
            // Ethernet_Server_Ready();
            // //退出透传
            // Ethernet_TTMode_Exit();
            // //关闭socket
            // Ethernet_Close_Socket(1);
            // //数据处理
            // Ethernet_Analysis();


            while(1)
            {
                switch(System_Data.Ethernet_Send_State)
                {
                    case 0 :
                        if(IP_Count)
                        {
                            //创建socket to IP2
                            Ethernet_Set_Socket(External_Data.Address_IP2,External_Data.port2);
                            if(!System_Data.Delay_Flag)
                            {
                                IP_Count = 0;
                                System_Data.Ethernet_Send_State++;
                            }
                        }
                        else 
                        {
                            //创建socket to IP1
                            Ethernet_Set_Socket(External_Data.Address_IP1,External_Data.port1);
                            if(!System_Data.Delay_Flag)
                            {
                                IP_Count++;
                                System_Data.Ethernet_Send_State++;
                            }
                        }
                        break;

                    case 1 :
                        if(!System_Data.Delay_Flag)
                        {
                            //是否连接成功
                            Ethernet_Senddata_Success();
                            //发送数据
                            Ethernet_SendDataFun(70 + i, Ethernet_SendData);
                        }
                        //等待数据返回
                        Ethernet_Server_Ready();
                        if(!System_Data.Delay_Flag)
                        {
                            System_Data.Ethernet_Send_State++;
                        }
                        break;

                    case 2 :
                        //退出透传
                        Ethernet_TTMode_Exit();
                        if(!System_Data.Delay_Flag)
                        {
                            System_Data.Ethernet_Send_State++;
                        }
                        break;

                    case 3 :
                        //关闭socket
                        Ethernet_Close_Socket(1);
                        if(!System_Data.Delay_Flag)
                        {
                            System_Data.Ethernet_Send_State++;
                        }
                        break;

                    case 4 :
                        //数据处理
                        Ethernet_Analysis();
                        if(IP_Count)
                        {
                            System_Data.Ethernet_Send_State = 0;
                        }
                        else 
                        {
                            goto clean;
                        }
                        break;
                    
                    default:
                        break;
                }
                Battery_Scan();
                SensorScan();
                BLE_Analysis();
                LCD_General();
            }
            clean:
            //发送进行状态清零
            System_Data.Ethernet_Send_State = IP_Count = 0;
        }

        //扫描次数满足上传次数时才清零
        if(External_Data.Sensor_ScanCount >= External_Data.Sensor_ScheduledTimes)
        {
            External_Data.Sensor_ScanCount = 0;
        }
        //4G下电
        App_LpUart1DeInit();
        //LED Off
        Ethernet_LED_State(0);
        //上传类型恢复默认
        System_Data.NET_Send_Type = 0;


        //两次IP都未连接成功，MCU复位
        // if(temp > 2 && (*((volatile unsigned char *)Flash_Address_Ethernet))!=171)
        // {
        //     //写入IP连接失败
        //     Flash_WriteByte(Flash_Address_Ethernet,171u);
        //     //MCU复位
        //     Sys_Reset();
        // }
        // else 
        // {
        //     Flash_SectorErase(Flash_Address_Ethernet);
        // }
    }
}



