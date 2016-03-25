/*******************************************************
成绩主站的代码,实现功能:
1.打印block2及之后的所有内容

作者:BH1TAN
时间:20151208
********************************************************/


#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>

uint8_t success_f, success_r;                         // Flag to check if there was an error with the PN532
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

/*************标签信息**********************/
uint8_t data[4];    //准备显示的数据,一个block有4个最大255的uint8_t数据
uint8_t EndBlock;
/*************标签信息**********************/
unsigned int TempNum1,TempNum2,TempNum3;

// Create an instance of the NFCShield_I2C class
Adafruit_NFCShield_I2C nfc(2);

/*********************************函数起始************************************/

unsigned short CheckZero(void)     //读到的数据全为0则输出1
{
  if(data[0]==0 && data[1]==0 && data[2]==0 && data[3]==0)
    return 1;
  else
    return 0;
}

void setup(void) {
  Serial.begin(115200);    //open serial with PC
  Serial1.begin(115200);   //open serial1 with device
  nfc.begin();
  nfc.SAMConfig();    // configure board to read RFID tags
  
  EndBlock=50;
}

void loop(void) {
  
    success_r=nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(!success_r)
  {
    return;
  }
  else
  {//007
    //提示标签类型
    Serial.println(' ');
    Serial.print("UID Length(READING): "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytes");
    Serial.print("UID Value(READING): ");
    nfc.PrintHex(uid, uidLength);
  
  
  for(TempNum1=2;TempNum1<63;TempNum1++)      //输出block 2-63 的信息
  {//005
    if(TempNum1 == 2)
    {//006
      Serial.println("*****************START*****************");
    }//006
    success_r = nfc.mifareclassic_ReadDataBlock (TempNum1,data);
    if(!success_r)
      {//001
        Serial.print("\nFailed to read Data block");
        Serial.print(TempNum1);
        Serial.println(' ');
        break;
      }//001
    else
    {//004
      if(CheckZero())   //读到的数据全为0
      {//002
        Serial.println(' ');
        Serial.print("block");
        Serial.print(TempNum1);
        Serial.print('**');
      }  //002
      else
      {//003
        Serial.println(' ');
        Serial.print("Block");
        Serial.print(TempNum1);
        Serial.print('-');
        Serial.print(data[0]);//点签序号
        Serial.print('#');
        Serial.print(data[1]);//时间小时
        Serial.print(':');
        Serial.print(data[2]);//时间分钟
        Serial.print(':');
        Serial.print(data[3]);//时间秒
        Serial.print("<<<<<");
      }//003      
    }//004
  }//005for
  }//007
  
  Serial.println(' '); 
  Serial.println("*****************END********************");
  delay(1000);
  
  
}//loop
