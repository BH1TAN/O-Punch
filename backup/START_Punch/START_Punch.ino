
/*******************************************************
起点点签的代码,实现功能:
1.清空标签
2.写入第一条记录

作者:BH1TAN
时间:20151208
********************************************************/


#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
 
#define IRQ                     (2)
#define RESET                   (3)     // Not connected by default on the NFC Shield
uint8_t success_f, success_r;                         // Flag to check if there was an error with the PN532
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
uint8_t keya[6]={ 0xff,0xff,0xff,0xff,0xff,0xff };

uint32_t versiondata;
uint8_t TempNum1,TempNum2;



/*************标签信息**********************/
uint8_t data[4];    //写入的数据,一个block有4个最大255的uint8_t数据
/*************标签信息**********************/

/*************点签信息**********************/
unsigned long RaceTime;
unsigned long TimeBios = 0;
uint8_t time[4];
/*************点签信息**********************/


Adafruit_NFCShield_I2C nfc(IRQ);

/*********************************函数起始************************************/

void GetTime(void)
{
  RaceTime=millis();
  time[1]=RaceTime/3600000;      //时
  time[2]=(RaceTime/60000)%60;   //分
  time[3]=(RaceTime/1000)%60;    //秒
  //time[0]=(RaceTime%1000)/100;   
}

void SerialPrintTime(void)
{
  GetTime();
  Serial.print(RaceTime);
  Serial.print("*");
  Serial.print(time[1]);
  Serial.print(":");
  Serial.print(time[2]);
  Serial.print(":");
  Serial.print(time[3]);
  Serial.print("#");
  Serial.print(time[0]);
  Serial.println("");
}

void formatnfc(void)
{
  for(TempNum1=0;TempNum1<4;TempNum1++)
    data[TempNum1] = 0;
    
  for(TempNum1=4;TempNum1<50;TempNum1++)
  {
    success_r = nfc.mifareclassic_AuthenticateBlock (uid, uidLength,TempNum1, 0, keya);
    if(!success_r)
    {
      Serial.print("Cant authenticate block.\n");
      return;
    }
    else
    {
      success_r = nfc.mifareclassic_WriteDataBlock (TempNum1,data);
      if(!success_r)
      {
        Serial.print("Cant write to block");
        Serial.print(TempNum1);
        Serial.print(' '); 
        break;
      }
      else
      {
        Serial.print("CLEAR");
        Serial.print(TempNum1);
        Serial.println(' ');
      }
    }
  }
}


void writenfc(void){
  success_r = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(!success_r)
  {
    Serial.print("Cant read Passive TargetID\n");  
    return;
  }
  else
  {
    formatnfc();
    success_r = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 5, 0, keya);
    if (!success_r)
    {
      Serial.println("Can not access normal block.");
      return;
    }
    //formatnfc();    //可在确认普通block后清空也可在确认前清空
    data[0] = 5;    //指向block5
    data[1] = 1;    //起点
    data[2] = 0;
    data[3] = 0;
    
    success_r = nfc.mifareclassic_WriteDataBlock(7,data);
    if (success_r)
      Serial.println("TAG BLOCK2 ACTIVED");
    else
      Serial.println("error:Can not write to block4 \n");
    
    GetTime();
    data[0]=1;    //起点
    data[1]=time[1];  //时
    data[2]=time[2];  //分
    data[3]=time[3];  //秒
    success_r = nfc.mifareclassic_WriteDataBlock(5,data);
    if (success_r)
      Serial.println("RACE BEGIN");
    else
      Serial.println("error:Can not write to block5 \n");
   
  }
}

void setup(void) {
  Serial.begin(9600);    //open serial with PC
  Serial1.begin(9600);   //open serial1 with device
  nfc.begin();
  versiondata = nfc.getFirmwareVersion();
  nfc.SAMConfig();    // configure board to read RFID tags
}

void loop(void) {
  writenfc();
  delay(10);
  
}
