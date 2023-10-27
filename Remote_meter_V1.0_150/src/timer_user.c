#include "bt.h"
#include "gpio.h"
#include "flash.h"
#include "uart_user.h"
#include "lpuart.h"
#include "lptim.h"
#include "lpm.h"
#include "lcd_user.h"
#include "ble_user.h"
#include "lpm.h"
#include "wdt.h"

extern user_Data External_Data;
extern Timely_Data System_Data;

//TMIER0中断服务，10ms
void Tim0_IRQHandler(void)
{
    Bt_ClearIntFlag(TIM0,BtUevIrq); //中断标志清零
    LpUartTimeOut();
    if(System_Data.Delay_Flag)System_Data.User_Delay_Count += 10;
    else System_Data.User_Delay_Count = 0;
}

//LPTIMER0/1中断服务, LPT0 10s, LPT1 100ms
void LpTim0_IRQHandler(void)
{
    //LPT1 100ms
    // if(Lptim_GetItStatus(M0P_LPTIMER1) == TRUE)
    // {
    //     Lptim_ClrItStatus(M0P_LPTIMER1); //清除LPTimer的中断标志位
    //     if(System_Data.User_Delay_Count)
    //     {
    //         System_Data.User_Delay_Count--;
    //         if(!System_Data.User_Delay_Count)
    //         {
    //             Lptim_Cmd(M0P_LPTIMER1, FALSE);   //LPT1关闭
    //         }
    //     }
    // }

    //LPT0 10s
    if(Lptim_GetItStatus(M0P_LPTIMER0) == TRUE)
    {
        Lptim_ClrItStatus(M0P_LPTIMER0); //清除LPTimer的中断标志位
        LCD_Timeout();
        BLE_Link_Timeout();
        if(!System_Data.sensorBusy)System_Data.timeCount++;
        if((System_Data.timeCount*10)>=External_Data.Sensor_Interval)   //定时器定时次数到达扫描间隔时间
        {
            System_Data.timeCount = 0;
            System_Data.sensorBusy = 1;    //启动传感器扫描

            //传感器扫描次数到达目标次数，启动电池扫描、4G发送
            if(External_Data.Sensor_ScanCount+1 >= External_Data.Sensor_ScheduledTimes)   
            {
                System_Data.ADBusy = 1;
            }
        }
    }
}

//LPTIMER1中断服务,100ms
// void LpTim1_IRQHandler(void)
// {
//     if(Lptim_GetItStatus(M0P_LPTIMER1) == TRUE)
//     {
//         Lptim_ClrItStatus(M0P_LPTIMER1); //清除LPTimer的中断标志位
//         if(System_Data.User_Delay_Count)
//         {
//             System_Data.User_Delay_Count--;
//             if(!System_Data.User_Delay_Count)
//             {
//                 Lptim_Cmd(M0P_LPTIMER1, FALSE);   //LPT1关闭
//             }
//         }
//     }
//     if(Lptim_GetItStatus(M0P_LPTIMER0) == TRUE)
//     {
//         Lptim_ClrItStatus(M0P_LPTIMER0); //清除LPTimer的中断标志位
//         LCD_Timeout();
//         if(System_Data.LpUart_BLE_Busy==2)System_Data.BLE_Idle_Timeout++;
//         if(!System_Data.sensorBusy)System_Data.timeCount++;
//         if((System_Data.timeCount*10)>=External_Data.Sensor_Interval)   //定时器定时次数到达扫描间隔时间
//         {
//             System_Data.timeCount = 0;
//             System_Data.sensorBusy = 1;    //启动传感器扫描

//             //传感器扫描次数到达目标次数，启动电池扫描、4G发送
//             if(External_Data.Sensor_ScanCount+1 >= External_Data.Sensor_ScheduledTimes)   
//             {
//                 System_Data.ADBusy = 1;
//             }
//         }
//     }
// }


//TIMER0配置 
void Timer0Init(void)
{
    stc_bt_mode0_cfg_t     stcBtBaseCfg;
    DDL_ZERO_STRUCT(stcBtBaseCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); //Base Timer外设时钟使能

    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //定时器模式
    stcBtBaseCfg.enCT       = BtTimer;                      //定时器功能，计数时钟为内部PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv8;                   //PCLK/8
    stcBtBaseCfg.enCntMode  = Bt16bitArrMode;               //自动重载16位计数器/定时器
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    Bt_Mode0_Init(TIM0, &stcBtBaseCfg);                     //TIM0 的模式0功能初始化

    Bt_M0_ARRSet(TIM0, 0xEC77);                        //设置重载值
    Bt_M0_Cnt16Set(TIM0, 0xEC77);                      //设置计数初值

    Bt_ClearIntFlag(TIM0,BtUevIrq);                         //清中断标志   
    Bt_Mode0_EnableIrq(TIM0);                               //使能TIM0中断(模式0时只有一个中断)
    EnableNvic(TIM0_IRQn, IrqLevel3, TRUE);                 //TIM0中断使能

    Bt_M0_Run(TIM0);                                        //TIM0 运行
}

//LPTIMER0配置，10s一次中断唤醒
void LPTimer0Init(void)
{
    stc_lptim_cfg_t    stcLptCfg;
    DDL_ZERO_STRUCT(stcLptCfg);
    ///< 使能LPTIM0 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);

    stcLptCfg.enPrs    = LptimPrsDiv8;
    stcLptCfg.enGate   = LptimGateLow;
    stcLptCfg.enGatep  = LptimGatePLow;
    stcLptCfg.enTcksel = LptimRcl;
    stcLptCfg.enTogen  = LptimTogEnLow;
    stcLptCfg.enCt     = LptimTimerFun;         //计数器功能
    stcLptCfg.enMd     = LptimMode2;            //工作模式为模式2
    stcLptCfg.u16Arr   = 17680;                     //预装载寄存器值
    Lptim_Init(M0P_LPTIMER0, &stcLptCfg);

    Lptim_ClrItStatus(M0P_LPTIMER0);   //清除中断标志位
    Lptim_ConfIt(M0P_LPTIMER0, TRUE);  //允许LPTIMER中断
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE);

    Lptim_Cmd(M0P_LPTIMER0, TRUE);   //运行
}

//LPTIMER1配置，100ms一次中断唤醒，仅代替延时使用
void LPTimer1Init(void)
{
    stc_lptim_cfg_t    stcLptCfg;
    DDL_ZERO_STRUCT(stcLptCfg);
    ///< 使能LPTIM1 外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim1, TRUE);

    stcLptCfg.enPrs    = LptimPrsDiv1;
    stcLptCfg.enGate   = LptimGateLow;
    stcLptCfg.enGatep  = LptimGatePLow;
    stcLptCfg.enTcksel = LptimRcl;
    stcLptCfg.enTogen  = LptimTogEnLow;
    stcLptCfg.enCt     = LptimTimerFun;         //计数器功能
    stcLptCfg.enMd     = LptimMode2;            //工作模式为模式2
    stcLptCfg.u16Arr   = 61888;                     //预装载寄存器值
    Lptim_Init(M0P_LPTIMER1, &stcLptCfg);

    Lptim_ClrItStatus(M0P_LPTIMER1);   //清除中断标志位
    Lptim_ConfIt(M0P_LPTIMER1, TRUE);  //允许LPTIMER中断
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE);

    // Lptim_Cmd(M0P_LPTIMER1, TRUE);   //运行
}

/*
使用LPT1进行睡眠延时
time: 延时时间，单位ms，100ms计
*/
void Delay_Sleep(unsigned long time)
{
    time /= 100;
    if(!time)return;
    System_Data.User_Delay_Count = time;
    Lptim_Cmd(M0P_LPTIMER1, TRUE);   //LPT1运行
    while(System_Data.User_Delay_Count)
    {
        Lpm_GotoDeepSleep(FALSE);
        while(!M0P_SYSCTRL->RCH_CR_f.STABLE);  //RCH稳定
    }
}

//利用看门狗复位MCU
void Sys_Reset(void)
{
    ///< 开启WDT外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralWdt,TRUE);
    ///< WDT 初始化
    Wdt_Init(WdtResetEn, WdtT1ms6);
    ///< 启动 WDT
    Wdt_Start();
    //延时使系统复位
    delay1ms(500);
}


