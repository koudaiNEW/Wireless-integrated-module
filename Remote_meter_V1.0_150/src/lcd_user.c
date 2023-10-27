#include "iic_user.h"
#include "lcd_user.h"
#include "sensor_user.h"
#include "timer_user.h"
#include "gpio.h"
#include "ddl.h"

extern user_Data External_Data;
extern Timely_Data System_Data;

unsigned char LCD_Lattice[16] = {0x00};

//LCD显示超时
void LCD_Timeout(void)
{
    if(External_Data.LCD_WakeUp_Time == 0xFFFF)
    {
        return;
    }
    if(System_Data.LCD_Busy&&System_Data.LCD_Count)
    {
        System_Data.LCD_Count--;
    }
}

/*
刷新显存
state: 发送显存状态
*/
static void LCD_Refresh(unsigned char state)
{
    unsigned char i = 0;
    unsigned char cmd[17] = {0x00};

    if(state)
    {
        for(i=0;i<16;i++)cmd[i+1] = LCD_Lattice[i];
        // for(i=0;i<16;i++)cmd[i+1] = 0xFF;
    }
    LCD_Write(cmd,17);
}

//LCD上电初始化
void LCD_Init(void)
{
    unsigned char temp = 0;
    LCDIICInit();
    delay1ms(100);
    //软件重启
    temp = 0xEA;
    LCD_Write(&temp,1);
    delay1ms(100);
    //功耗设定
    temp = 0xBC;
    LCD_Write(&temp,1);
    //显存清空
    LCD_Refresh(CLEAN);
    //开启显示
    temp = 0xC8;
    LCD_Write(&temp,1);

    System_Data.LCD_Busy = 1;
    // System_Data.LCD_Count = 30;  //LCD显示定时300s
}

/*
大数字转换
number: 需要转换数字，一位
point: 左下角小数点
<return> 对应显存
*/
unsigned char LCD_BigNumber(unsigned char number,unsigned char point)
{
    unsigned char data = 0;

    //左下角小数点显示
    if(point)data |= 0x08;

    //大数字管脚
    switch(number)
    {
        case 0:
            data |= 0xF5;
            break;
        case 1:
            data |= 0x60;
            break;
        case 2:
            data |= 0xB6;
            break;
        case 3:
            data |= 0xF2;
            break;
        case 4:
            data |= 0x63;
            break;
        case 5:
            data |= 0xD3;
            break;
        case 6:
            data |= 0xD7;
            break;
        case 7:
            data |= 0x70;
            break;
        case 8:
            data |= 0xF7;
            break;
        case 9:
            data |= 0xF3;
            break;
        default:
            break;
    }
    return data;
}

/*
米字格第一位
number: 需要转换数字，一位
*/
static void LCD_Mizi_One(unsigned char number)
{
    switch(number)
    {
        case 0:
            LCD_Lattice[15] |= 0xC0;
            LCD_Lattice[0] |= 0x11;
            LCD_Lattice[1] |= 0x90;
            break;
        case 1:
            LCD_Lattice[1] |= 0x90;
            break;
        case 2:
            LCD_Lattice[15] |= 0x80;
            LCD_Lattice[0] |= 0x51;
            LCD_Lattice[1] |= 0xC0;
            break;
        case 3:
            LCD_Lattice[15] |= 0x80;
            LCD_Lattice[0] |= 0x41;
            LCD_Lattice[1] |= 0xD0;
            break;
        case 4:
            LCD_Lattice[15] |= 0x40;
            LCD_Lattice[0] |= 0x40;
            LCD_Lattice[1] |= 0xD0;
            break;
        case 5:
            LCD_Lattice[15] |= 0xC0;
            LCD_Lattice[0] |= 0x41;
            LCD_Lattice[1] |= 0x50;
            break;
        case 6:
            LCD_Lattice[15] |= 0xC0;
            LCD_Lattice[0] |= 0x51;
            LCD_Lattice[1] |= 0x50;
            break;
        case 7:
            LCD_Lattice[15] |= 0x80;
            LCD_Lattice[1] |= 0x90;
            break;
        case 8:
            LCD_Lattice[15] |= 0xC0;
            LCD_Lattice[0] |= 0x51;
            LCD_Lattice[1] |= 0xD0;
            break;
        case 9:
            LCD_Lattice[15] |= 0xC0;
            LCD_Lattice[0] |= 0x41;
            LCD_Lattice[1] |= 0xD0;
            break;
        default:
            break;
    }
}

/*
米字格第四位
number: 需要转换数字，一位
*/
static void LCD_Mizi_Four(unsigned char number)
{
    switch(number)
    {
        case 0:
            LCD_Lattice[5] |= 0x0C;
            LCD_Lattice[6] |= 0x11;
            LCD_Lattice[15] |= 0x09;
            break;
        case 1:
            LCD_Lattice[15] |= 0x09;
            break;
        case 2:
            LCD_Lattice[5] |= 0x08;
            LCD_Lattice[6] |= 0x51;
            LCD_Lattice[15] |= 0x0C;
            break;
        case 3:
            LCD_Lattice[5] |= 0x08;
            LCD_Lattice[6] |= 0x41;
            LCD_Lattice[15] |= 0x0D;
            break;
        case 4:
            LCD_Lattice[5] |= 0x04;
            LCD_Lattice[6] |= 0x40;
            LCD_Lattice[15] |= 0x0D;
            break;
        case 5:
            LCD_Lattice[5] |= 0x0C;
            LCD_Lattice[6] |= 0x41;
            LCD_Lattice[15] |= 0x05;
            break;
        case 6:
            LCD_Lattice[5] |= 0x0C;
            LCD_Lattice[6] |= 0x51;
            LCD_Lattice[15] |= 0x05;
            break;
        case 7:
            LCD_Lattice[5] |= 0x08;
            LCD_Lattice[15] |= 0x09;
            break;
        case 8:
            LCD_Lattice[5] |= 0x0C;
            LCD_Lattice[6] |= 0x51;
            LCD_Lattice[15] |= 0x0D;
            break;
        case 9:
            LCD_Lattice[5] |= 0x0C;
            LCD_Lattice[6] |= 0x41;
            LCD_Lattice[15] |= 0x0D;
            break;
        default:
            break;
    }
}

/*
米字格显示数字
number: 需要转换数字，一位
digit: 选择位数
*/
void LCD_Mizi_Number(unsigned char number,unsigned char digit)
{
    unsigned char *address = &LCD_Lattice[1];

    switch(digit)
    {
        case 1:
            LCD_Mizi_One(number);
            return;
        case 2:
            address = &LCD_Lattice[1];
            break;
        case 3:
            address = &LCD_Lattice[3];
            break;
        case 4:
            LCD_Mizi_Four(number);
            return;
        default:
            break;
    }

    switch(number)
    {
        case 0:
            *address     |= 0x0C;
            *(address+1) |= 0x11;
            *(address+2) |= 0x90;
            break;
        case 1:
            *(address+2) |= 0x90;
            break;
        case 2:
            *address     |= 0x08;
            *(address+1) |= 0x51;
            *(address+2) |= 0xC0;
            break;
        case 3:
            *address     |= 0x08;
            *(address+1) |= 0x41;
            *(address+2) |= 0xD0;
            break;
        case 4:
            *address     |= 0x04;
            *(address+1) |= 0x40;
            *(address+2) |= 0xD0;
            break;
        case 5:
            *address     |= 0x0C;
            *(address+1) |= 0x41;
            *(address+2) |= 0x50;
            break;
        case 6:
            *address     |= 0x0C;
            *(address+1) |= 0x51;
            *(address+2) |= 0x50;
            break;
        case 7:
            *address     |= 0x08;
            *(address+2) |= 0x90;
            break;
        case 8:
            *address     |= 0x0C;
            *(address+1) |= 0x51;
            *(address+2) |= 0xD0;
            break;
        case 9:
            *address     |= 0x0C;
            *(address+1) |= 0x41;
            *(address+2) |= 0xD0;
            break;
        default:
            break;
    }
}

/*
米字格小数点
point: 需要显示的小数点，按Bit选择
       Bit0: 第1位，Bit1: 第2位，Bit2: 第3位，Bit3: 第4位，
*/
void LCD_Mizi_Point(unsigned char point)
{
    if(point&0x01)LCD_Lattice[5] |= 0x02;
    if(point&0x02)LCD_Lattice[3] |= 0x01;
    if(point&0x04)LCD_Lattice[1] |= 0x02;
    if(point&0x08)LCD_Lattice[3] |= 0x02;
}

/*
显示测量单位选择
unit: 需要显示的单位，按Bit选择
       Bit0: kPa，Bit1: MPa，Bit2: bar，Bit3: mH2O，Bit4: ℃，Bit5: ℉
*/
void LCD_Unit(unsigned char unit)
{
    if(unit&0x01)LCD_Lattice[11] |= 0x01;
    if(unit&0x02)LCD_Lattice[12] |= 0x10;
    if(unit&0x04)LCD_Lattice[12] |= 0x01;
    if(unit&0x08)LCD_Lattice[13] |= 0x10;
    if(unit&0x10)LCD_Lattice[13] |= 0x01;
    if(unit&0x20)LCD_Lattice[15] |= 0x10;
}

/*
特殊符号选择
symbols: 需要显示的单位，按Bit选择
       Bit0: 蓝牙，Bit1: 未知，Bit2: 报警，Bit3: 外部电源，Bit4: 上，Bit5: 中，Bit6: 下
*/
void LCD_Symbols(unsigned char symbols)
{
    if(symbols&0x01)LCD_Lattice[12] |= 0x40; //蓝牙
    if(symbols&0x02)LCD_Lattice[12] |= 0x20; //未知
    if(symbols&0x04)LCD_Lattice[13] |= 0x40; //报警
    if(symbols&0x08)LCD_Lattice[14] |= 0x40; //外部电源
    if(symbols&0x10)LCD_Lattice[11] |= 0x10; //上
    if(symbols&0x20)LCD_Lattice[11] |= 0x20; //中
    if(symbols&0x40)LCD_Lattice[11] |= 0x40; //下
}

/*
4G信号强度
signal: 当前4G信号强度
*/
void LCD_4GSignal(unsigned char signal)
{
    LCD_Lattice[12] |= 0x02;
    if(signal == 99)return;
    if(signal > 8)
    {
        LCD_Lattice[13]  |= 0x20;
    }
    if(signal > 16)
    {
        LCD_Lattice[13]  |= 0x04;
    }
    if(signal > 24)
    {
        LCD_Lattice[13]  |= 0x02;
    }
}

/*
电池电量
battery: 当前电池电压/3*100
*/
void LCD_Battery(unsigned char battery)
{
    LCD_Lattice[14] |= 0x01;
    if(battery < 110)return;
    if(battery >= 110)
    {
        LCD_Lattice[14] |= 0x20;
    }
    if(battery > 114)
    {
        LCD_Lattice[14] |= 0x10;
    }
    if(battery > 117)
    {
        LCD_Lattice[14] |= 0x02;
    }
}

/*
测量数值到达当前量程百分比
pdata: 当前压力/100
*/
void LCD_Range(short pdata)
{
    unsigned short temp = External_Data.Sensor_Max - External_Data.Sensor_Min;
    LCD_Lattice[11] |= 0x02;
    LCD_Lattice[15] |= 0x20;
    if(pdata <= 0)return;
    if(pdata > 0)  //S2
    {
        LCD_Lattice[11] |= 0x04;
    }
    if(pdata > (temp / 10))  //S3
    {
        LCD_Lattice[11] |= 0x08;
    }
    if(pdata > (temp / 5))  //S4
    {
        LCD_Lattice[12] |= 0x80;
    }
    if(pdata > (unsigned short)(temp / 3.33))  //S5
    {
        LCD_Lattice[12] |= 0x08;
    }
    if(pdata > (unsigned short)(temp / 2.5))  //S6
    {
        LCD_Lattice[12] |= 0x04;
    }
    if(pdata > (temp / 2))  //S7
    {
        LCD_Lattice[13] |= 0x80;
    }
    if(pdata > (unsigned short)(temp / 1.67))  //S8
    {
        LCD_Lattice[13] |= 0x08;
    }
    if(pdata > (unsigned short)(temp / 1.43))  //S9
    {
        LCD_Lattice[14] |= 0x80;
    }
    if(pdata > (unsigned short)(temp / 1.25))  //S10
    {
        LCD_Lattice[14] |= 0x08;
    }
    if(pdata > (unsigned short)(temp / 1.11))  //S11
    {
        LCD_Lattice[14] |= 0x04;
    }
}

/*
米字格，数字显示，整体
number: 需要转换数字，最大四位,有符号
*/
void LCD_SmallNumber(short number)
{
    if(number>=0)
    {
        if(number<1000)
        {
            LCD_Mizi_Number(number%10,4);
            LCD_Mizi_Number(number/10%10,3);
            LCD_Mizi_Number(number/100,2);
            LCD_Mizi_Point(0x02);
            return;
        }
        if(number<10000)
        {
            LCD_Mizi_Number(number%10,4);
            LCD_Mizi_Number(number/10%10,3);
            LCD_Mizi_Number(number/100%10,2);
            LCD_Mizi_Number(number/1000,1);
            LCD_Mizi_Point(0x02);
            return;
        }
        LCD_Mizi_Number(number/10%10,4);
        LCD_Mizi_Number(number/100%10,3);
        LCD_Mizi_Number(number/1000%10,2);
        LCD_Mizi_Number(number/10000,1);
        LCD_Mizi_Point(0x01);
    }
    else 
    {
        number = -number;
        LCD_Lattice[0] |= 0x40;
        LCD_Lattice[1] |= 0x40;
        if(number<1000)
        {
            LCD_Mizi_Number(number%10,4);
            LCD_Mizi_Number(number/10%10,3);
            LCD_Mizi_Number(number/100,2);
            LCD_Mizi_Point(0x02);
            return;
        }
        if(number<10000)
        {
            LCD_Mizi_Number(number/10%10,4);
            LCD_Mizi_Number(number/100%10,3);
            LCD_Mizi_Number(number/1000,2);
            LCD_Mizi_Point(0x01);
            return;
        }
        LCD_Mizi_Number(number/100%10,4);
        LCD_Mizi_Number(number/1000%10,3);
        LCD_Mizi_Number(number/10000,2);
    }
}

/*
压力显示，同时显示单位及百分比
pdata: 压力
unit: 单位
*/
void LCD_Pressure(short pdata,unsigned char unit)
{
    //单位
    LCD_Unit(unit);

    //量程百分比
    LCD_Range(pdata);

    if(pdata > (short)External_Data.Sensor_Max || pdata < (short)External_Data.Sensor_Min)
    {
        LCD_Lattice[13] |= 0x40; //报警
    }

    if(pdata > 18888)
    {
        pdata = 18888;
    }
    //判断正负号
    if(pdata >= 0);
    else
    {
        pdata = -pdata;
        LCD_Lattice[11] |= 0x20; //中
    }

    if(pdata < 1000)
    {
        LCD_Lattice[7] |= LCD_BigNumber(pdata%10,0);
        LCD_Lattice[8] |= LCD_BigNumber(pdata/10%10,1);
        LCD_Lattice[9] |= LCD_BigNumber(pdata/100,0);
        return;
    }
    if(pdata >= 10000)
    {
        LCD_Lattice[11] |= 0x80;
        pdata -= 10000;
    }
    LCD_Lattice[7] |= LCD_BigNumber(pdata%10,0);
    LCD_Lattice[8] |= LCD_BigNumber(pdata/10%10,1);
    LCD_Lattice[9] |= LCD_BigNumber(pdata/100%10,0);
    LCD_Lattice[10] |= LCD_BigNumber(pdata/1000,0);
}

/*
压力显示，同时显示单位及百分比
pdata: 压力
unit: 单位
*/
void LCD_Pressure_Big(short pdata,unsigned char unit)
{
    //单位
    LCD_Unit(unit);

    //量程百分比
    LCD_Range(pdata);

    if(pdata > (short)External_Data.Sensor_Max || pdata < (short)External_Data.Sensor_Min)
    {
        LCD_Lattice[13] |= 0x40; //报警
    }

    if(pdata > 18888)
    {
        pdata = 18888;
    }
    //判断正负号
    if(pdata >= 0);
    else
    {
        pdata = -pdata;
        LCD_Lattice[11] |= 0x20; //中
    }

    // if(pdata < 1000)
    // {
    //     LCD_Lattice[7] |= LCD_BigNumber(pdata%10,0);
    //     LCD_Lattice[8] |= LCD_BigNumber(pdata/10%10,1);
    //     LCD_Lattice[9] |= LCD_BigNumber(pdata/100,0);
    //     return;
    // }
    // if(pdata >= 10000)
    // {
    //     LCD_Lattice[11] |= 0x80;
    //     pdata -= 10000;
    // }
    LCD_Lattice[7] |= LCD_BigNumber(pdata%10,0);
    LCD_Lattice[8] |= LCD_BigNumber(pdata/10%10,0);
    LCD_Lattice[9] |= LCD_BigNumber(pdata/100%10,1);
    LCD_Lattice[10] |= LCD_BigNumber(pdata/1000,0);
}


/*
温度显示，同时显示单位
tdata: 温度，有符号
unit: 单位
*/
void LCD_Temperature(short tdata,unsigned char unit)
{
    //单位
    LCD_Unit(unit);

    //温度显示
    LCD_SmallNumber(tdata);
}

//唤醒后显示
void LCD_General(void)
{
    unsigned char i = 0;
    unsigned char temp = 0;

    if(System_Data.LCD_Busy)
    {
        if(System_Data.LCD_Busy == 2)
        {
            LCD_Init();
            System_Data.LCD_Busy = 1;
        }
        delay1ms(200);

        //清空显存
        for(i=0;i<16;i++)LCD_Lattice[i] = 0;

        //压力显示
        SensorIICInit();
        LCD_Pressure(GetPTValue(PMODE),0x01);
        // if(External_Data.Sensor_Max > 10000)
        // {
        //     LCD_Pressure(GetPTValue(PMODE),0x01);
        // }
        // else 
        // {
        //     LCD_Pressure_Big(GetPTValue(PMODE),0x02);
        // }
        
        System_Data.TData = GetPTValue(TMODE);
        SensorIICDeInit();

        //温度显示
        LCD_Temperature(System_Data.TData,0x10);

        i = 0;
        while(i<30)
        {
            if(System_Data.ADBusy==0)break;
            i++;
        }
        LCD_Battery(External_Data.Battery_Level);

        //4G信号显示
        LCD_4GSignal(System_Data.Ethernet_CSQ);

        //蓝牙状态
        if(System_Data.BLE_Link_Flag)
        {
            temp |= 0x01;
        }

        //外部电源状态
        if(Gpio_GetInputIO(GpioPortA,GpioPin3))
        {
            temp |= 0x08;
        }

        //状态显示
        LCD_Symbols(temp);

        //显存发送
        LCD_Refresh(REFRESH);

        if(!System_Data.LCD_Count)
        {
            LCDIICDeInit();
            System_Data.LCD_Busy = 0;
            if(External_Data.LCD_UpData_Flag)
            {
                System_Data.NET_Send_Type = 5;  //屏幕熄灭上传
            }
        }
    }
}



