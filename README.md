# SmartCar使用说明
  通过TCP协议控制多辆小车运动，并接收小车发送的信息，通信数据格式为json.
### 环境配置
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
  - 运行程序：
    python main.py

### 小车程序
  在vscode中安装插件Platformio IDE作为开发环境
  见Client文件夹，使用ESP32作为主控，使用esp-NOW组网

### 通信数据格式定义
  - 定位传感器上传服务器数据:
  ```
  {"device": "deviceName",
   "sensor": "carera",
   "value" : {"x": 10,"y":100,"angle":60}
  }
  "device":  小车name，默认为macAddress，可更改
   "sensor": 操作对象
  ```
 - 服务器下发命令格式：
 ```
 {"device":"car1",
 "actuator":"car",
 "action":"update",
 "value":{"mode":"STOP","speed":0} 
 }
  "device":  carName，可设置
  "actuator"：操作对象
  "action":
    "set"
    "get"
    "update"   
  "mode": 
    "STOP"
    "FORWARD"
    "BACKWARD"
    "TURNLEFT"
    "TURNRIGHT"
    "ROTATELEFT"
    "ROTATERIGHT" 
  ```
### todolist
   - 解决两个电机之间因为机械误差而造成的转速不同步情况
   - 两个轮子单独设置一个转速PID控制器，使小车匀速行驶，参数整定
   - 尝试使用转速差PID控制器控制小车走直线，参数整定
   - json 通信数据格式优化
   - 调试界面错误优化
   - 小车程序整理优化
   - 小车各接口例子
   - 下一版本PCB电路优化





