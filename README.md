# SmartCar使用说明
  通过TCP协议控制多辆小车运动，并接收小车发送的信息，通信数据格式为json.
### 环境配置
配置使用的路由器：
  - 路由器SSID：Freedomislife
  - 路由器PASW：Freedomislife
  - 路由器地址：192.168.5.1
### 运行服务器端程序
  - 服务器端IP：192.168.5.100
  - 服务器端PORT：5650
###### 主机环境
  - 安装python3
  - 安装python包：
      pip install matplotlib
      pip install numpy
  - 运行上位机程序：
    Server文件夹下，打开终端，运行一下程序：
    `python main.py`

### 小车程序
  在vscode中安装插件Platformio IDE作为开发环境
  见Client文件夹，使用ESP32作为主控，使用esp-NOW组网

### 通信数据格式定义
  - 定位传感器上传服务器数据:
  ```
  {"deviceType": "deviceName",
   "deviceName": "carera",
   "value" : {"x": 10,"y":100,"angle":60}
  }
  "deviceType": 对象类型
   "deviceName": 对象名字
   "value"：    具体数据，如果是电池，则此处定义为{"voltage": 10}
  ```
 - 服务器下发命令格式：
 ```
 {"deviceType":"motor",
  "deviceName": "car1"
 "action":"setName",
 "value":{"mode":"STOP","speed":0} 
 }
  ```
 json数据key含义：
  ```
  ""deviceType"": 对象类型
  "deviceName": 对象名字
  "action":     执行的操作类型，每种对象不一样，以下为电机控制方式
    "setName"  
    "indirectControl"    
    "directControl"  
 "value":  具体发送的数据
  "mode":   间接控制时，小车的模式种类
    "STOP"
    "FORWARD"
    "BACKWARD"
    "TURNLEFT"
    "TURNRIGHT"
    "ROTATELEFT"
    "ROTATERIGHT" 
  ```
### todolist :
   - 解决两个电机之间因为机械误差而造成的转速不同步情况
   - 两个轮子单独设置一个转速PID控制器，使小车匀速行驶，参数整定
   - 尝试使用转速差PID控制器控制小车走直线，参数整定
   - json 通信数据格式优化
   - 调试界面错误优化
   - 小车程序整理优化
   - 小车各接口例子
   - 下一版本PCB电路优化
   - 电量显示，如果可以，最好做到点击哪个小车，即显示那个小车的电量

### PID控制相关:
 - 目前主要目的为控制小车沿直线行走，但由于电机机械特性和PID算法或者参数整定等原因，目前效果不是很理想。
 - 目前的思路有三点：
   1. 在未加入PID运算前，加一个虚参数DIFF，使得小车尽量克服机械误差走直线
   2. 采用增量式PID算法
   3. 采用速度差算法，也就是设定左右轮子的速度差作为输入值，设定值设置为0，目的是希望左右两个轮子的速度差接近于零。下面是相关部分的代码：
 - 设置虚参数部分代码：
这部分还有待验证，设置DIFF的值能够有效控制小车左偏还是右偏
   ```
      right_speedValue = DIFF*right_speedValue;
   ```   
- PID控制器部分代码：
```
void PIDHandle()
{
  abs_durationL = abs(durationL);
  abs_durationR = abs(durationR);
  if (changeSpeedFlag == true)
  {
    if (cmd.speed < 0 || cmd.speed > PWM_Restrict)
    {
      return;
    }
    changeSpeedFlag = false;
    startFlag = 1;
    switch (cmd.mode)
    {
    case TURNLEFT:
      left_speedValue = 0;
      right_speedValue = cmd.speed;
      break;
    case TURNRIGHT:
      left_speedValue = cmd.speed;
      right_speedValue = 0;
      break;
    case STOP:
      left_speedValue = 0;
      right_speedValue = 0;
      break;
    default:
      left_speedValue = cmd.speed;
      right_speedValue = cmd.speed;
      startFlag = 0;
      speed.SetPoint = 0;
    }
  }
  if (startFlag == 0)
  {
    if (cmd.mode == FORWARD || cmd.mode == BACKWARD)
    {
      if (old_left_speedValue < cmd.speed-20 && old_right_speedValue < cmd.speed-20)
      {
        left_speedValue = old_left_speedValue + 20;
        right_speedValue = old_right_speedValue + 20;
        //right_speedValue = DIFF*right_speedValue;
      }else{
        left_speedValue = cmd.speed;
        right_speedValue = DIFF*right_speedValue;
        startFlag =1;  
      }
    }
    else if (cmd.mode == STOP)
    {
      if (old_left_speedValue > 5 && old_right_speedValue > 5)
      {
        left_speedValue = old_left_speedValue - 5;
        right_speedValue = old_right_speedValue - 5;
      }else{
        left_speedValue = 0;
        right_speedValue = 0;
        startFlag =1;
      }
    }
  }
  int dTotal = abs_durationL - abs_durationR;//计算两个轮子的转速差
  float gapValue = IncPIDCalc(&speed, dTotal);//进行PID调速
  //left_speedValue = left_speedValue + gapValue;//调节左轮功率值
  right_speedValue = right_speedValue - gapValue;//调节右轮功率值
  timeCount += 1;//输出调试信息
  //if (timeCount >= 10)
  //{
   // timeCount = 0;
    //Serial.print("abs_durationL::");
    //Serial.println(abs_durationL);
    //Serial.print("abs_durationR::");
    //Serial.println(abs_durationR);
    Serial.print("dTotal::");
    Serial.println(dTotal);
    //Serial.print("gapValue::");
    //Serial.print(" ");
    //Serial.println(gapValue);
    //Serial.print(" ");
    Serial.print("left_speedValue=");
    Serial.println(left_speedValue);
    //Serial.print(" ");
    Serial.print("right_speedValue=");
    Serial.println(right_speedValue);
 // }
  if (left_speedValue < startPWM)
    left_speedValue = startPWM;
  else if (left_speedValue > PWM_Restrict)
    left_speedValue = PWM_Restrict;
  if (right_speedValue < startPWM)
    right_speedValue = startPWM;
  else if (right_speedValue > PWM_Restrict)
    right_speedValue = PWM_Restrict;
  old_left_speedValue = left_speedValue;
  old_right_speedValue = right_speedValue;

  switch (cmd.mode)
  {
  case FORWARD:
    car.forward(left_speedValue, right_speedValue);
    break;
  case BACKWARD:
    car.backward(left_speedValue, right_speedValue);
    break;
  case TURNLEFT:
    car.turnLeft(right_speedValue);
    break;
  case TURNRIGHT:
    car.turnRight(left_speedValue);
    break;
  case ROTATELEFT:
    car.rotateLeft(left_speedValue, right_speedValue);
    break;
  case ROTATERIGHT:
    car.rotateRight(left_speedValue, right_speedValue);
    break;
  case STOP:
    car.stop();
    break;
  }
  durationL = 0; // 计数清零
  durationR = 0; // 计数清零
}
```
- PID控制器定义在PID_v1.h文件中，如下图：
```
  typedef struct
{
  int SetPoint;     // 设定目标 Desired Value
  float Proportion; // 比例常数 Proportional Const
  float Integral;   // 积分常数 Integral  Const
  float Derivative; // 微分常数 Derivative Const
  int LastError;    // Error[-1]
  int PrevError;    // Error[-2]
} PID;

float IncPIDCalc(PID *sptr, int NextPoint)
{
  register int iError;
  register float iIncpid;
  iError = sptr->SetPoint - NextPoint;
  iIncpid = sptr->Proportion * (iError - sptr->LastError)                          // E[k] 项
            + sptr->Integral * iError                                     // E[k－1]项
            + sptr->Derivative * (iError - 2 * sptr->LastError + sptr->PrevError); // E[k－2]项
  sptr->PrevError = sptr->LastError;
  sptr->LastError = iError;
  return (iIncpid);
}

```
### 帮助文档
  - PlatformIO在线说明文档
  <https://docs.platformio.org/en/latest/what-is-platformio.html>
  - PlatformIO中有关ESP32的配置命令
  <https://docs.platformio.org/en/latest/platforms/espressif32.html>
  - github上一些分区表示例
  <https://github.com/espressif/arduino-esp32/tree/master/tools/partitions>





