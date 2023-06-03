#include <Car.h>

Car::Car(uint8_t ch1, uint8_t ch2, uint32_t frequency, uint8_t resolution)
{
  lch2 = ch1;
  rch2 = ch2;
  freq = frequency;
  bit_num = resolution;
}
/*电机引脚初始化*/
void Car::init()
{
  pinMode(Apin1, OUTPUT);
  pinMode(Bpin1, OUTPUT);
  ledcSetup(lch2, freq, bit_num);
  ledcSetup(rch2, freq, bit_num);
  ledcAttachPin(Apin2, lch2);
  ledcAttachPin(Bpin2, rch2);
  ledcWrite(lch2, 0);
  ledcWrite(rch2, 0);
}

/*电机前进*/
void Car::forward(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, LOW);
  digitalWrite(Bpin1, LOW);
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*电机后退*/
void Car::backward(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, HIGH);
  digitalWrite(Bpin1, HIGH);
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*电机左转*/
void Car::turnLeft(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, LOW);
  digitalWrite(Bpin1, LOW);
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*电机右转*/
void Car::turnRight(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, LOW);
  digitalWrite(Bpin1, LOW);
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*原地左转*/
void Car::rotateLeft(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, LOW);
  digitalWrite(Bpin1, HIGH);
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*原地右转*/
void Car::rotateRight(uint32_t speedValue_L, uint32_t speedValue_R)
{
  digitalWrite(Apin1, HIGH); // 正转
  digitalWrite(Bpin1, LOW);  // 正转
  ledcWrite(lch2, speedValue_L);
  ledcWrite(rch2, speedValue_R);
}

/*电机停止*/
void Car::stop()
{
  digitalWrite(Apin1, LOW); // 反转
  digitalWrite(Bpin1, LOW); // 正转
  ledcWrite(lch2, 0);
  ledcWrite(rch2, 0);
}

/*直接控制*/
void Car::directControl(int32_t speedValue_L, int32_t speedValue_R)
{
  if (speedValue_L < 0)
  {
    digitalWrite(Apin1, HIGH); // 反转
  }
  else
  {
    digitalWrite(Apin1, LOW); // 反转
  }
  if (speedValue_R < 0)
  {
    digitalWrite(Bpin1, HIGH);
  }
  else
  {
    digitalWrite(Bpin1, LOW);
  }
  ledcWrite(lch2, abs(speedValue_L));
  ledcWrite(rch2, abs(speedValue_R));
}