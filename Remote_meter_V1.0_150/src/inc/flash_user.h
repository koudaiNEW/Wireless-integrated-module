#ifndef __FLASH_USER_H__
#define __FLASH_USER_H__

#define Flash_Address_Ethernet                    0x0000FA00
#define Flash_Address_PowerOn                     0x0000FC00
#define Flash_Address_UserData                    0x0000FE00
#define Flash_Address_ScanData                    0x00010000

void Flash_User_Write(void);
void Flash_Power_Init(void);
void Flash_PData_Write(unsigned short PData);
unsigned short Flash_PData_Read(unsigned short number);
void Default_Setting(unsigned char mode);
void Flash_User_Read(void);

#endif
