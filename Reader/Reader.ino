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
unsigned short TempNum1;
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
    
    //display basic information about the tag
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    for(TempNum1=0;TempNum1<51;TempNum1++){
      //AuthenticateBlock
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, TempNum1, 0, keya);
      if(success){
        //读取测试
        success = nfc.mifareclassic_ReadDataBlock(TempNum1, data);
        if (success){
          // Data seems to have been read ... spit it out
          Serial.print("Block");
          Serial.print(TempNum1,DEC);
          Serial.print(":");
          nfc.PrintHexChar(data, 16);
          
        }//成功读出block4的数据
        else{
          Serial.print("!!!!!unable to read block");
          Serial.println(TempNum1,DEC);   
        }//没成功读出数据
      }//取得了权限
      else
        Serial.println("Authentication Failed");
    }//end of for
    Serial.println("***********finish reading*************");
    delay(1000);
  }//如果读到了target id

}//loop的括号
