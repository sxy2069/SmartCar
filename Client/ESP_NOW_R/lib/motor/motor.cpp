#include "Arduino.h"
#include <motor.h>

Motor::Motor(double* lspeedValue,double* rpeedValue,uint8_t pin1, uint8_t pin2,uint8_t pin3, uint8_t pin4,uint8_t ch1,uint8_t ch2,uint8_t ch3,uint8_t ch4){
  lpin1 = pin1;
  lpin2 = pin2;
  rpin1 = pin3;
  rpin2 = pin4;
  lch1 =ch1;
  lch2 =ch2;
  rch1 =ch3;
  rch2 =ch4;
  speedValue_L = lspeedValue;
  speedValue_R =rpeedValue;
}
/*电机引脚初始化*/
void Motor::motor_init()
{
  ledcSetup(lch1, 5000, 8);
  ledcSetup(lch2, 5000, 8);
  ledcSetup(rch1, 5000, 8);
  ledcSetup(rch2, 5000, 8);
  ledcAttachPin(lpin1, lch1);
  ledcAttachPin(lpin2, lch2);
  ledcAttachPin(rpin1, rch1);
  ledcAttachPin(rpin2, rch2);
}

/*电机前进*/
void Motor::forward()
{
  ledcWrite(lch2, *speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, *speedValue_R);
  ledcWrite(rch2, 0);
}

/*电机后退*/
void Motor::backward()
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, *speedValue_L);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, *speedValue_R);
}

/*电机左转*/
void Motor::turnLeft()
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, *speedValue_R);
  ledcWrite(rch2, 0);
}

/*电机右转*/
void Motor::turnRight()
{
  ledcWrite(lch2, *speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, 0);
}

/*原地左转*/
void Motor::rotateLeft()
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, *speedValue_L);
  ledcWrite(rch1, *speedValue_R);
  ledcWrite(rch2, 0);
}

/*原地右转*/
void Motor::rotateRight()
{
  ledcWrite(lch2, *speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, *speedValue_R);
}

/*电机停止*/
void Motor::stop()
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, 0);
}