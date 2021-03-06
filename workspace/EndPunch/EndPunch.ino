/*
 * 
 * （记录时间）
 * 
 * 目前：copy了普通点签的代码
 */

#include<Wire.h>
#include<Adafruit_NFCShield_I2C.h>
#include <Adafruit_NeoPixel.h>

#define IRQ 2
#define LEDPIN 6
#define BUZZERPIN 8

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1,LEDPIN, NEO_GRB +NEO_KHZ800);
Adafruit_NFCShield_I2C nfc(IRQ);
unsigned short TempNum1;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t Block1data[16];
unsigned short PunchNum=255;

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
  strip.begin();
  strip.show();// Initialize all pixels to 'off'
  pinMode(BUZZERPIN,OUTPUT);
}

void loop() {
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
      //AuthenticateBlock
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength,1, 0, keya);
      if(success){
        //读取Block1
        success = nfc.mifareclassic_ReadDataBlock(1, Block1data);
        if (success){
          if((Block1data[2]!=PunchNum)&&(Block1data[2]!=255)){//上一个记录不是这个点，或标签未激活
            success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,Block1data[0],0,keya);
            if(success){//成功进入可能空白的Block

              uint8_t data[16]={0};
              unsigned short NowNum=Block1data[0];//本次写的Block
              success = nfc.mifareclassic_ReadDataBlock(Block1data[0],data);
              if(Block1data[1]==0x01){//本Block全空
                data[0]=PunchNum;
                
                int nowtime;
                nowtime=millis();
                data[2]=nowtime/3600000;
                data[3]=(nowtime/60000)%60;
                data[4]=(nowtime/1000)%60;
                data[5]=(nowtime/10)%100;//时间信息
                
                Block1data[1]=0x02;//指示只有低位空
              }//Block全空
              else {
                data[8]=PunchNum;
                
                int nowtime;
                nowtime=millis();
                data[10]=nowtime/3600000;
                data[11]=(nowtime/60000)%60;
                data[12]=(nowtime/1000)%60;
                data[13]=(nowtime/10)%100;//时间信息
                
                Block1data[1]=0x01;//指示全位空
                if((Block1data[0]%4)==2)
                  Block1data[0]++;   //下一个是trailerblock，跳过之
                Block1data[0]++;//指向下一位
                
                
              }//Block前面被占
              success = nfc.mifareclassic_WriteDataBlock(NowNum,data);//写入记录
              Block1data[2]=PunchNum;
              
            }//成功进入空白Block
            success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,1,0,keya);//更新Block1
            success = nfc.mifareclassic_WriteDataBlock(1,Block1data);//更新Block1
            Serial.println("$$$$END$$$$");
            strip.setPixelColor(0,strip.Color(0,0,255));//三个位是RGB，只有一个灯时为0号灯
            strip.show();
            tone(BUZZERPIN,1000);
            delay(100);
            strip.setPixelColor(0,strip.Color(0,0,0));
            strip.show();//亮灯提醒
            noTone(BUZZERPIN);
            //至此，新增一个记录点的步骤完成
             
          }//上一个记录不是这个点
          else Serial.println("You have punched or finished");
        }//成功读出block的数据
      }//取得了权限
      else
        Serial.println("Authentication Failed");
    delay(1000);
  }//如果读到了target id

}//loop的括号
