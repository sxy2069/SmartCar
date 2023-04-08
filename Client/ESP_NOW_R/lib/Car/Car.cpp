#include <Car.h>

Car::Car(uint8_t ch1, uint8_t ch2, uint8_t ch3, uint8_t ch4,uint32_t frequency,uint8_t resolution)
{
  lch1 = ch1;
  lch2 = ch2;
  rch1 = ch3;
  rch2 = ch4;
  freq = frequency;
  bit_num = resolution;
}
/*电机引脚初始化*/
void Car::init()
{
  ledcSetup(lch1, freq, bit_num);
  ledcSetup(lch2, freq, bit_num);
  ledcSetup(rch1, freq, bit_num);
  ledcSetup(rch2, freq, bit_num);
  ledcAttachPin(Apin1, lch1);
  ledcAttachPin(Apin2, lch2);
  ledcAttachPin(Bpin1, rch1);
  ledcAttachPin(Bpin2, rch2);
}

/*电机前进*/
void Car::forward(uint32_t speedValue_L, uint32_t speedValue_R)
{
  ledcWrite(lch2, speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, speedValue_R);
  ledcWrite(rch2, 0);
}

/*电机后退*/
void Car::backward(uint32_t speedValue_L, uint32_t speedValue_R)
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, speedValue_L);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, speedValue_R);
}

/*电机左转*/
void Car::turnLeft(uint32_t speedValue_R)
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, speedValue_R);
  ledcWrite(rch2, 0);
}

/*电机右转*/
void Car::turnRight(uint32_t speedValue_L)
{
  ledcWrite(lch2, speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, 0);
}

/*原地左转*/
void Car::rotateLeft(uint32_t speedValue_L, uint32_t speedValue_R)
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, speedValue_L);
  ledcWrite(rch1, speedValue_R);
  ledcWrite(rch2, 0);
}

/*原地右转*/
void Car::rotateRight(uint32_t speedValue_L, uint32_t speedValue_R)
{
  ledcWrite(lch2, speedValue_L);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, speedValue_R);
}

/*电机停止*/
void Car::stop()
{
  ledcWrite(lch2, 0);
  ledcWrite(lch1, 0);
  ledcWrite(rch1, 0);
  ledcWrite(rch2, 0);
}