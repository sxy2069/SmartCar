// Include Libraries
#include <WiFi.h>
#include <esp_now.h>
#include <TaskScheduler.h>
#include <motor.h>
#include <OpticalData.h>
#include <ArduinoJson.h>
#include <PID_v1.h>
#define _TASK_SLEEP_ON_IDLE_RUN

// PID相关参数
byte encoder0PinALast;
byte encoder0PinCLast;

double durationL, durationR, abs_durationL, abs_durationR; // the number of the pulses
boolean DirectionL, DirectionR;                            // the rotation direction
boolean resultL, resultR; //PID 调节返回值

double SetpointL, SetpointR;   //设置电机速度
double Kp = 0.5, Ki = 0.5, Kd = 0;
double left_speedValue, right_speedValue;   //设置电机速度
PID myPIDL(&abs_durationL, &left_speedValue, &SetpointL, Kp, Ki, Kd, DIRECT);
PID myPIDR(&abs_durationR, &right_speedValue, &SetpointR, Kp, Ki, Kd, DIRECT);

Motor motor(&left_speedValue,&right_speedValue);
// 接收JSON格式数据
boolean beginFlag = 0; //json数据刚开始接收
unsigned char count = 0; //多层json数据计数
boolean stringComplete = false;//json数据接收完毕一帧数据
String inputString = ""; //json字符串

// 声明定位数据读取函数
void dataRead();
void getCor();

//编码器数据采集
void EncoderInitL();
void wheelSpeedL();
void EncoderInitR();
void wheelSpeedR();

//定位模块对象初始化
OpticalData data(4,14);

IPAddress localIP;

car_data pdata;
car_data rdata;
control_cmd cmd = {STOP, 0};

double X_cor;
double Y_cor;
int angle;

//路由器账号和密码
const char *id = "Freedomislife";
const char *psw = "Freedomislife";

//远程服务器地址
const IPAddress serverIP(192, 168, 5, 100); // 欲访问的服务端IP地址
uint16_t serverPort = 5650;
WiFiClient client;

Scheduler ts; // to control your personal task

//任务处理函数
void broadcast();            // 板子之间广播信息
void connectToServer();            // 和上位机交互
void getCameraData();            // 读取定位信息
void motorChangeSpeed(); // 电机调速
void PIDHandle();          // PID调节

Task BroadcastTask(50, TASK_FOREVER, &broadcast, &ts, true);
Task ConnectToServerTask(50, TASK_FOREVER, &connectToServer, &ts, true);
Task GetCameraDataTask(40, TASK_FOREVER, &getCameraData, &ts, true);
Task MotorChangeSpeedTask(100, TASK_FOREVER, &motorChangeSpeed, &ts, true);
Task PIDTask(TASK_IMMEDIATE, TASK_FOREVER, &PIDHandle, &ts, true);

//和上位机通信
void connectToServer()
{
  if (client.connected()) // 尝试访问目标地址
  {
    StaticJsonDocument<150> doc;
    doc["macAddress"] = pdata.macAddress;
    doc["deviceName"] = pdata.deviceName;
    doc["x"] = pdata.x;
    doc["y"] = pdata.y;
    doc["angle"] = pdata.angle;
    char json_string[256];
    serializeJson(doc, json_string);
    client.print(json_string);
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
      Serial.println(inputString);
      DeserializationError error = deserializeJson(doc, inputString.c_str());
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        inputString = "";
        return;
      }
      
      pdata.deviceName = doc["deviceName"].as<String>();
      if (doc["mode"] == "STOP")
      {
        cmd.mode = STOP;
      }
      else if (doc["mode"] == "FORWARD")
      {
        cmd.mode = FORWARD;
      }
      else if (doc["mode"] == "BACKWARD")
      {
        cmd.mode = BACKWARD;
      }
      else if (doc["mode"] == "TURNLEFT")
      {
        cmd.mode = TURNLEFT;
      }
      else if (doc["mode"] == "TURNRIGHT")
      {
        cmd.mode = TURNRIGHT;
      }
      else if (doc["mode"] == "ROTATELEFT")
      {
        cmd.mode = ROTATELEFT;
      }
      else if (doc["mode"] == "ROTATERIGHT")
      {
        cmd.mode = ROTATERIGHT;
      }
      Serial.println(cmd.mode);
      cmd.speed = doc["speed"];
      inputString = "";
    }
  }
  else
  {
    // 连接服务器;
    client.connect(serverIP, serverPort);
  }
}

//获取定位数据
void getCameraData()
{
  getCor();
} 

//电机调节
void motorChangeSpeed()
{
  if (cmd.mode == STOP)
  {
    cmd.speed = 0;
  }
  if (cmd.speed >= 0 && cmd.speed <= 255)
  {
    SetpointR = cmd.speed;
    SetpointL = cmd.speed;
  }
  else
  {
    return;
  }

  switch (cmd.mode)
  {
  case FORWARD:
    motor.forward();
    break;
  case BACKWARD:
    motor.backward();
    break;
  case TURNLEFT:
    motor.turnLeft();
    break;
  case TURNRIGHT:
    motor.turnRight();
    break;
  case ROTATELEFT:
    motor.rotateLeft();
    break;
  case ROTATERIGHT:
    motor.rotateRight();
    break;
  case STOP:
    motor.stop();
    break;
  }
} // 电机调速

//PID控制
void PIDHandle()
{
  abs_durationR = abs(durationR);
  abs_durationL = abs(durationL);
  resultR = myPIDR.Compute(); // PID转换完成返回值为1
  resultL = myPIDL.Compute(); // PID转换完成返回值为1
  if (resultR)
  {
    // Serial.print("PluseR: ");
    // Serial.print(durationR);
    // Serial.print("  speedValue_R::");
    // Serial.println(speedValue_R);
    // Serial.print(durationR);
    durationR = 0; // 计数清零等                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               .0   待下次计数
  }
  if (resultL)
  {
    // Serial.print("PluseL: ");
    // Serial.print(durationL);
    // Serial.print("  speedValue_L::");
    // Serial.print(" ");
    // Serial.println(durationL);
    durationL = 0; // 计数清零等待下次计数
  }
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
  pdata.macAddress = WiFi.macAddress();
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
  motor.motor_init();

  SetpointR = 0; // 设置PID的输出值
  SetpointL = 0; // 设置PID的输出值

  myPIDL.SetMode(AUTOMATIC); // 设置PID为自动模式
  myPIDR.SetMode(AUTOMATIC); // 设置PID为自动模式
  myPIDL.SetSampleTime(20);  // 设置PID采样频率为100ms
  myPIDR.SetSampleTime(20);

  EncoderInitL();
  EncoderInitR();
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
