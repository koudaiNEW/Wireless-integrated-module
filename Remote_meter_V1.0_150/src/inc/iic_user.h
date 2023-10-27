#ifndef __IIC_USER_H__
#define __IIC_USER_H__

void SensorIICInit(void);
void SensorIICDeInit(void);
unsigned char WriteSensor(unsigned char RedAdd,unsigned char *DATA,unsigned char count);
unsigned char ReadSensor(unsigned char RedAdd,unsigned char *DATA,unsigned char count);
void LCDIICInit(void);
void LCDIICDeInit(void);
unsigned char LCD_Write(unsigned char *DATA,unsigned char count);

#endif
