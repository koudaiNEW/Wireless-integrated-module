
#include "ddl.h"
#include "lpm.h"
#include "lpuart.h"
#include "sys_user.h"
#include "uart_user.h"
#include "timer_user.h"
#include "iic_user.h"
#include "sensor_user.h"
#include "ble_user.h"
#include "ethernet_user.h"
#include "flash_user.h"
#include "adc_user.h"
#include "lcd_user.h"

extern Timely_Data System_Data;

int main(void)
{
    delay1ms(5000);
    SysInit();
    Flash_Power_Init();
    BleAnd4GLpUartInit(LP_BLE,LPMODE_POWER);
    BleAnd4GLpUartInit(LP_NET,LPMODE_POWER);
    GetK();
    Timer0Init();
    Ethernet_Init();
    BLE_Init();
    Battery_Init();
    LPTimer0Init();
    while(1)
    {
        Battery_Scan();
        SensorScan();
        BLE_Analysis();
        LCD_General();
        Ethernet_Send_Time();

        if(!(System_Data.LpUart_BLE_Busy||System_Data.sensorBusy||System_Data.Valid_Data_Ready||System_Data.ADBusy||System_Data.LCD_Busy))
        {
            Lpm_GotoDeepSleep(FALSE);
            while(!M0P_SYSCTRL->RCH_CR_f.STABLE);  //RCH稳定
        }
    }
}

