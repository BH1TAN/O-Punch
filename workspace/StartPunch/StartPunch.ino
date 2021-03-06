/*
 * 20160325于台湾清华仁斋0316
 * 此为出发器，实现的功能如下：
 * 
 * 清空之前的所有比赛数据
 * 写Block1,指示下一个Block为2，当前序号0
 * 提示写好可以出发
 * （计时出发，每分钟出发一次或两次等可选）
 */
#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Adafruit_NeoPixel.h>

#define IRQ 2
#define LEDPIN 6
#define BUZZERPIN 8

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

Adafruit_NFCShield_I2C nfc(IRQ);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1,LEDPIN, NEO_GRB + NEO_KHZ800);

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
  //后8位可储存出发时间

  strip.begin();
  strip.show();
  pinMode(BUZZERPIN,OUTPUT);
  
}

void loop(){
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
    Serial.println("Access to block1,please wait...");
    success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,1,0,keya);
    
    int nowtime;
    nowtime=millis();
    startdata[10]=nowtime/3600000;
    startdata[11]=(nowtime/60000)%60;
    startdata[12]=(nowtime/1000)%60;
    startdata[13]=(nowtime/10)%100;
    
    success = nfc.mifareclassic_WriteDataBlock(1,startdata);
    if(success){
      Serial.println("####START####");
    strip.setPixelColor(0,strip.Color(255,0,0));//三个位是RGB，只有一个灯时为0号灯
    strip.show();
    tone(BUZZERPIN,1000);//声音提醒
    delay(100);
    strip.setPixelColor(0,strip.Color(0,0,0));
    strip.show();//亮灯提醒
    noTone(BUZZERPIN);
    }//成功写入block1
  }//成功到一张卡
  delay(1000);
}//loop的括号

