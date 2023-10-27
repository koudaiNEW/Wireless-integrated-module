#ifndef __SENSOR_USER_H__
#define __SENSOR_USER_H__

#define PMODE    0
#define TMODE    1

void GetK(void);
short GetPTValue(unsigned char type);
void SensorScan(void);

#endif
