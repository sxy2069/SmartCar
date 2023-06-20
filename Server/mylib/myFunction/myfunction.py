#!/usr/bin/python
#coding:utf-8
  
def translate(value, leftMin, leftMax, rightMin, rightMax):
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin
    valueScaled = float(value - leftMin) / float(leftSpan)
    return rightMin + (valueScaled * rightSpan)


if __name__ == '__main__':
    print(translate(10,0,100,0,1000))