/******************************************************************************
* Copyright (C) 2019, Huada Semiconductor Co.,Ltd All rights reserved.    
*
* This software is owned and published by: 
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC 
* components. This software is licensed by HDSC to be adapted only 
* for use in systems utilizing HDSC components. HDSC shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein. HDSC is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.
*/
/******************************************************************************/
/** \file base_types.h
 **
 ** base type common define.
 ** @link SampleGroup Some description @endlink
 **
 **   - 2019-03-01  1.0  Lux First version.
 **
 ******************************************************************************/

#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>


/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
#ifndef TRUE
  /** Value is true (boolean_t type) */
  #define TRUE        ((boolean_t) 1u)
#endif

#ifndef FALSE
  /** Value is false (boolean_t type) */
  #define FALSE       ((boolean_t) 0u)
#endif  

#if defined (__ICCARM__)
#define __WEAKDEF            __WEAK __ATTRIBUTES
#elif defined (__CC_ARM)
#define __WEAKDEF            __weak
#else
#error    "unsupported compiler!!"
#endif

/** Returns the minimum value out of two values */
#define MINIMUM( X, Y )  ((X) < (Y) ? (X) : (Y))

/** Returns the maximum value out of two values */
#define MAXIMUM( X, Y )  ((X) > (Y) ? (X) : (Y))

/** Returns the dimension of an array */
#define ARRAY_SZ( X )  (sizeof(X) / sizeof((X)[0]))

#ifdef __DEBUG_ASSERT
    #define ASSERT(x) do{ assert((x)> 0u) ; }while(0);
#else
    #define ASSERT(x) {}
#endif
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

typedef union 
{
    unsigned short u16;
    short i16;
    unsigned char byte[2];
} user_Bit16;

typedef union 
{
    unsigned long u32;
    long i32;
    unsigned char byte[4];
    float flt;
} user_Bit32;

typedef struct 
{
  unsigned long UTC;                    //UTC时间
  unsigned long Battery_Level;          //电池电压 / 3 * 100
  unsigned short Sensor_Min;            //测量下限
  unsigned short Sensor_Max;            //测量上限
  unsigned short port1;                 //端口号 1
  unsigned short port2;                 //端口号 2
  unsigned short Sensor_ScanCount;      //传感器扫描次数
  unsigned short Sensor_ScheduledTimes; //传感器扫描次数满足上传次数
  unsigned short Sensor_Interval;       //传感器扫描间隔，单位 1s
  short Zero_Offset;                    //零点偏移量
  unsigned short LCD_WakeUp_Time;       //LCD点亮时间，单位 10s
  unsigned char Address_IP1[4];         //IP 1
  unsigned char Address_IP2[4];         //IP 2
  unsigned char IMEI[16];               //IMEI号，由SIM卡决定
  unsigned char IMSI[16];               //IMSI号，由SIM卡决定
  unsigned char ID[12];                 //ID
  unsigned char Version_Software;       //软件版本
  unsigned char Version_Hardware;       //硬件版本
  unsigned char Sensor_Type;            //传感器类型
  unsigned char LCD_UpData_Flag;        //LCD唤醒上传有效标志位
  unsigned char Sys_reserve_1;          //保留变量
  unsigned char Sys_reserve_2;          //保留变量
} user_Data;

typedef struct 
{
  float pressureCoefficient;          //压力系数
  unsigned long timeCount;            //低功耗定时器定时次数 时间 = DATA * 10s
  unsigned long User_Delay_Count;     //delay计数
  short TData;                        //采集温度
  user_Bit16 BLE_PData;               //蓝牙采集压力
  unsigned short LCD_Count;           //LCD显示计数
  unsigned char Sensor_State;         //传感器通讯状态
  unsigned char Ethernet_CSQ;         //4G信号强度
  unsigned char LpUart_4G_Timeout;    //4G超时时间
  unsigned char LpUart_4G_Count;      //4G接收缓存地址偏移量
  unsigned char LpUart_BLE_Timeout;   //蓝牙超时时间
  unsigned char LpUart_BLE_Count;     //蓝牙接收缓存地址偏移量
  unsigned char BLE_Idle_Timeout;     //蓝牙空闲时间 时间 = DATA * 10s
  unsigned char LpUart_4G_Send_Busy;  //4G发送忙标志
  unsigned char LpUart_BLE_Busy;      //蓝牙忙标志
  unsigned char Valid_Data_Ready;     //服务器数据接收完成标志位
  unsigned char Valid_Data_Flag;      //接收服务器数据事件发生标志，0：无数据  1：数据接收中
  unsigned char Valid_Data_Count;     //服务器数据缓存地址偏移量
  unsigned char sensorBusy;           //传感器忙标志
  unsigned char ADBusy;               //电量扫描忙标志
  unsigned char ADCount;              //电量扫描计数
  unsigned char LCD_Busy;             //LCD忙标志
  unsigned char BLE_Link_Flag;        //蓝牙连接标志
  unsigned char NET_Send_Type;        //4G上传数据类型
  unsigned char Ethernet_Send_State;  //4G发送进行状态
  unsigned char Delay_Flag;           //等待计数标志
  unsigned char Delay_Through;        //已等待计数
} Timely_Data;

typedef union
{
  user_Data won;
  unsigned long toFlash[21];
} temp_Flash;

/** logical datatype (only values are TRUE and FALSE) */
typedef uint8_t      boolean_t;
  
/** single precision floating point number (4 byte) */
typedef float        float32_t;

/** double precision floating point number (8 byte) */
typedef double       float64_t;

/** ASCII character for string generation (8 bit) */
typedef char         char_t;

/** function pointer type to void/void function */
typedef void         (*func_ptr_t)(void);

/** function pointer type to void/uint8_t function */
typedef void         (*func_ptr_arg1_t)(uint8_t u8Param);

/** generic error codes */
typedef enum en_result
{
    Ok                          = 0u,  ///< No error
    Error                       = 1u,  ///< Non-specific error code
    ErrorAddressAlignment       = 2u,  ///< Address alignment does not match
    ErrorAccessRights           = 3u,  ///< Wrong mode (e.g. user/system) mode is set
    ErrorInvalidParameter       = 4u,  ///< Provided parameter is not valid
    ErrorOperationInProgress    = 5u,  ///< A conflicting or requested operation is still in progress
    ErrorInvalidMode            = 6u,  ///< Operation not allowed in current mode
    ErrorUninitialized          = 7u,  ///< Module (or part of it) was not initialized properly
    ErrorBufferFull             = 8u,  ///< Circular buffer can not be written because the buffer is full
    ErrorTimeout                = 9u,  ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
    ErrorNotReady               = 10u, ///< A requested final state is not reached
    OperationInProgress         = 11u  ///< Indicator for operation in progress
} en_result_t;


/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/

/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/

#endif /* __BASE_TYPES_H__ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/



