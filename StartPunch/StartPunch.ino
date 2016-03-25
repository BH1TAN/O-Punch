/*
 * 20160325于台湾清华仁斋0316
 * 此为出发器，实现的功能如下：
 * 
 * 清空之前的所有比赛数据
 * 写Block1
 * （提示写好可以出发）
 * （计时出发，每分钟出发一次或两次等可选）
 */
#include<Wire.h>
#include<Adafruit_NFCShield_I2C.h>

#define IRQ 2

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

Adafruit_NFCShield_I2C nfc(IRQ);
unsigned short TempNum1;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t voiddata[16]={0},startdata[16]={0},data[16]={0};

void setup(){
  Serial.begin(115200);
  nfc.begin();
  nfc.SAMConfig();
  startdata[0]=2;//下一个是block2
  startdata[1]=1;//前八位
  startdata[2]=0;//当前点签号为0，表示刚出发
  
}

void loop(){
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
    Serial.println("Access to block1,please wait...");
    success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,1,0,keya);
    success = nfc.mifareclassic_WriteDataBlock(1,startdata);
    if(success){
      Serial.println("####START####");
    }//成功写入block1
  }//成功到一张卡
  delay(1000);
}//loop的括号

