/***********************************************/
/*
 * 20160324于台湾清华仁斋0316
 * microduino板子作为点签器使用
 * 4位UID的mifare classic标签做为指卡使用
 * 
 * */
#include<Wire.h>
#include<Adafruit_NFCShield_I2C.h>

#define IRQ 2

uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥

Adafruit_NFCShield_I2C nfc(IRQ);
unsigned short TempNum1;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t Block1data[16];
unsigned short PunchNum=47;

/************************************
 * personal function
 */

 
/**********************************
 * end of personal function
 */

void setup() {
  Serial.begin(115200);//与电脑通信，不能太小
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  
  /*below gives out board information
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  // configure board to read RFID tags
  */
  
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");

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
