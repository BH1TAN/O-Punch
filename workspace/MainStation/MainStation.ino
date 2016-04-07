/***********************************************/
/*
 * 20160324于台湾清华仁斋0316
 * microduino板子作为点签器使用
 * 4位UID的mifare classic标签做为指卡使用
 * 
 * */
#include<Wire.h>
#include<Adafruit_NFCShield_I2C.h>
#include<Adafruit_NeoPixel.h>

#define IRQ 2
#define LEDPIN 6
#define BUZZERPIN 8

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NFCShield_I2C nfc(IRQ);
unsigned short BlockNum=1;
unsigned short TempNum1,TempNum2;
unsigned short Order,ReadFlag;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t data[16];
uint8_t Block1Data[16];

/************************************
 * personal function
 */

 
/**********************************
 * end of personal function
 */

void setup() {
  Serial.begin(115200);//与电脑通信，不能太小
  nfc.begin();
  nfc.SAMConfig();
  pinMode(BUZZERPIN,OUTPUT);
  strip.begin();
  strip.show();

}

void loop() {
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
    Order = 1 ;
    ReadFlag = 0;
    //display basic information about the tag
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength,1, 0, keya);
    success = nfc.mifareclassic_ReadDataBlock(1,Block1Data);
    for(TempNum1=2;TempNum1<Block1Data[0]+1;TempNum1++){
      //AuthenticateBlock
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, TempNum1, 0, keya);
      if(success){
        //读取
        success = nfc.mifareclassic_ReadDataBlock(TempNum1, data);
        if (success){
          /*下面注释代码调试用
          Serial.print("Block");
          Serial.print(TempNum1,DEC);
          Serial.print(":");
          nfc.PrintHexChar(data, 16);
          */
          if((TempNum1%4)!=3) {
            for(TempNum2=0;TempNum2<2;TempNum2++){
              Serial.print(Order);
              Serial.print("  ");
              Serial.print(data[8*TempNum2+0],DEC);
              Serial.print("  ");
              Serial.print(data[8*TempNum2+2],DEC);
              Serial.print(":");
              Serial.print(data[8*TempNum2+3],DEC);
              Serial.print(":");
              Serial.print(data[8*TempNum2+4],DEC);
              Serial.print(".");
              Serial.println(data[8*TempNum2+5],DEC);
              Order++;
              if(Block1Data[1]==0x02 && Block1Data[0]==TempNum1) TempNum2 = 2;
            }//分别读出Block里两个数据
        }//if不为Trailer
          ReadFlag = 1;
        }//成功读出block
        else{
          Serial.print("!!!!!unable to read block");
          Serial.println(TempNum1,DEC);
          ReadFlag = 0;   
        }//没成功读出数据
      }//取得了权限
      else {
        Serial.println("Authentication Failed Somewhere");
        ReadFlag = 0;
        break;
      }//没取得进入block权限
    }//end of for
    
    Serial.println("***********finish reading*************");
    if(ReadFlag) {//成功读出数据
      strip.setPixelColor(0,strip.Color(0,0,225));
      strip.show();
      tone(BUZZERPIN,1500);
      delay(200);
      noTone(BUZZERPIN);
      strip.setPixelColor(0,strip.Color(0,0,0));
      strip.show();
    }//成功读出数据
    delay(500);
    ReadFlag = 0;
    
  }//如果读到了target id

}//loop的括号
