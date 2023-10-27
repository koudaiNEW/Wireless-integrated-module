#ifndef __UART_USER_H__
#define __UART_USER_H__

#define LPMODE_POWER         1
#define LPMODE_USUAL         0

#define LP_NET               1
#define LP_BLE               0

void BleAnd4GLpUartInit(unsigned char LpUartNumber,unsigned char LpMode);
void LpUartTimeOut(void);
void App_LpUart0PortCfg(void);
void App_LpUart1DeInit(void);
unsigned short CRC16( unsigned char *puchMsg,  unsigned short usDataLen) ;

#endif
