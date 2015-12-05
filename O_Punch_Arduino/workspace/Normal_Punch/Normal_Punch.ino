/*******************************************************
普通点签的代码,实现功能:
1.拥有自己的时钟,能被对时器校准时间
2.判断标签激活与否和上一条记录,决定是否写入新一条记录

作者:BH1TAN
时间:20151125
********************************************************/


#include<Wire.h>
#include <Adafruit_NFCShield_I2C.h>

#define IRQ                     (2)
#define RESET                   (3)     // Not connected by default on the NFC Shield
#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector
//
// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector)<NR_SHORTSECTOR)? \
    ((sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1):\
    (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1))

// Determine the sector's first block based on the sector number
#define BLOCK_NUMBER_OF_SECTOR_1ST_BLOCK(sector) (((sector)<NR_SHORTSECTOR)? \
    ((sector)*NR_BLOCK_OF_SHORTSECTOR):\
    (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR))

uint8_t success_f, success_r;                         // Flag to check if there was an error with the PN532
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
bool authenticated = false;               // Flag to indicate if the sector is authenticated
uint8_t blockBuffer[16];                  // Buffer to store block contents
uint8_t blankAccessBits[3] = { 0xff, 0x07, 0x80 };
uint8_t idx = 0;
uint8_t numOfSector = 16;                 // Assume Mifare Classic 1K for now (16 4-block sectors)

uint8_t keya[6]={ 0xff,0xff,0xff,0xff,0xff,0xff };

uint32_t versiondata;

char * data ;
char * url = "microduino.cc";  //The message to write
uint8_t ndefprefix = NDEF_URIPREFIX_HTTP_WWWDOT;

// Create an instance of the NFCShield_I2C class
Adafruit_NFCShield_I2C nfc(IRQ);      //修改了推荐文件,编译通过了 


void writenfc(void){
  
  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success_r = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(!success_r)
    return;
  else
  {
    //对标签写入数据
    Serial.print("  UID Length: "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
  }
  //复制开始
  success_r = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success_r)
    {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
      return;
    }
    success_r = nfc.mifareclassic_FormatNDEF();
    if (!success_r)
    {
      Serial.println("Unable to format the card for NDEF");
      return;
    }

    Serial.println("Card has been formatted for NDEF data using MAD1");

    // Try to authenticate block 4 (first block of sector 1) using our key
    success_r = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 4, 0, keya);

    // Make sure the authentification process didn't fail
    if (!success_r)
    {
      Serial.println("Authentication failed.");
      return;
    }

    // Try to write a URL
    Serial.println("Writing URI to sector 1 as an NDEF Message");

    // Authenticated seems to have worked
    // Try to write an NDEF record to sector 1
    // Use 0x01 for the URI Identifier Code to prepend "http://www."
    // to the url (and save some space).  For information on URI ID Codes
    // see http://www.ladyada.net/wiki/private/articlestaging/nfc/ndef
    if (strlen(url) > 38)
    {
      Serial.println("URI is too long ... must be less than 38 characters long");
      return;
    }

    // URI is within size limits ... write it to the card and report success/failure
    success_r = nfc.mifareclassic_WriteNDEFURI(15, ndefprefix, url);      //sector to write
    if (success_r)
    {
      Serial.println("NDEF URI Record seems to have been written to sector 1");
    }
    else
    {
      Serial.println("NDEF Record creation failed! ");
    }

  //复制结束
 

}
void setup(void) {
  
  
  // put your setup code here, to run once:
  Serial.begin(115200);    //open serial with PC
  Serial1.begin(115200);   //open serial1 with device
  Serial.println("Looking for PN532...\n");
  
  nfc.begin();
  
  versiondata = nfc.getFirmwareVersion();
  /*
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  */
  
  nfc.SAMConfig();    // configure board to read RFID tags
}

void loop(void) {
  // put your main code here, to run repeatedly:
  //Serial.println("WORKING NOW...");
  

  if (Serial.available())
  {
    Serial.print("Chip PN5"); 
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. "); 
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); 
    Serial.println((versiondata >> 8) & 0xFF, DEC);
    Serial.println(versiondata & 0xff , HEX);
  }
  
  writenfc();
  
}
