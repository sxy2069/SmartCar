#!/usr/bin/python
#coding:utf-8

import socket
import threading
import json

from mylib.myFunction.myfunction import *
from mylib.Config.config import *
from mylib.Gui.gui import *

def handle(ip_port,new_client): 
    clientdict[ip_port[0]]={}
    clientdict[ip_port[0]]['client']= new_client
    clientdict[ip_port[0]]['camera']= [0,0,0]
    clientdict[ip_port[0]]['voltage']= 7.4
    clientdict[ip_port[0]]['lable']= f_plot.annotate('',xy=(0,0))
    set_optionmenu(list(clientdict))
    json_data = bytearray()
    startFlag = False
    completeFlag = False
    startCount = 0
    try:
      while  True:
            rece_data = new_client.recv(1)
            if rece_data:
              if rece_data ==b'{' and startFlag == False:
                startFlag = True
              if startFlag == True:
                if rece_data ==b'{':
                  startCount = startCount + 1
                if rece_data == b'}':
                  startCount = startCount - 1
                json_data = json_data + rece_data
                if startCount == 0:
                  startFlag = False
                  completeFlag = True
              if completeFlag == True:
                completeFlag = False
                inputString = json_data.decode('ascii')
                json_data = b''
                inputJson = json.loads(inputString)
                if inputJson['deviceType'] == "camera": #定位器数据
                  x= round(inputJson['value']['x'], 2)
                  y= round(inputJson['value']['y'], 2)
                  #x= translate(int(x),500,1000,0,1000) # 数据范围映射
                 # y = translate(int(y),0,500,0,1000)
                  clientdict[ip_port[0]]['camera'][0] = x
                  clientdict[ip_port[0]]['camera'][1] = y
                  clientdict[ip_port[0]]['camera'][2] = inputJson['value']['angle']
                  res = "LocalIP is {},point_x is {},point_y is {},angle is {}".format(ip_port[0],x,y,inputJson['value']['angle'])
                elif inputJson['deviceType'] == "battery": # 电池电量
                  clientdict[ip_port[0]]['voltage'] = inputJson['value']['voltage']
                  updateVoltage()
            else:
              #clientdict[ip_port[0]]['lable'].remove()
              clientdict.pop(ip_port[0],None)
              set_optionmenu(list(clientdict))
              break
    except TimeoutError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
    except OSError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
    except ConnectionResetError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
              
def tcpServer():
    tcp_server_socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,True)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1) #开启keep-alive
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 3) #空闲多久发送keep-alive包
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 1) #多久循环一次
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3) # 最大重试次数，这里所示：如果发了10次keep-alive包，都没有收到响应，就可以认为连接已经断开
    tcp_server_socket.bind(IPADDRESS)
    tcp_server_socket.listen(10)
    try:
      while True:
        new_client , ip_port = tcp_server_socket.accept()
        sub_thread = threading.Thread(target=handle,args=(ip_port,new_client))
        sub_thread.setDaemon(True)
        sub_thread.start()
    except:
      tcp_server_socket.close()
                 
if __name__ == '__main__':
  tcp_thread = threading.Thread(target=tcpServer)
  tcp_thread.setDaemon(True)
  tcp_thread.start()
  gui()