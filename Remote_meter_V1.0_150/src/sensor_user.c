#include "iic_user.h"
#include "ddl.h"
#include "flash_user.h"
#include "sensor_user.h"

extern user_Data External_Data;
extern Timely_Data System_Data;

/*
从2860获取压力或温度AD
type: 获取数值类型 0:压力 1:温度
*/
static long GetPTADC(unsigned char type)
{
    user_Bit32 ADData;
    ADData.u32 = 0;

    if(type)
    {
        System_Data.Sensor_State = ReadSensor(0x09,&ADData.byte[3],3);
    }
    else
    {
        System_Data.Sensor_State = ReadSensor(0x06,&ADData.byte[3],3);
    }
    ADData.i32 /= 256;

    return ADData.i32;
}

//求K
void GetK(void)
{
    // System_Data.pressureCoefficient = (External_Data.Sensor_Max - External_Data.Sensor_Min) / 100.0 / 6710887.0;  
    // System_Data.pressureCoefficient = 2500 / 6710887.0;  
    System_Data.pressureCoefficient = 150 / 6710887.0 * 100;  //150kPa量程保留两位小数
}

//冒泡排序，有符号
static void bubbleSort_Signed(long *arr,unsigned char n)
{
   unsigned char i=0;
   unsigned char j=0;
//    unsigned long temp = 0;
   long temp = 0;
   for (i = 0; i < n-1; i++)
   {
       for (j = 0; j < n-1-i; j++)
       {
           if (arr[j]> arr[j + 1])
           {
               temp = arr[j];
               arr[j] = arr[j + 1];
               arr[j + 1] = temp; 
           }
       }
   }
}

/*
压力、温度获取，单位kPa * 100
type: 获取数值类型 0:压力 1:温度
*/
short GetPTValue(unsigned char type)
{
    short PTValue = 0;
    long Data[10] = {0};
    long temp = 0;
    unsigned char i = 0;

    for(i=0;i<10;i++)Data[i] = GetPTADC(type);
    bubbleSort_Signed(Data,10);
    for(i=2;i<8;i++)temp += Data[i];
    temp /= 6;
    if(type)
    {
        PTValue = (short)((temp / 65536.0 + 25) * 100);
    }
    else 
    {
        // PTValue = (short)((temp - 838860) * System_Data.pressureCoefficient * 100) - External_Data.Zero_Offset;
        PTValue = (short)((temp - 838860) * System_Data.pressureCoefficient) - External_Data.Zero_Offset;
    }
    
    return PTValue;
}

//压力温度扫描
void SensorScan(void)
{
    user_Bit16 PData;
    PData.u16 = 0;

    // System_Data.sensorBusy = 1; //Test
    if(System_Data.sensorBusy)
    {
        SensorIICInit();
        External_Data.Sensor_ScanCount++;   //传感器扫描次数自加
        PData.i16 = GetPTValue(PMODE);              //扫描压力
        Flash_PData_Write(PData.u16);           //写压力数据
        // System_Data.TData = GetTemperature();           //扫描温度
        System_Data.sensorBusy = 0;
        SensorIICDeInit();
    }
}
