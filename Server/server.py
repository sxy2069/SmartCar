#!/usr/bin/python
#coding:utf-8

import datetime
import os
import sys
import socket
import threading
import json
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import tkinter as tk
import tkinter.ttk as ttk

matplotlib.use('TkAgg')

sendCmd = {"deviceName":"","mode":"STOP","speed":1}

def translate(value, leftMin, leftMax, rightMin, rightMax):
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin
    valueScaled = float(value - leftMin) / float(leftSpan)
    return rightMin + (valueScaled * rightSpan)
  
def handle(ip_port,new_client):
    json_data= bytearray()
    startFlag = False 
    try:
      while  True:
            rece_data = new_client.recv(1)
            if rece_data:
              if rece_data ==b'{' and startFlag == False:
                startFlag = True
                json_data = json_data + rece_data
              elif rece_data ==b'}' and startFlag == True:
                startFlag = False
                json_data = json_data + rece_data
                CameraDataString = json_data.decode('ascii')
                CameraData = json.loads(CameraDataString)
                x= round(CameraData['x'], 2)
                y= round(CameraData['y'], 2)
                x= translate(int(x),500,1000,0,1000)
                y = translate(int(y),0,500,0,1000)
                deviceName = CameraData['deviceName']
                if deviceName == "":
                  clientdict[ip_port[0]]['motorData'][0] = CameraData['macAddress']
                else:
                  clientdict[ip_port[0]]['motorData'][0] = deviceName
                clientdict[ip_port[0]]['motorData'][1] = x
                clientdict[ip_port[0]]['motorData'][2] = y
                clientdict[ip_port[0]]['motorData'][3] = CameraData['angle']
                res = "LocalIP is {},point_x is {},point_y is {},angle is {}".format(ip_port[0],x,y,CameraData['angle'])
                json_data = b''
              else:
                if startFlag == True:
                  json_data= json_data + rece_data
            else:
              print("client Closed")
              clientdict[ip_port[0]]['lable'].remove()
              clientdict.pop(ip_port[0],None)
              set_optionmenu(list(clientdict))
              break
    except TimeoutError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
        print(e)
        print ("client timeOut Closed")
    except OSError as e:
        #new_client.close()
        #clientdict.pop(ip_port[0],None)
        #set_optionmenu(list(clientdict))
        print ("lj")
    except ConnectionResetError as e:
        #new_client.close()
        #clientdict.pop(ip_port[0],None)
        #set_optionmenu(list(clientdict))
        print("zj")
                
def set_optionmenu(opl:list):
    clientList.delete(0, "end") 
    for op in opl:
        clientList.insert('end', op)
    clientList.insert(0, "BROADCAST")
              
def tcpServer():
    tcp_server_socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,True)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1) #开启keep-alive
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 3) #空闲多久发送keep-alive包
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 1) #多久循环一次
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3) # 最大重试次数，这里所示：如果发了10次keep-alive包，都没有收到响应，就可以认为连接已经断开
    tcp_server_socket.bind(('192.168.5.100',5650))
    tcp_server_socket.listen(10)
    while True:
      new_client , ip_port = tcp_server_socket.accept()
      clientdict[ip_port[0]]={}
      clientdict[ip_port[0]]['client']= new_client
      clientdict[ip_port[0]]['motorData']= [0,0,0,0]
      clientdict[ip_port[0]]['lable']= f_plot.annotate('',xy=(0,0))
      set_optionmenu(list(clientdict))
      sub_thread = threading.Thread(target=handle,args=(ip_port,new_client))
      sub_thread.setDaemon(True)
      sub_thread.start()
    tcp_server_socket.close()
                  
def update(num):
  global tk_x
  global tk_y
  tk_x =[]
  tk_y =[]
  if len(clientdict): 
    for value in clientdict.values():
      tk_x.append(value['motorData'][1])
      tk_y.append(value['motorData'][2])
      value['lable'].set(x=value['motorData'][1]-70,y=value['motorData'][2]-20,text='name:{}{}x:{},y:{},angle:{}'.format(value['motorData'][0],'\n',value['motorData'][1],value['motorData'][2],value['motorData'][3]))
  data = [[x, y] for x, y in zip(tk_x,tk_y)]
  if len(data):
    scat.set_offsets(data)
  else:
    scat.set_offsets([[0,0],[0,0]])
  return scat,

def cmdSend():
    if clientList.curselection():
      ipAddress = clientList.get(clientList.curselection())
      clientHost = clientdict.get(ipAddress)
      if clientHost:
        try:
          clientHost['client'].send(json.dumps(sendCmd).encode())
        except ConnectionResetError as e:
          clientHost['client'].close()
          clientHost['lable'].remove()
          clientdict.pop(ipAddress,None)
          set_optionmenu(list(clientdict))
          print ("567")
      elif(ipAddress =="BROADCAST"):
        if clientdict:
          for x,y in clientdict.items():
            try:  
              y['client'].send(json.dumps(sendCmd).encode())
            except ConnectionResetError as e:
              y['client'].close()
              y['lable'].remove()
              clientdict.pop(x,None)
              set_optionmenu(list(clientdict))
              print ("789")
        else:
          clientList.delete(0, "end")
    
def forward():
  sendCmd["mode"] = "FORWARD"
  cmdSend()
  
def backward():
  sendCmd["mode"] = "BACKWARD"
  cmdSend()
  
def turnLeft():
  sendCmd["mode"] = "TURNLEFT"
  cmdSend()
  
def turnRight():
  sendCmd["mode"] = "TURNRIGHT"
  cmdSend()
  
def rotateLeft():
  sendCmd["mode"] = "ROTATELEFT"
  cmdSend()
  
def rotateRight():
  sendCmd["mode"] = "ROTATERIGHT"
  cmdSend()
  
def stop():
    sendCmd["mode"] = "STOP"
    cmdSend()
    
def updateData():
  speed = entry.get()
  try:
    speed = int(speed)
    if(speed>=0 and speed <=0xff):
      sendCmd["speed"] = speed
      cmdSend()
  except ValueError as e:
    print(e)
    
def updateName():
  value = entryName.get()
  try:
    sendCmd["deviceName"] = value
    cmdSend()
  except ValueError as e:
    print(e)
   
if __name__ == '__main__':
  clientdict={}
  tk_x =[]
  tk_y =[]
  
  root = tk.Tk()
  root.title("Motor_ControlGUI")
  root.geometry('1600x1200')
  
  fig = plt.figure(figsize=(12,8))
  f_plot =fig.add_subplot(111)#划分区域
  plt.tight_layout()#使画布尽可能大
  f_plot.set_xlabel('point_x')
  f_plot.set_ylabel('point_y')
  f_plot.set_title('CameraImage')
  lj = f_plot.annotate('',xy=(0,0))
  plt.axis([0,1000,0,1000])                          
  scat = plt.scatter(tk_x,tk_y,s=100,c='y')
  ax = plt.gca()                                 #获取到当前坐标轴信息
  ax.xaxis.set_ticks_position('top')   #将X坐标轴移到上面
  ax.invert_yaxis()  
  ani = animation.FuncAnimation(fig=fig,func=update,frames=10,interval=20)
  
  canvas_spice = FigureCanvasTkAgg(fig,root)
  canvas_spice.get_tk_widget().place(x=10,y=150)
  canvas_spice.draw()
  
  b0 = tk.Button(root,text="前进",command=forward)
  b0.pack()
  b0.place(x=300,y=30)
  
  b1 = tk.Button(root,text="后退",command=backward)
  b1.pack()
  b1.place(x=400,y=30)
  
  b2 = tk.Button(root,text="左转",command=turnLeft)
  b2.pack()
  b2.place(x=500,y=30)
  
  b3 = tk.Button(root,text="右转",command=turnRight)
  b3.pack()
  b3.place(x=600,y=30)
  
  b6 = tk.Button(root,text="停止",command=stop)
  b6.pack()
  b6.place(x=700,y=30)
  
  b4 = tk.Button(root,text="原地左转",command=rotateLeft)
  b4.pack()
  b4.place(x=300,y=90)
  
  b5 = tk.Button(root,text="原地右转",command=rotateRight)
  b5.pack()
  b5.place(x=400,y=90)
  
  b7 = tk.Button(root,text="更新速度",command=updateData)
  b7.pack()
  b7.place(x=500,y=70)
  
  entry = tk.Entry(root, show=None)
  entry.pack()
  entry.place(x=560,y=75)
  
  b8 = tk.Button(root,text="设置名字",command=updateName)
  b8.pack()
  b8.place(x=500,y=100)
  
  entryName = tk.Entry(root, show=None)
  entryName.pack()
  entryName.place(x=560,y=105)
  
  clientLabel = tk.Label(root,text='客户端IP')
  clientLabel.place(x=970, y=15)
  
  v = tk.StringVar(root)
  clientList = tk.Listbox(root, height=5, width=50)
  clientList.place(x=850, y=40)
  
  tcp_thread = threading.Thread(target=tcpServer)
  tcp_thread.setDaemon(True)
  tcp_thread.start()
  
  root.mainloop()  # 进入事件循环
  

    
