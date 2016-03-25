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
unsigned short BlockNum=1;//要写哪个Block？不能是4n-1
unsigned short DigitNum=1;//要写在哪一位
uint8_t TargetNum=0x01;//要写的数据
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;//4 or 7 bytes depending on ISO14443A card type


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
  if((BlockNum%4)==3){//不要碰trailer
    Serial.println("******Don't touch this block******");
  }
  else {
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(success){
    success = nfc.mifareclassic_AuthenticateBlock(uid,uidLength,BlockNum,0,keya);
    uint8_t data[16]={0};
    success = nfc.mifareclassic_ReadDataBlock(BlockNum,data);
    data[DigitNum]=TargetNum;
    success = nfc.mifareclassic_WriteDataBlock(BlockNum,data);
    if(success){
      Serial.println("Maybe Done!");
    }
    else Serial.println("#######FAILED#########");
    }
    
    
    
  }//安全的block
  delay(500);
}//loop的括号
