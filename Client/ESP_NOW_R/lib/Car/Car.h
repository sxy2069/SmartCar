#ifndef _MOTOR_H
#define _MOTOR_H

#include "Arduino.h"
#include <PinMap.h>

typedef enum
{
  STOP,
  FORWARD,
  BACKWARD,
  TURNLEFT,
  TURNRIGHT,
  ROTATELEFT,
  ROTATERIGHT
} motorMode;

typedef struct
{
  motorMode mode;
  uint32_t speed;
} ControlCmd;

typedef struct
{
  String deviceName;
  volatile double x;
  volatile double y;
  volatile int angle;
} CarData;

class Car
{
private:
  uint8_t lch1, lch2, rch1, rch2;
  uint32_t freq;
  uint8_t bit_num;

public:
  Car(uint8_t ch1 = 0, uint8_t ch2 = 1, uint32_t frequency = 5000, uint8_t resolution = 10);
  void init();
  void forward(uint32_t, uint32_t);
  void backward(uint32_t, uint32_t);
  void turnLeft(uint32_t,uint32_t);
  void turnRight(uint32_t,uint32_t);
  void rotateLeft(uint32_t, uint32_t);
  void rotateRight(uint32_t, uint32_t);
  void stop();
  void directControl(int32_t, int32_t);
};
#endif
