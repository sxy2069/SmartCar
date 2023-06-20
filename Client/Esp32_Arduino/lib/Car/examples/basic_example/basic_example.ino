#include <Car.h>

Car car(0,1,2,3);
//Car car;
//Car car =Car();

void setup()
{
  car.init();
  car.forward(255, 255);
  delay(5000);
  car.stop();
}

void loop()
{
}