#ifndef _PINMAP_H
#define _PINMAP_H

#include "Arduino.h"

/*定位模块引脚定义*/
#define SCK 4
#define SDIO 14

//RGB strip_PIN
#define RGB_DATA 2
//SPI
#define MOSI 23
#define MISO 19
#define SCLK 18
#define CS 5
//I2C
#define SDA 2
#define SCL 2

//UART
//串口0即TYPEC接口，用于下载程序以及调试程序

//GPIO15引出用于外设
//GPIO12引出用于外设

//UART2
#define TX2 17
#define RX2 16

/*电机A驱动引脚定义*/
#define Apin1  25
#define Apin2  26
/*电机B驱动引脚定义*/
#define Bpin1  27
#define Bpin2  13
/*电机A霍尔编码器引脚定义*/
#define encoder0pinA  34// A pin -> the interrupt pin 2
#define encoder0pinB 35 // B pin -> the digital pin 4
/*电机B霍尔编码器引脚定义*/
#define encoder0pinC  32 // A pin -> the interrupt pin 3
#define encoder0pinD  33 // B pin -> the digital pin 5
#endif  