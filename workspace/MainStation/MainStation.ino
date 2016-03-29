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
unsigned short BlockNum=1;
unsigned short TempNum1,TempNum2;
unsigned short TargetBlockNum,TargetBlockFlag;
unsigned short Order;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type
uint8_t data[16];

/************************************
 * personal function
 */

 
/**********************************
 * end of personal function
 */

void setup() {
  Serial.begin(115200);//与电脑通信，不能太小
  nfc.begin();
  
  /*below gives out board information
  uint32_t versiondata = nfc.getFirmwareVersion();
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
    Order= 1 ;
    //display basic information about the tag
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength,1, 0, keya);
    success = nfc.mifareclassic_ReadDataBlock(1,data);
    TargetBlockNum=data[0];
    TargetBlockFlag=data[1];
    for(TempNum1=0;TempNum1<TargetBlockNum+1;TempNum1++){
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
          for(TempNum2=0;TempNum2<2;TempNum2++){
            Serial.print(Order);
            Serial.print("  ");
            Serial.print(data[0],DEC);
            Serial.print("  ");
            Serial.print(data[2],DEC);
            Serial.print(":");
            Serial.print(data[3],DEC);
            Serial.print(":");
            Serial.print(data[4],DEC);
            Serial.print(".");
            Serial.println(data[5],DEC);
            Order++;
            if(data[2]==0x02&&data[1]==TempNum2) break;
          }//分别读出Block里两个数据
        }//成功读出block
        else{
          Serial.print("!!!!!unable to read block");
          Serial.println(TempNum1,DEC);   
        }//没成功读出数据
      }//取得了权限
      else {
        Serial.println("Authentication Failed Somewhere");
        break;
      }//没取得进入block权限
    }//end of for
    Serial.println("***********finish reading*************");
    delay(1000);
  }//如果读到了target id

}//loop的括号
