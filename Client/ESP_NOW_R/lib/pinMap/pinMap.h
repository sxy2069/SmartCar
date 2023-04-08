#ifndef _PINMAP_H
#define _PINMAP_H

#include "Arduino.h"

/*定位模块引脚定义*/
#define SCK 4
#define SDIO 14

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