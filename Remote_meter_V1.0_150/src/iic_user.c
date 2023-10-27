#include "gpio.h"
#include "timer_user.h"

#define SENSOR_SDA_H           Gpio_SetIO(GpioPortA,GpioPin12)
#define SENSOR_SDA_L           Gpio_ClrIO(GpioPortA,GpioPin12)
#define SENSOR_SCL_H           Gpio_SetIO(GpioPortA,GpioPin11)
#define SENSOR_SCL_L           Gpio_ClrIO(GpioPortA,GpioPin11)

#define SENSOR_SDA             Gpio_GetInputIO(GpioPortA,GpioPin12)
// #define SENSOR_SCL             Gpio_GetInputIO(GpioPortA,GpioPin11)

#define LCD_SDA_H              Gpio_SetIO(GpioPortA,GpioPin10)
#define LCD_SDA_L              Gpio_ClrIO(GpioPortA,GpioPin10)
#define LCD_SCL_H              Gpio_SetIO(GpioPortA,GpioPin9)
#define LCD_SCL_L              Gpio_ClrIO(GpioPortA,GpioPin9)

#define LCD_SDA                Gpio_GetInputIO(GpioPortA,GpioPin10)


//传感器IIC引脚初始化
void SensorIICInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
	stcGpioCfg.bOutputVal = TRUE;
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);  ///< 使能GPIO时钟
    Gpio_Init(GpioPortA,GpioPin11,&stcGpioCfg);     //clock
    Gpio_Init(GpioPortA,GpioPin12,&stcGpioCfg);     //data
	Gpio_Init(GpioPortA,GpioPin15,&stcGpioCfg);     //pwr
	delay1ms(1000);
}

//传感器IIC引脚去初始化
void SensorIICDeInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirIn;  //输入
	// stcGpioCfg.enDir = GpioDirOut;  //输出
	// stcGpioCfg.enPu = GpioPuEnable;  //上拉
	stcGpioCfg.enPd = GpioPdEnable;  //下拉
	Gpio_Init(GpioPortA,GpioPin15,&stcGpioCfg);     //pwr
    Gpio_Init(GpioPortA,GpioPin11,&stcGpioCfg);     //clock
    Gpio_Init(GpioPortA,GpioPin12,&stcGpioCfg);     //data
	// SENSOR_SCL_H;
	// SENSOR_SDA_H;
}

//LCDIIC引脚初始化
void LCDIICInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
	stcGpioCfg.bOutputVal = TRUE;
	Gpio_Init(GpioPortA,GpioPin8,&stcGpioCfg);       //pwr
    Gpio_Init(GpioPortA,GpioPin9,&stcGpioCfg);       //clock
    Gpio_Init(GpioPortA,GpioPin10,&stcGpioCfg);      //data
}

//LCDIIC引脚去初始化
void LCDIICDeInit(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirIn;  //输入
	// stcGpioCfg.enPu = GpioPuDisable;  //无上拉
	stcGpioCfg.enPd = GpioPdEnable;  //下拉
	Gpio_Init(GpioPortA,GpioPin8,&stcGpioCfg);       //pwr
    Gpio_Init(GpioPortA,GpioPin9,&stcGpioCfg);       //clock
    Gpio_Init(GpioPortA,GpioPin10,&stcGpioCfg);      //data
}

static void Sensor_SDA_OUT(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin12,&stcGpioCfg);
}

static void Sensor_SDA_IN(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin12,&stcGpioCfg);
}

static void Sensor_Start(void)
{
	SENSOR_SDA_H;
	SENSOR_SCL_H;
	SENSOR_SDA_L;
	SENSOR_SCL_L;
}

static void Sensor_Stop(void)
{
	Sensor_SDA_OUT();
	SENSOR_SDA_L;
	SENSOR_SCL_H;
	SENSOR_SDA_H;
}

static unsigned char Sensor_CheckAsk(void)
{
	unsigned char ack;
	unsigned char i;
	SENSOR_SDA_H;
	Sensor_SDA_IN();
	SENSOR_SCL_H;
	for(i=0;i<10;i++)
	{
		if(SENSOR_SDA==0)
		{
			ack=0;
			break;
		}
		else ack=1;
	}
	SENSOR_SCL_L;
	Sensor_SDA_OUT();
	return ack;
}

static void Sensor_SendAsk(void)
{
  Sensor_SDA_OUT();
  SENSOR_SDA_L;
  SENSOR_SCL_H;
  SENSOR_SCL_L;
  Sensor_SDA_IN();
}

static void Sensor_SendByte(unsigned char DATA)
{
	unsigned char i=0;
	do
	{
		if(DATA&0x80)
		{
			SENSOR_SDA_H;
		}
		else 
		{
			SENSOR_SDA_L;
		}
		SENSOR_SCL_H;
		DATA<<=1;
		i++;
		SENSOR_SCL_L;
	} while (i<8);
}

static void Sensor_GetByte(unsigned char *DATA)
{
  unsigned char temp=0;
  unsigned char i;
  Sensor_SDA_IN();
  for(i=0;i<8;i++)
  {
    SENSOR_SCL_H;
	// delay10us(1);
    if(SENSOR_SDA)temp|=0x01;
    // else temp&=0xFE;
    if(i<7)temp<<=1;
    SENSOR_SCL_L;
  }
  *DATA=temp;
  Sensor_SDA_OUT();
}


static void LCD_SDA_OUT(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin10,&stcGpioCfg);
}

static void LCD_SDA_IN(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin10,&stcGpioCfg);
}

static void LCD_Start(void)
{
	LCD_SDA_H;
	LCD_SCL_H;
	LCD_SDA_L;
	LCD_SCL_L;
}

static void LCD_Stop(void)
{
	LCD_SDA_OUT();
	LCD_SDA_L;
	LCD_SCL_H;
	LCD_SDA_H;
}

static unsigned char LCD_CheckAsk(void)
{
	unsigned char ack;
	unsigned char i;
	LCD_SDA_H;
	LCD_SDA_IN();
	LCD_SCL_H;
	for(i=0;i<2;i++)
	{
		if(LCD_SDA==0)
		{
			ack=0;
			break;
		}
		else ack=1;
	}
	LCD_SCL_L;
	LCD_SDA_OUT();
	return ack;
}

// static void LCD_SendAsk(void)
// {
//   LCD_SDA_OUT();
//   LCD_SDA_L;
//   LCD_SCL_H;
//   LCD_SCL_L;
//   LCD_SDA_IN();
// }

static void LCD_SendByte(unsigned char DATA)
{
	unsigned char i=0;
	do
	{
		if(DATA&0x80)
		{
			LCD_SDA_H;
		}
		else 
		{
			LCD_SDA_L;
		}
		LCD_SCL_H;
		DATA<<=1;
		i++;
		LCD_SCL_L;
	} while (i<8);
}

// static void LCD_GetByte(unsigned char *DATA)
// {
//   unsigned char temp=0;
//   unsigned char i;
//   LCD_SDA_IN();
//   for(i=0;i<8;i++)
//   {
//     LCD_SCL_H;
//     if(LCD_SDA)temp|=0x01;
//     // else temp&=0xFE;
//     if(i<7)temp<<=1;
//     LCD_SCL_L;
//   }
//   *DATA=temp;
//   LCD_SDA_OUT();
// }


//传感器IIC发送
unsigned char WriteSensor(unsigned char RedAdd,unsigned char *DATA,unsigned char count)
{
	Sensor_Start();
	Sensor_SendByte(0xDA);    //从机地址
	if(Sensor_CheckAsk())
	{
		Sensor_Stop();
		return 1;
	}
    Sensor_SendByte(RedAdd);  //从机寄存器地址
	if(Sensor_CheckAsk())
	{
		Sensor_Stop();
		return 2;
	}
	while(count)
	{
		Sensor_SendByte(*DATA);
		if(Sensor_CheckAsk())
		{
			Sensor_Stop();
			return 3;
		}
		DATA--;
		count--;
	}
	Sensor_Stop();
	return 0;
}

//传感器IIC接收
unsigned char ReadSensor(unsigned char RedAdd,unsigned char *DATA,unsigned char count)
{
	Sensor_Start();
	Sensor_SendByte(0xDA);    //从机地址
	if(Sensor_CheckAsk())
	{
		Sensor_Stop();
		return 1;
	}
    Sensor_SendByte(RedAdd);  //从机寄存器地址
	// if(Sensor_CheckAsk())
	// {
	// 	Sensor_Stop();
	// 	return 1;
	// }

    Sensor_Stop();
    Sensor_Start();
	Sensor_SendByte(0xDB);    //从机地址
	if(Sensor_CheckAsk())
	{
		Sensor_Stop();
		return 1;
	}
	while(count)
	{
		Sensor_GetByte(DATA);
        if(count>1)Sensor_SendAsk();
		DATA--;
		count--;
	}
	Sensor_Stop();
	return 0;
}

//LCDIIC发送
unsigned char LCD_Write(unsigned char *DATA,unsigned char count)
{
	LCD_Start();
	LCD_SendByte(0x7C);    //从机地址
	if(LCD_CheckAsk())
	{
		LCD_Stop();
		return 1;
	}
	while(count)
	{
		LCD_SendByte(*DATA);
		if(LCD_CheckAsk())
		{
			LCD_Stop();
			return 2;
		}
		DATA++;
		count--;
	}
	LCD_Stop();
	return 0;
}


