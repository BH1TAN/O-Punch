/***********************************************/
/*
 * 20160324于台湾清华仁斋0316
 * microduino板子作为点签器使用
 * 4位UID的mifare classic标签做为指卡使用
 * 
 * LED部分参考strandtest
 * 
 * */
 
#include<Wire.h>
#include<Adafruit_NFCShield_I2C.h>
#include<Adafruit_NeoPixel.h>

#define IRQ 2
#define LEDPIN 6

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NFCShield_I2C nfc(IRQ);

unsigned short TempNum1;
uint8_t success;
uint8_t uid[] = {0,0,0,0,0,0,0};
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t Block1data[16];
unsigned short PunchNum=51;

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
  strip.show();//Initialize all pixels to 'off'

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
          if(Block1data[2]!=PunchNum&&Block1data[2]!=222){//上一个记录不是这个点，或标签未激活
            success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,Block1data[0],0,keya);
            if(success){//成功进入可能空白的Block

              uint8_t data[16]={0};
              unsigned short NowNum=Block1data[0];//本次写的Block
              success = nfc.mifareclassic_ReadDataBlock(Block1data[0],data);
              if(Block1data[1]==0x01){//本Block全空
                data[0]=PunchNum;
                Block1data[1]=0x02;//指示只有低位空
                /*今后加时间相关，调整data的函数*/
                
              }//Block全空
              else {
                data[8]=PunchNum;
                Block1data[1]=0x01;//指示全位空
                if((Block1data[0]%4)==2)
                  Block1data[0]++;   //下一个是trailerblock，跳过之
                Block1data[0]++;//指向下一位
                /*今后加时间相关，调整data的函数*/
                
              }//Block前面被占
              success = nfc.mifareclassic_WriteDataBlock(NowNum,data);//写入记录
              Block1data[2]=PunchNum;
              
            }//成功进入空白Block
            success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,1,0,keya);//更新Block1
            success = nfc.mifareclassic_WriteDataBlock(1,Block1data);//更新Block1
            Serial.println("$$$$New$$$$");
            strip.setPixelColor(0,strip.Color(255,0,0));//Red,第一个灯为0号
            strip.show();
            delay(200);
            strip.setPixelColor(0,strip.Color(0,0,0));//关灯
            strip.show();
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
