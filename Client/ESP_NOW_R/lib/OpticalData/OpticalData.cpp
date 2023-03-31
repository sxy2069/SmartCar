#include "OpticalData.h"

// #define OID_SCK 3
// #define OID_SDIO 4

// extern void OIDPowerOn(void);
// extern void OIDPowerOff(void);  

#define USERCMD_POWERDOWNOID  0X57

OpticalData::OpticalData(uint8_t SCK_PIN, uint8_t SDIO_PIN){
  
  OID_SCK=SCK_PIN;
  OID_SDIO=SDIO_PIN;
 
  Xcoordinate = 0;
  Ycoordinate = 0;
  Angle = 0;
  state=0;
}

void OpticalData::begin(void) {
  // put your setup code here, to run once:
  pinMode(OID_SCK,OUTPUT);
  pinMode(OID_SDIO,INPUT);
 // Serial.println(digitalRead(OID_SCK));
  WakeUpOID();
 // OIDWakeUpMode();

}

 void OpticalData::CheckAndReadOpticalData(void)
 {
  state=0;
   //Serial.println(digitalRead(OID_SDIO));
   if(!digitalRead(OID_SDIO))
   {
     RecvOptData();
     ParseData();
   }
 }

double OpticalData::getYcoordinate(void)
{
  if(state==1)
      return Ycoordinate;
  else 
      return -1;
}

double OpticalData::getXcoordinate(void)
{
  if(state==1)
      return Xcoordinate;
  else 
      return -1;

}
int OpticalData::getAngle(void){
  
  //if(!digitalRead(OID_SDIO))
   if(state==1)
      return Angle;
   else 
      return -1;
}

void OpticalData::SendCmd(uint8_t Cmd)
{
  uint8_t i;

  digitalWrite(OID_SCK,HIGH);
  digitalWrite(OID_SDIO,HIGH);// Write control bit  
  pinMode(OID_SDIO,OUTPUT);         
  //Delay_10us();
  digitalWrite(OID_SCK,LOW);
  Delay_10us();

  for(i=0;i<8;i++)
  {
    digitalWrite(OID_SCK,HIGH);   
    if(Cmd & 0x80)
    {
      digitalWrite(OID_SDIO,HIGH);
    }
    else
    {
      digitalWrite(OID_SDIO,LOW);
    }
    
    Delay_10us();
    
    digitalWrite(OID_SCK,LOW);     

    
    Delay_10us();
    
    Cmd <<= 1;
  }
  Delay_10us();
  pinMode(OID_SDIO,INPUT); 
  Delay_78us();
}

uint8_t OpticalData::TransmitCmd(uint8_t Cmd)
{
  uint8_t TxCmd,RxAckCnt;
  
  if(!digitalRead(OID_SDIO))
  {
    RecvOptData();
  }  
  TxCmd = Cmd;
  SendCmd(TxCmd);
  
  RxAckCnt = 0;
  while(1)
  {
    if(!digitalRead(OID_SDIO))
    {
      TxCmd = (uint8_t)RecvAck();
      TxCmd -= 1;
      if(TxCmd == Cmd)
        return 1;
      else
        return 0;
    }
    
    Delay_30ms();
    RxAckCnt += 1;
    if(RxAckCnt >= 10)
      return 0;
  }
  
}

void OpticalData::Delay_10us(void)
{
  delayMicroseconds(5);
}
void OpticalData::Delay_78us(void)
{
  delayMicroseconds(78);
}
void OpticalData::Delay_30ms(void)
{
  delay(30);
  }

  
void OpticalData::RecvOptData(void)
{
  uint8_t i;

  RecvData[0] = 0;
  RecvData[1] = 0;
  
  digitalWrite(OID_SCK,HIGH);
  digitalWrite(OID_SDIO,LOW);
  pinMode(OID_SDIO,OUTPUT);
  Delay_10us();
  digitalWrite(OID_SCK,LOW);
  Delay_10us();
  
  for(i = 0; i < 64; i++)
  {
    digitalWrite(OID_SCK,HIGH);
    pinMode(OID_SDIO,INPUT);
    Delay_10us();
    digitalWrite(OID_SCK,LOW);
   // Serial.println(digitalRead(OID_SDIO));
    if(digitalRead(OID_SDIO))
    {
      if(i < 32)
      {
        RecvData[0] += 1;
      }
      else
      {
        RecvData[1] += 1;
      }
    }
    
    if(i < 31)
    {
      RecvData[0] <<= 1;
    }
    
    if( i >31 && i < 63)
    {
      RecvData[1] <<= 1;
    }

    Delay_10us();
  }
  Delay_78us();
  
  return;
}
void OpticalData::ParseData(void)
{
  int Coordinate[44] = {0};
  int Ydecimal[8] = {0};
  int Xdecimal[8] = {0};
  int Yinteger[14] = {0};
  int Xinteger[14] = {0};
  int BinAngle[9] = {0};
  if(RecvData[0] & 0X80000000){  // bit 63 = 0: Index  = 1: CMD
    //  Serial.println("bit 63=0");
      return;     
      }
  if(RecvData[0] & 0X40000000)  // bit 62 = 1: OID 3
  {
    //Serial.println("bit 62=1");
    if(RecvData[0] & 0X20000000) // bit 61 = 1: Position Code
    {
      state=1;
      // Serial.println("bit 61=1");
        //角度二进制
      if (RecvData[0] & 0X00200000) {
        BinAngle[8] = 1;
      }
      else {
        BinAngle[8] = 0;
      }
      if (RecvData[0] & 0X00100000) {
        BinAngle[7] = 1;
      }
      else {
        BinAngle[7] = 0;
      }
      if (RecvData[0] & 0X00080000) {
        BinAngle[6] = 1;
      }
      else {
        BinAngle[6] = 0;
      }
      if (RecvData[0] & 0X00040000) {
        BinAngle[5] = 1;
      }
      else {
        BinAngle[5] = 0;
      }
      if (RecvData[0] & 0X00020000) {
        BinAngle[4] = 1;
      }
      else {
        BinAngle[4] = 0;
      }
      if (RecvData[0] & 0X00010000) {
        BinAngle[3] = 1;
      }
      else {
        BinAngle[3] = 0;
      }
      if (RecvData[0] & 0X00008000) {
        BinAngle[2] = 1;
      }
      else {
        BinAngle[2] = 0;
      }
      if (RecvData[0] & 0X00004000) {
        BinAngle[1] = 1;
      }
      else {
        BinAngle[1] = 0;
      }
      if (RecvData[0] & 0X00002000) {
        BinAngle[0] = 1;
      }
      else {
        BinAngle[0] = 0;
      }
  //    Serial.print(RecvData[0]);
       
      //Y坐标的小数部分
      if (RecvData[0] & 0X00000800) {
        Ydecimal[7] = 1;
      }
      else {
        Ydecimal[7] = 0;
      }
      if (RecvData[0] & 0X00000400) {
        Ydecimal[6] = 1;
      }
      else {
        Ydecimal[6] = 0;
      }
      if (RecvData[0] & 0X00000200) {
        Ydecimal[5] = 1;
      }
      else {
        Ydecimal[5] = 0;
      }
      if (RecvData[0] & 0X00000100) {
        Ydecimal[4] = 1;
      }
      else {
        Ydecimal[4] = 0;
      }
      if (RecvData[0] & 0X00000080) {
        Ydecimal[3] = 1;
      }
      else {
        Ydecimal[3] = 0;
      }
      if (RecvData[0] & 0X00000040) {
        Ydecimal[2] = 1;
      }
      else {
        Ydecimal[2] = 0;
      }
      if (RecvData[0] & 0X00000020) {
        Ydecimal[1] = 1;
      }
      else {
        Ydecimal[1] = 0;
      }
      if (RecvData[0] & 0X00000010) {
        Ydecimal[0] = 1;
      }
      else {
        Ydecimal[0] = 0;
      }

      //X坐标的小数部分
      if (RecvData[0] & 0X00000008) {
        Xdecimal[7] = 1;
      }
      else {
        Xdecimal[7] = 0;
      }
      if (RecvData[0] & 0X00000004) {
        Xdecimal[6] = 1;
      }
      else {
        Xdecimal[6] = 0;
      }
      if (RecvData[0] & 0X00000002) {
        Xdecimal[5] = 1;
      }
      else {
        Xdecimal[5] = 0;
      }
      if (RecvData[0] & 0X00000001) {
        Xdecimal[4] = 1;
      }
      else {
        Xdecimal[4] = 0;
      }
      if (RecvData[1] & 0X80000000) {
        Xdecimal[3] = 1;
      }
      else {
        Xdecimal[3] = 0;
      }
      if (RecvData[1] & 0X40000000) {
        Xdecimal[2] = 1;
      }
      else {
        Xdecimal[2] = 0;
      }
      if (RecvData[1] & 0X20000000) {
        Xdecimal[1] = 1;
      }
      else {
        Xdecimal[1] = 0;
      }
      if (RecvData[1] & 0X10000000) {
        Xdecimal[0] = 1;
      }
      else {
        Xdecimal[0] = 0;
      }
      //Y坐标的整数部分
      if (RecvData[1] & 0X08000000) {
        Yinteger[13] = 1;
      }
      else {
        Yinteger[13] = 0;
      }
      if (RecvData[1] & 0X04000000) {
        Yinteger[12] = 1;
      }
      else {
        Yinteger[12] = 0;
      }
      if (RecvData[1] & 0X02000000) {
        Yinteger[11] = 1;
      }
      else {
        Yinteger[11] = 0;
      }
      if (RecvData[1] & 0X01000000) {
        Yinteger[10] = 1;
      }
      else {
        Yinteger[10] = 0;
      }
      if (RecvData[1] & 0X00800000) {
        Yinteger[9] = 1;
      }
      else {
        Yinteger[9] = 0;
      }
      if (RecvData[1] & 0X00400000) {
        Yinteger[8] = 1;
      }
      else {
        Yinteger[8] = 0;
      }
      if (RecvData[1] & 0X00200000) {
        Yinteger[7] = 1;
      }
      else {
        Yinteger[7] = 0;
      }
      if (RecvData[1] & 0X00100000) {
        Yinteger[6] = 1;
      }
      else {
        Yinteger[6] = 0;
      }
      if (RecvData[1] & 0X00080000) {
        Yinteger[5] = 1;
      }
      else {
        Yinteger[5] = 0;
      }
      if (RecvData[1] & 0X00040000) {
        Yinteger[4] = 1;
      }
      else {
        Yinteger[4] = 0;
      }
      if (RecvData[1] & 0X00020000) {
        Yinteger[3] = 1;
      }
      else {
        Yinteger[3] = 0;
      }
      if (RecvData[1] & 0X00010000) {
        Yinteger[2] = 1;
      }
      else {
        Yinteger[2] = 0;
      }
      if (RecvData[1] & 0X00008000) {
        Yinteger[1] = 1;
      }
      else {
        Yinteger[1] = 0;
      }
      if (RecvData[1] & 0X00004000) {
        Yinteger[0] = 1;
      }
      else {
        Yinteger[0] = 0;
      }
      //X坐标的整数部分
      if (RecvData[1] & 0X00002000) {
        Xinteger[13] = 1;
      }
      else {
        Xinteger[13] = 0;
      }
      if (RecvData[1] & 0X00001000) {
        Xinteger[12] = 1;
      }
      else {
        Xinteger[12] = 0;
      }
      if (RecvData[1] & 0X00000800) {
        Xinteger[11] = 1;
      }
      else {
        Xinteger[11] = 0;
      }
      if (RecvData[1] & 0X00000400) {
        Xinteger[10] = 1;
      }
      else {
        Xinteger[10] = 0;
      }
      if (RecvData[1] & 0X00000200) {
        Xinteger[9] = 1;
      }
      else {
        Xinteger[9] = 0;
      }
      if (RecvData[1] & 0X00000100) {
        Xinteger[8] = 1;
      }
      else {
        Xinteger[8] = 0;
      }
      if (RecvData[1] & 0X00000080) {
        Xinteger[7] = 1;
      }
      else {
        Xinteger[7] = 0;
      }
      if (RecvData[1] & 0X00000040) {
        Xinteger[6] = 1;
      }
      else {
        Xinteger[6] = 0;
      }
      if (RecvData[1] & 0X00000020) {
        Xinteger[5] = 1;
      }
      else {
        Xinteger[5] = 0;
      }
      if (RecvData[1] & 0X00000010) {
        Xinteger[4] = 1;
      }
      else {
        Xinteger[4] = 0;
      }
      if (RecvData[1] & 0X00000008) {
        Xinteger[3] = 1;
      }
      else {
        Xinteger[3] = 0;
      }
      if (RecvData[1] & 0X00000004) {
        Xinteger[2] = 1;
      }
      else {
        Xinteger[2] = 0;
      }
      if (RecvData[1] & 0X00000002) {
        Xinteger[1] = 1;
      }
      else {
        Xinteger[1] = 0;
      }
      if (RecvData[1] & 0X00000001) {
        Xinteger[0] = 1;
      }
      else {
        Xinteger[0] = 0;
      }

      Angle=bin2angle(BinAngle);
      //Serial.print("角度：");
      //Serial.println(Angle);
      
      int Y_integer = 0;
      Y_integer = bin2integer(Yinteger);
//      Serial.print("Y整数部分：");
//      Serial.println(Y_integer);

      double Y_decimal = 0;
      Y_decimal = int2decimal(Ydecimal);
//      Serial.print("Y小数部分：");
//      Serial.println(Y_decimal, 4);
      if (Ydecimal[7] == 1) {
        Ycoordinate = Y_integer + Y_decimal - 1;
      }
      else {
        Ycoordinate = Y_integer + Y_decimal;
      }
      //Serial.print("Y坐标：");
      //Serial.println(Ycoordinate, 4);

      int X_integer = 0;
      X_integer = bin2integer(Xinteger);
//      Serial.print("X整数部分：");
//      Serial.println(X_integer);

      double X_decimal = 0;
      X_decimal = int2decimal(Xdecimal);
//      Serial.print("X小数部分：");
//      Serial.println(X_decimal, 4);

      if (Xdecimal[7] == 1) {
        Xcoordinate = X_integer + X_decimal - 1;
      }
      else {
        Xcoordinate = X_integer + X_decimal;
      }
      //Serial.print("X坐标：");
     // Serial.println(Xcoordinate, 4);
    }
    else // bit 61 = 0: General Code
    {
      //Serial.println("bit 61=0");
      if(RecvData[0] & 0X10000000) {// bit 60 = 1: Invalid Code
        return;
        }
      else      //  bit 60 = 0: Valid Code
      {
      }
    }
  }
  else // bit 62 = 0: OID2
  {
    if(RecvData[0] & 0x10000000) // bit 60 = 1: Invalid Code
      return;
    else    //  bit 60 = 0:Valid Code
    {
    }
  }
}


//二进制转换为小数部分
double OpticalData::int2decimal(int* num) {
  int intNumber = 0;
  double decimal = 0.0;
  intNumber = num[6] * 64 + num[5] * 32 + num[4] * 16 + num[3] * 8 + num[2] * 4 + num[1] * 2 + num[0] * 1;
  decimal = (double)intNumber / 128.0;
  return decimal;
}
//二进制转换为整数部分
int OpticalData::bin2integer(int* num) {
  int intNumber = 0;
  intNumber = num[13] * 8192 + num[12] * 4096 + num[11] * 2048 + num[10] * 1024 + num[9] * 512 + num[8] * 256 + num[7] * 128 + num[6] * 64 + num[5] * 32 + num[4] * 16 + num[3] * 8 + num[2] * 4 + num[1] * 2 + num[0] * 1;
  return intNumber;
}
//二进制转换成角度值
int OpticalData::bin2angle(int* num) {
  int intNumber = 0;
  intNumber = num[8] * 256 + num[7] * 128 + num[6] * 64 + num[5] * 32 + num[4] * 16 + num[3] * 8 + num[2] * 4 + num[1] * 2 + num[0] * 1;
  return intNumber;
}

uint16_t OpticalData::RecvAck(void)
{
  uint8_t i;
  uint16_t AckValue;
  AckValue = 0;
  digitalWrite(OID_SCK,HIGH);
  digitalWrite(OID_SDIO,LOW);
  pinMode(OID_SDIO,OUTPUT);
  Delay_10us();
  digitalWrite(OID_SCK,LOW);
  Delay_10us();
  
  for(i = 0; i < 16; i++)
  {
    digitalWrite(OID_SCK,HIGH);
    pinMode(OID_SDIO,INPUT);
    Delay_10us();
    digitalWrite(OID_SCK,LOW);
    if(digitalRead(OID_SDIO))
    {
      AckValue +=1;
    }
    
    if(i < 15)
    {
      AckValue <<= 1;
    }
    
    Delay_10us();
  }
  Delay_78us(); 
  return AckValue;
}

uint8_t OpticalData::Write_Register(uint16_t Data1,uint16_t Data2)
{
  //Cmd:0x73, Data1:Register Adddress, Data2: Register Value
  uint16_t ReckValue;
  uint8_t data_H,data_L;
  uint8_t Ret,i;
  uint8_t ErrorCount=0;
  uint8_t TransCmd[7];
  Ret=i=0;
  TransCmd[0]=0x73;
  TransCmd[1]=0x05;
  TransCmd[2]=(Data1>>8);
  TransCmd[3]=Data1;
  TransCmd[4]=(Data2>>8);
  TransCmd[5]=Data2;
  TransCmd[6]=(TransCmd[1]+TransCmd[2]+TransCmd[3]+TransCmd[4]+TransCmd[5]);
  for(i=0;i<7;i++){
    TransmitCmd(TransCmd[i]);
    ReckValue=RecvAck();
//    data_H=(uint8_t)(ReckValue>>8);
//    data_L=(uint8_t)(ReckValue);
//    if(data_L!=(TransCmd[i]+1)){
//      i=i-1;
//      }  
    }
  return Ret;
  }
void OpticalData::WakeUpOID(void)
{
  uint16_t i,ReckValue;
  int recv=0;
  uint8_t data_H,data_L;
  
  pinMode(OID_SCK,OUTPUT);
  digitalWrite(OID_SCK,HIGH); 
  Delay_30ms();
  delay(2000);
  digitalWrite(OID_SCK,LOW); 

  int count = 0;
  //Serial.println(digitalRead(OID_SDIO));
  i = 0;
  while(1)
  {
    
    i += 1;
    if(i >= 0x7FFF)
    {
      //SCK_HIGH;
      i = 0;
      break;
    }
    if(!digitalRead(OID_SDIO))
    {
      RecvOptData();
      if(RecvData[1] == 0x0000FFF8)
      {
        //Serial.println("position code enable");
        delay(1);
        // Write_Register(0Xe15,0);//写寄存器
        // TransmitCmd(0x10);   //打开角度获取命令
        // ReckValue=RecvAck();
//        while(recv==0){
//        TransmitCmd(0x10);   //打开角度获取命令
//        ReckValue=RecvAck();
//        Serial.println(ReckValue);
//        data_H=(uint8_t)(ReckValue>>8);
//        data_L=(uint8_t)(ReckValue);
//        if(data_L==0x11){
//          recv=1;
//          }
//        }      
        TransmitCmd(0x35);//position enable
		/*
        while(1){
        	if (digitalRead(OID_SDIO)){
        		ReckValue=RecvAck();  
        		count = 0 ;
        		break;
        	}
	        else{
	        	count++;
	        	delay(10);
	        }
	        if(count>19){
	        	Serial.println("Init failed!");
	        	TransmitCmd(0x35);
	        	count = 0;
	        }

        }  
		*/
		Delay_30ms();
        TransmitCmd(0x40);//position enable
		/*
        while(1){
        	if (digitalRead(OID_SDIO)){
        		ReckValue=RecvAck();  
        		count = 0 ;
        		break;
        	}
	        else{
	        	count++;
	        	delay(10);
	        }
	        if(count>19){
	        	Serial.println("Init failed!");
	        	TransmitCmd(0x40);
	        	count = 0;
	        }

        } 
		*/
		Delay_30ms();
        TransmitCmd(0x10);   //打开角度获取命令
		/*
         while(1){
        	if (digitalRead(OID_SDIO)){
        		ReckValue=RecvAck();  
        		count = 0 ;
        		break;
        	}
	        else{
	        	count++;
	        	delay(10);
	        }
	        if(count>19){
	        	Serial.println("Init failed!");
	        	TransmitCmd(0x10);
	        	count = 0;
	        }

        } 
		*/
      }
      break;
    }
  }
   //Serial.println("Wakeupmode执行完毕");
}
void OpticalData::OpticalDataInit(void)
{
  //OIDPowerOn();
  
  Delay_30ms();
  
 // SCK_PH_DIS;
 // SDIO_PH_DIS;  
  pinMode(OID_SCK,OUTPUT);
  digitalWrite(OID_SCK,LOW);
  pinMode(OID_SDIO,INPUT);

  digitalWrite(OID_SCK,HIGH);
  //Serial.println("OpticalDataInit执行完毕");
  Delay_30ms();
}

void OpticalData::OIDPowerOffMode(void)
{
  TransmitCmd(USERCMD_POWERDOWNOID);
  //OIDPowerOff();
  pinMode(OID_SCK,OUTPUT);
  digitalWrite(OID_SCK,LOW);
  pinMode(OID_SDIO,OUTPUT);
  digitalWrite(OID_SDIO,LOW);
 // SCK_PH_DIS;
 // SDIO_PH_DIS;
}

void OpticalData::OIDWakeUpMode(void)
{
  OpticalDataInit();
}