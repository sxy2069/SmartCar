#!/usr/bin/python
#coding:utf-8

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

from mylib.myFunction.myfunction import *


def handle(ip_port,new_client): 
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
                  #y = translate(int(y),0,500,0,1000)
                  clientdict[ip_port[0]]['camera'][0] = x
                  clientdict[ip_port[0]]['camera'][1] = y
                  clientdict[ip_port[0]]['camera'][2] = inputJson['value']['angle']
                  res = "LocalIP is {},point_x is {},point_y is {},angle is {}".format(ip_port[0],x,y,inputJson['value']['angle'])
                elif inputJson['deviceType'] == "battery": # 电池电量
                  clientdict[ip_port[0]]['voltage'] = inputJson['value']['voltage']
                  updateVoltage()
            else:
              print("client Closed")
              print(clientdict)
              clientdict[ip_port[0]]['lable'].remove()
              clientdict.pop(ip_port[0],None)
              set_optionmenu(list(clientdict))
              break
    except TimeoutError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
        #print(e)
    except OSError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
        #print (e)
    except ConnectionResetError as e:
        new_client.close()
        clientdict[ip_port[0]]['lable'].remove()
        clientdict.pop(ip_port[0],None)
        set_optionmenu(list(clientdict))
        #print (e)  
              
def tcpServer():
    tcp_server_socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,True)
    tcp_server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1) #开启keep-alive
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE, 3) #空闲多久发送keep-alive包
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL, 1) #多久循环一次
    tcp_server_socket.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT, 3) # 最大重试次数，这里所示：如果发了10次keep-alive包，都没有收到响应，就可以认为连接已经断开
    tcp_server_socket.bind(IPADDRESS)
    tcp_server_socket.listen(10)
    while True:
      new_client , ip_port = tcp_server_socket.accept()
      clientdict[ip_port[0]]={}
      clientdict[ip_port[0]]['client']= new_client
      clientdict[ip_port[0]]['camera']= [0,0,0]
      clientdict[ip_port[0]]['voltage']= 7.4
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
      tk_x.append(value['camera'][0])
      tk_y.append(value['camera'][1])
      value['lable'].set(x=value['camera'][0]-70,y=value['camera'][1]-20,text='name:{}{}x:{},y:{},angle:{}'.format('daiding','\n',value['camera'][0],value['camera'][1],value['camera'][2]))
  data = [[x, y] for x, y in zip(tk_x,tk_y)]
  if len(data):
    scat.set_offsets(data)
  else:
    scat.set_offsets([[0,0],[0,0]])
  return scat,

def set_optionmenu(opl:list):
    clientList.delete(1, "end") 
    for op in opl:
        clientList.insert(1, op)
    

def cmdSend():
    if clientList.curselection():
      for index in clientList.curselection():
        ipAddress = clientList.get(index)
        clientHost = clientdict.get(ipAddress)
        if clientHost:
          try:
            clientHost['client'].send(json.dumps(sendCmd).encode())
          except ConnectionResetError as e:
            clientHost['client'].close()
            clientHost['lable'].remove()
            clientdict.pop(ipAddress,None)
            set_optionmenu(list(clientdict))
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
        else:
          clientList.delete(0, "end")
    
def carControl(mode):
  global motorMode
  speed = setSpeed.get()
  motorMode = mode
  speed = int(speed)
  if(speed>=0 and speed <=1023):
    sendCmd["deviceType"] = "motor"
    sendCmd["action"] = "indirectControl"
    sendCmd['value']={"mode":motorMode,"speed":speed}
    cmdSend()

def updateSpeed():
  global motorMode
  speed = setSpeed.get()
  speed = int(speed)
  if(speed>=0 and speed <=1023):
    sendCmd["deviceType"] = "motor"
    sendCmd["action"] = "indirectControl"
    sendCmd['value']={"mode":motorMode,"speed":speed}
    cmdSend()
    
def directControl():
  leftSpeed = int(speedL.get())
  rightSpeed = int(speedR.get())
  if(leftSpeed>=-1023 and leftSpeed <=1023 and rightSpeed>=-1023 and rightSpeed <=1023):
    sendCmd["deviceType"] = "motor"
    sendCmd["action"] = "directControl"
    sendCmd["value"] = {"speedL":leftSpeed,"speedR":rightSpeed}
    cmdSend()
  
def updateName():
  name = setName.get()
  sendCmd["deviceType"] = "motor"
  sendCmd["action"] = "setName"
  sendCmd['value']={"deviceName":name}
  cmdSend()

def onSelected(event):
  if clientList.curselection():
      if clientList.curselection()[0]!=0:
        ipAddress = clientList.get(clientList.curselection()[0])
        clientHost = clientdict.get(ipAddress)
        if clientHost:
          voltage = clientHost["voltage"]
          voltageText = str(voltage) + "V"
          voltageNumbers.delete(0, 10)
          voltageNumbers.insert(0, voltageText)  

def updateVoltage():
  if clientList.curselection():
      if clientList.curselection()[0]!=0:
        ipAddress = clientList.get(clientList.curselection()[0])
        clientHost = clientdict.get(ipAddress)
        if clientHost:
          voltage = clientHost["voltage"]
          voltageText = str(voltage) + "V"
          voltageNumbers.delete(0, 10)
          voltageNumbers.insert(0, voltageText)  

         
if __name__ == '__main__':
  IPADDRESS =  ("192.168.5.103",5650)
  sendCmd = {"deviceType":"","action":"","value":{}}
  motorMode = "STOP"
  clientdict={}
  tk_x =[]
  tk_y =[]
  
  matplotlib.use('TkAgg')
  root = tk.Tk()
  root.title("SmartCarGUI")
  root.geometry('1600x1200')
  fig = plt.figure(figsize=(12,8))
  f_plot =fig.add_subplot(111)#划分区域
  plt.tight_layout()#使画布尽可能大
  f_plot.set_xlabel('point_x')
  f_plot.set_ylabel('point_y')
  f_plot.set_title('CameraImage')
  f_plot.annotate('',xy=(0,0))
  plt.axis([0,1600,0,1600])                          
  scat = plt.scatter(tk_x,tk_y,s=100,c='y')
  ax = plt.gca()                                 #获取到当前坐标轴信息
  ax.xaxis.set_ticks_position('top')   #将X坐标轴移到上面
  ax.invert_yaxis()  
  ani = animation.FuncAnimation(fig=fig,func=update,frames=10,interval=20)
  
  canvas_spice = FigureCanvasTkAgg(fig,root)
  canvas_spice.get_tk_widget().place(x=50,y=140)
  canvas_spice.draw()
  
  b0 = tk.Button(root,text="前进", command = lambda:carControl("FORWARD"))
  b0.pack()
  b0.place(x=100,y=20)
  
  b1 = tk.Button(root,text="后退",command = lambda:carControl("BACKWARD"))
  b1.pack()
  b1.place(x=150,y=20)
  
  b2 = tk.Button(root,text="左转",command = lambda:carControl("TURNLEFT"))
  b2.pack()
  b2.place(x=200,y=20)
  
  b3 = tk.Button(root,text="右转",command = lambda:carControl("TURNRIGHT"))
  b3.pack()
  b3.place(x=250,y=20)
  
  b4 = tk.Button(root,text="原地左转",command = lambda:carControl("ROTATELEFT"))
  b4.pack()
  b4.place(x=300,y=20)
  
  b5 = tk.Button(root,text="原地右转",command = lambda:carControl("ROTATERIGHT"))
  b5.pack()
  b5.place(x=370,y=20)
  
  b6 = tk.Button(root,text="停止",command = lambda:carControl("STOP"))
  b6.pack()
  b6.place(x=440,y=20)
  
  b7 = tk.Button(root,text="更新速度",command=updateSpeed)
  b7.pack()
  b7.place(x=100,y=60)
  
  setSpeed = tk.Entry(root, show=None)
  setSpeed.pack()
  setSpeed.place(x=165,y=65)
  setSpeed.insert(0, "200")
  
  b8 = tk.Button(root,text="设置名字",command=updateName)
  b8.pack()
  b8.place(x=100,y=100)
  
  setName = tk.Entry(root, show=None)
  setName.pack()
  setName.place(x=165,y=105)
  
  b9 = tk.Button(root,text="直接控制",command=directControl)
  b9.pack()
  b9.place(x=600,y=20)
 
  lable1 = tk.Label(root,text='左轮速度:')
  lable1.pack()
  lable1.place(x=510,y=60)
  
  lable2 = tk.Label(root,text='右轮速度:')
  lable2.pack()
  lable2.place(x=510,y=100)
  
  speedL = tk.Entry(root, show=None)
  speedL.pack()
  speedL.place(x=570,y=60)
  speedL.insert(0, "200")

  speedR = tk.Entry(root, show=None)
  speedR.pack()
  speedR.place(x=570,y=100)
  speedR.insert(0, "200")

  lable2 = tk.Label(root,text='电量显示:')
  lable2.pack()
  lable2.place(x=730,y=60)
  
  voltageNumbers = tk.Entry(root, show=None)
  voltageNumbers.pack()
  voltageNumbers.place(x=785,y=60)
  
  clientLabel = tk.Label(root,text='客户端IP')
  clientLabel.place(x=1350, y=5)
  
  v = tk.StringVar(root)
  clientList = tk.Listbox(root, height=10, width=20,selectmode=tk.EXTENDED)
  clientList.insert(0, "BROADCAST")
  clientList.bind('<<ListboxSelect>>',onSelected)
  clientList.place(x=1300, y=25)


  tcp_thread = threading.Thread(target=tcpServer)
  tcp_thread.setDaemon(True)
  tcp_thread.start()
  root.mainloop()  # 进入事件循环

  

    
