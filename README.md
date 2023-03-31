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
     安装python3
     安装python包：
        pip install matplotlib
        pip install numpy
     运行程序：
        python main.py
### 小车程序
  见Client文件夹，使用ESP32作为主控，使用esp-NOW组网


