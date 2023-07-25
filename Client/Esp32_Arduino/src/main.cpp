// Include Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TaskScheduler.h>
#include <Car.h>
#include <PID_v1.h>
#include <OpticalData.h>
#include <ArduinoJson.h>
#define _TASK_SLEEP_ON_IDLE_RUN

// PID相关参数
byte encoder0PinALast;
byte encoder0PinCLast;
// 编码器值
double durationL, durationR, abs_durationL, abs_durationR; // the number of the pulses
// 方向
boolean DirectionL, DirectionR; // the rotation direction


uint32_t startPWM = 90;
uint32_t PWM_Restrict = 1023;
int timeCount = 0;
float DIFF = 0.915;

Car car;
// PID控制器参数
float kp = 0;
float ki = 0;
float kd = 0;
const float setpoint = 100.0; // 设定目标值
// 控制器状态变量
float last_error = 0;
float integral = 0;
// 定义控制时间间隔
const float dt = 0.1;
// 输出限制范围
const int min_output = 0;
const int max_output = 1023;
float control_signal = 0.0;
float pid_control(float current_value) {
  // 计算误差
  float error = setpoint - current_value;
  // 计算积分项
  integral += error * dt;
  // 计算微分项
  float derivative = (error - last_error) / dt;
  // 计算PID输出
  float output = kp * error + ki * integral + kd * derivative;
  // 限制输出范围
  output = constrain(output, min_output, max_output);
  // 更新上一个误差值
  last_error = error;
  return output;
}
// Ziegler-Nichols自动调参算法
void auto_tune() {

  // 停止其他控制操作，只运行自动调参过程
  // 设置初始增益
  kp = 1;
  ki = 0;
  kd = 0;
  // 设置临时采样时间，建议与控制周期相同
  const float sample_time = dt;
  // 计算临界增益和周期
  float kp_critical = 0;
  float period = 0;
  while (kp_critical == 0) {
    // 运行自动调参过程，记录临界增益和周期
    abs_durationL = abs(durationL);
    abs_durationR = abs(durationR);
    float current_value = abs_durationL;
    control_signal = pid_control(current_value);
    car.forward(control_signal, control_signal);
    Serial.print("current_value Init:");
    Serial.println(control_signal);
    // 输出控制信号，控制电机转速
    /*
    analogWrite(motor1Pin, control_signal);
    analogWrite(motor2Pin, control_signal);
    */
    // 更新周期
    period += sample_time;
    // 检查是否出现超调现象
    if (control_signal == max_output) {
      kp_critical = kp;
      break;
    }
    // 等待采样时间
    delay(sample_time * 1000);
    durationL = 0; // 计数清零
    durationR = 0; // 计数清零
  }

  // 计算临界周期和增益
  float ku = kp_critical;
  float tu = period;

  // 使用Ziegler-Nichols方法计算PID参数
  kp = 0.6 * ku;
  ki = 1.2 * ku / tu;
  kd = 0.075 * ku * tu;
}


// 设置电机速度
double left_speedValue, right_speedValue;                               // 设置电机速度
double old_left_speedValue = startPWM, old_right_speedValue = startPWM; // 保存电机速度

uint8_t startFlag = 0;

uint32_t timeCounts = 0;
double   voltage = 0; //电池电压

// 接收JSON格式数据
boolean beginFlag = 0;          // json数据刚开始接收
unsigned char count = 0;        // 多层json数据计数
boolean stringComplete = false; // json数据接收完毕一帧数据
String inputString = "";        // json字符串
uint8_t motorState = 0;
// 声明定位数据读取函数
void dataRead();
void getCor();

// 编码器数据采集
void EncoderInitL();
void wheelSpeedL();
void EncoderInitR();
void wheelSpeedR();
float IncPIDCalc(PID *, int);
// 定位模块对象初始化
OpticalData data;

IPAddress localIP;

CarData pdata;
CarData rdata;
ControlCmd cmd = {STOP, 0};

double X_cor;
double Y_cor;
int angle;

// 路由器账号和密码
const char *id = "Freedomislife";
const char *psw = "Freedomislife";

// 远程服务器地址
const IPAddress serverIP(192, 168, 5, 100); // 欲访问的服务端IP地址
uint16_t serverPort = 5650;
WiFiClient client;

Scheduler ts; // to control your personal task

// 任务处理函数
void broadcast();       // 板子之间广播信息
void connectToServer(); // 和上位机交互
void getCameraData();   // 读取定位信息
void motorControl();    // PID调节

Task BroadcastTask(50, TASK_FOREVER, &broadcast, &ts, true);
Task ConnectToServerTask(50, TASK_FOREVER, &connectToServer, &ts, true);
Task GetCameraDataTask(40, TASK_FOREVER, &getCameraData, &ts, true);
Task MotorControlTask(100, TASK_FOREVER, &motorControl, &ts, true);

PID speed = {0, 0.5, 0.2, 0, 0, 0}; // 速度PID

// 和上位机通信
void connectToServer()
{
  if (client.connected()) // 尝试访问目标地址
  {
    timeCounts++;
    StaticJsonDocument<256> doc;
    doc["deviceType"] = "camera";
    doc["deviceName"] = pdata.deviceName;
    JsonObject root = doc.createNestedObject("value");
    root["x"] = pdata.x;
    root["y"] = pdata.y;
    root["angle"] = pdata.angle;
    char json_string[300];
    serializeJson(doc, json_string);
    client.print(json_string);
    if (timeCounts >= 100)
    {
      doc["deviceType"] = "battery";
      doc["deviceName"] = "battery";
      JsonObject root = doc.createNestedObject("value");
      root["voltage"] = (int)(voltage*1000.0)/1000.0; //保留三位小数
      char json_string[300];
      serializeJson(doc, json_string);
      client.print(json_string);
    }

    while (client.available())
    {
      char inChar = client.read();
      if (inChar == '{' && beginFlag == 0)
      {
        beginFlag = 1;
      }
      if (beginFlag == 1)
      {
        if (inChar == '{')
        {
          count += 1;
        }
        else if (inChar == '}')
        {
          count -= 1;
        }
        inputString += inChar;
        if (count == 0)
        {
          beginFlag = 0;
          stringComplete = true;
        }
      }
    }
    if (stringComplete)
    {
      stringComplete = false;
      // Serial.println(inputString);
      DeserializationError error = deserializeJson(doc, inputString.c_str());
      inputString = "";
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      if (doc["deviceType"] == "motor")
      {
        if (doc["action"] == "indirect")
        {
          motorState = 1;
          if (doc["value"]["mode"] == "STOP")
          {
            cmd.mode = STOP;
          }
          else if (doc["value"]["mode"] == "FORWARD")
          {
            cmd.mode = FORWARD;
          }
          else if (doc["value"]["mode"] == "BACKWARD")
          {
            cmd.mode = BACKWARD;
          }
          else if (doc["value"]["mode"] == "TURNLEFT")
          {
            cmd.mode = TURNLEFT;
          }
          else if (doc["value"]["mode"] == "TURNRIGHT")
          {
            cmd.mode = TURNRIGHT;
          }
          else if (doc["value"]["mode"] == "ROTATELEFT")
          {
            cmd.mode = ROTATELEFT;
          }
          else if (doc["value"]["mode"] == "ROTATERIGHT")
          {
            cmd.mode = ROTATERIGHT;
          }
          cmd.speed = doc["value"]["speed"];
        }
        else if (doc["action"] == "direct")
        {
          motorState = 2;
          left_speedValue = doc["value"]["speedL"];
          right_speedValue = doc["value"]["speedR"];
        }
        else if (doc["action"] == "setName")
        {
          pdata.deviceName = doc["value"]["deviceName"].as<String>();
        }
      }
    }
  }
  else
  {
    // 连接服务器;
    client.connect(serverIP, serverPort);
  }
}

// 获取定位数据
void getCameraData()
{
  getCor();
  double adc_val = analogRead(BATTERYPIN);
  voltage = (((double)adc_val)/4095)*3.3*4;
}

// 电机控制控制
void motorControl()
{
  abs_durationL = abs(durationL);
  abs_durationR = abs(durationR);
  if (motorState == 1)
  {
    if (cmd.speed < 0 || cmd.speed > 1024)
    {
      return;
    }
    motorState = 3;
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
  else if (motorState == 2) //直接控制
  {
    car.directControl((int32_t)left_speedValue, (int32_t)right_speedValue);
  }
  else if (motorState == 3)//PID控制
  {
    if (startFlag == 0)
    {
      /*
      if (cmd.mode == FORWARD || cmd.mode == BACKWARD)
      {
        if (old_left_speedValue < cmd.speed - 20 && old_right_speedValue < cmd.speed - 20)
        {
          left_speedValue = old_left_speedValue + 20;
          right_speedValue = old_right_speedValue + 20;
          // right_speedValue = DIFF*right_speedValue;
        }
        else
        {
          left_speedValue = cmd.speed;
          //right_speedValue = DIFF * right_speedValue;
        }
      }
      else if (cmd.mode == STOP)
      {
        if (old_left_speedValue > 5 && old_right_speedValue > 5)
        {
          left_speedValue = old_left_speedValue - 5;
          right_speedValue = old_right_speedValue - 5;
        }
        else
        {
          left_speedValue = 0;
          right_speedValue = 0;
          startFlag = 1;
        }
      }
      */
      control_signal = pid_control(abs_durationL);
      Serial.print("control_signal");
      Serial.println(control_signal);
      /*
      int dTotal = abs_durationL - abs_durationR;  // 计算两个轮子的转速差
      float gapValue = IncPIDCalc(&speed, dTotal); // 进行PID调速
      right_speedValue = right_speedValue - gapValue; // 调节右轮功率值
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
      */
    }
    switch (cmd.mode)
    {
    case FORWARD:
      car.forward(control_signal, control_signal);
      break;
    case BACKWARD:
      car.backward(left_speedValue, right_speedValue);
      break;
    case TURNLEFT:
      car.turnLeft(0, right_speedValue);
      break;
    case TURNRIGHT:
      car.turnRight(left_speedValue, 0);
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
  }

  durationL = 0; // 计数清零
  durationR = 0; // 计数清零
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
// Formats MAC Address
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
// Called when data is received
{
  // Only allow a maximum of 250 characters in the message + a null terminating byte
  // char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  // int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  // strncpy(buffer, (const char *)data, msgLen);

  // Make sure we are null terminated
  // buffer[msgLen] = 0;

  // Format the MAC address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  memcpy(&rdata, data, sizeof(rdata));
  // Send Debug log message to the serial port
  // Serial.printf("Data from macAddress=%s  ", macStr);
  // Serial.printf("r_x=%4lf  ", rdata.x);
  // Serial.printf("r_y=%4lf  ", rdata.y);
  // Serial.printf("r_angle=%d\n", rdata.angle);
}

void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
// Called when data is sent
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // Serial.print("Last Packet Sent to: ");
  // Serial.println(macStr);
  // Serial.print("Last Packet Send Status: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
// 板子之间传递信息
void broadcast()
// Emulates a broadcast
{
  // Broadcast a message to every device in range
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  // Send message
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&pdata, sizeof(pdata));

  // Print results to serial monitor
  if (result == ESP_OK)
  {
    // Serial.println("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    // Serial.println("ESP-NOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    // Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    // Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    // Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    // Serial.println("Peer not found.");
  }
  else
  {
    // Serial.println("Unknown error");
  }
}

void setup()
{
  // Set up Serial Monitor
  Serial.begin(115200);
  delay(1000);
  data.begin();
  // Set ESP32 in STA mode to begin with
  WiFi.mode(WIFI_AP_STA);
  // Print MAC address
  // Serial.print("MAC Address: ");
  pdata.deviceName = WiFi.macAddress();
  // Serial.println(pdata.mac);
  WiFi.begin(id, psw);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    // Serial.println("正在连接至wifi...");
  }
  localIP = WiFi.localIP();
  // Initialize ESP-NOW
  if (esp_now_init() == ESP_OK)
  {
    // Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    // println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }
  car.init();
  //speed.SetPoint = 0;
  EncoderInitL();
  EncoderInitR();
  auto_tune();
}

void loop()
{
  ts.execute();
}

void dataRead()
{
  data.CheckAndReadOpticalData();
  Y_cor = data.getYcoordinate();
  X_cor = data.getXcoordinate();
  angle = data.getAngle();
}

void getCor()
{
  dataRead();
  if (X_cor < 1500 && Y_cor < 1500)
  {
    if (X_cor > 1 && Y_cor > 1)
    {
      pdata.x = X_cor;
      pdata.y = Y_cor;
      pdata.angle = angle;
      dataRead();
    }
  }
}

IRAM_ATTR void EncoderInitL()
{
  DirectionL = true; // default -> Forward
  pinMode(encoder0pinB, INPUT);
  attachInterrupt(encoder0pinA, wheelSpeedL, CHANGE);
}

void wheelSpeedL()
{
  int LstateL = digitalRead(encoder0pinA);
  if ((encoder0PinALast == LOW) && LstateL == HIGH)
  {
    int valL = digitalRead(encoder0pinB);
    if (valL == LOW && DirectionL)
    {
      DirectionL = false; // Reverse
    }
    else if (valL == HIGH && !DirectionL)
    {
      DirectionL = true; // Forward
    }
  }
  encoder0PinALast = LstateL;

  if (!DirectionL)
    durationL++;
  else
    durationL--;
}

IRAM_ATTR void EncoderInitR()
{
  DirectionR = true; // default -> Forward
  pinMode(encoder0pinD, INPUT);
  attachInterrupt(encoder0pinC, wheelSpeedR, CHANGE);
}

void wheelSpeedR()
{
  int LstateR = digitalRead(encoder0pinC);
  if ((encoder0PinCLast == LOW) && LstateR == HIGH)
  {
    int valR = digitalRead(encoder0pinD);
    if (valR == LOW && DirectionR)
    {
      DirectionR = false; // Reverse
    }
    else if (valR == HIGH && !DirectionR)
    {
      DirectionR = true; // Forward
    }
  }
  encoder0PinCLast = LstateR;

  if (!DirectionR)
    durationR++;
  else
    durationR--;
}
