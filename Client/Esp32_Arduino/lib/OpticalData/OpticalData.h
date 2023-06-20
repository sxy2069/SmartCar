#ifndef __OPTICALDATA__
#define __OPTICALDATA__

//导入arduino核心头文件
#include "Arduino.h"
#include <PinMap.h>
//typedef unsigned char uint8_t; // 8bit无符号类型
//typedef unsigned long int uint32_t;  //32bit无符号类型

class OpticalData{
    private:
    uint8_t OID_SCK, OID_SDIO;
    double Ycoordinate;
    double Xcoordinate;
    int Angle;
    uint32_t RecvData[2];
    int state;

    protected:
    void WakeUpOID(void);
    void SendCmd(void);
    uint16_t RecvAck(void);
    int bin2angle(int* num);
    int bin2integer(int* num);
    double int2decimal(int* num);
    void ParseData(void);
    void RecvOptData(void);
    void Delay_10us(void);
    void Delay_78us(void);
    void Delay_30ms(void);
    void SendCmd(uint8_t Cmd);
    uint8_t Write_Register(uint16_t Data1,uint16_t Data2);
    
    public:
    OpticalData(uint8_t SCK_PIN = SCK , uint8_t SDIO_PIN = SDIO);
    void begin(void);
    double getYcoordinate(void);
    double getXcoordinate(void);
    int getAngle(void);
    void OpticalDataInit(void);
    void CheckAndReadOpticalData(void);
    unsigned char TransmitCmd(unsigned char Cmd);
    void OIDPowerOffMode(void);
    void OIDWakeUpMode(void);
};

#endif  // __OPTICALDATA__