#ifndef _MOTOR_h
#define _MOTOR_h
#include <pinMap.h>

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
  int speed;
} control_cmd;

typedef struct
{
  String macAddress;
  String deviceName;
  volatile double x;
  volatile double y;
  volatile int angle;
} car_data;

class Motor{
private:
 uint8_t lpin1,lpin2,rpin1,rpin2,lch1,lch2,rch1,rch2;
 double* speedValue_L;
 double* speedValue_R;
public:
 Motor(double* lspeedValue, double* rpeedValue,uint8_t pin1 = Apin1, uint8_t pin2=Apin2,uint8_t pin3 =Bpin1, uint8_t pin4 =Bpin2,uint8_t ch1 =0,uint8_t ch2 =1,uint8_t ch3=2,uint8_t ch4=3);
 void motor_init();
 void forward();
 void backward();
 void turnLeft();
 void turnRight();
 void rotateLeft();
 void rotateRight();
 void stop();
};
#endif  
