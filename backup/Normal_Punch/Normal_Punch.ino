/*******************************************************
普通点签的代码,实现功能:
1.拥有自己的时钟,能被对时器校准时间
2.判断标签激活与否和上一条记录,决定是否写入新一条记录

作者:BH1TAN
时间:20151125
********************************************************/


#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>

#define IRQ                     (2)
#define RESET                   (3)     // Not connected by default on the NFC Shield
#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector

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
unsigned short TempNum1,TempNum2;



/*************标签信息**********************/
uint8_t data1[4];    //写入的数据,一个block有4个最大255的uint8_t数据
uint8_t data2[4];    //读出的数据
uint8_t PreCursor;   //上一次写入的block号
uint8_t PreOrder;    //上一个点签号
uint8_t OrderNum;    //0为起点,1为终点
/*************标签信息**********************/

/*************点签信息**********************/
unsigned long RaceTime;
unsigned long TimeBios = 0;
uint8_t time[4];
unsigned short StatesNum;   //点签工作模式,0为收到未激活标签等,1为普通写入
/*************点签信息**********************/


// Create an instance of the NFCShield_I2C class
Adafruit_NFCShield_I2C nfc(IRQ);

/*********************************函数起始************************************/

void GetTime(void)
{
  RaceTime=millis();
  time[1]=RaceTime/3600000;      //时
  time[2]=(RaceTime/60000)%60;   //分
  time[3]=(RaceTime/1000)%60;    //秒
  //time[0]=(RaceTime%1000)/100;    //秒的小数
}

void SerialPrintTime(void)
{
  GetTime();
  Serial.print(RaceTime);
  Serial.print("*");
  Serial.print(time[0]);
  Serial.print(":");
  Serial.print(time[1]);
  Serial.print(":");
  Serial.print(time[2]);
  Serial.print(".");
  Serial.print(time[3]);
  Serial.println("");
}

void readnfc(void){
  success_r=nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(!success_r)
  {
    Serial.println("Fail to read");
    return;
  }
  else
  {
    //提示标签类型
    Serial.print("UID Length(READING): "); 
    Serial.print(uidLength, DEC); 
    Serial.println(" bytes");
    Serial.print("UID Value(READING): ");
    nfc.PrintHex(uid, uidLength);
  }
  
  success_r = nfc.mifareclassic_ReadDataBlock (4,data2);
  if(!success_r)
  {
    Serial.println('Cant read data block4');
  }
  else
  {
    if(data2[0]==3)        //标签未激活
    {
      Serial.println('Tag NOT in use');
      StatesNum=0;
      return;
    }
    else if (data2[0]>3)
    {
      PreOrder=data2[1];         //收集上一个点签号
      Serial.println('Tag in use');
      StatesNum=1;
    }
    else if(data2[0]<3)
    {
      Serial.println('Tag is strange');
    }
    else
      return;
  }
}

void writenfc(void){
  success_r = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if(!success_r)
    return;

    if(StatesNum)
    {
      //准备即将写入的信息
      GetTime();
      for(TempNum1=0;TempNum1<4;TempNum1++)
        data1[TempNum1]=time[TempNum1];
      data1[0]=OrderNum;
      //准备即将写入的信息
      if(PreOrder==OrderNum)    //上一条记录由本点签写入
      {
        success_r = nfc.mifareclassic_WriteDataBlock(PreCursor,data1);      //block to write
        if (success_r)
        Serial.println("Write to OLD block\n");
      else
        Serial.println("error:Fail to write to the old block\n");
      }
      else
      {
        PreCursor++;
        success_r = nfc.mifareclassic_WriteDataBlock(PreCursor,data1);      //block to write
        if (success_r)
          Serial.println("Write to NEW block\n");
        else
          Serial.println("error:Fail to write to a new block\n");        
      }
      data1[0]=PreCursor;
      data1[1]=OrderNum;
      data1[2]=0;
      data1[3]=0;
      success_r = nfc.mifareclassic_WriteDataBlock(4,data1);      //block4 to write
      if (success_r)
        Serial.println("Block4 updated \n");
      else
        Serial.println("error:Fail to update Block4 \n");       
    }
    StatesNum=0;

}
void setup(void) {
  
  
  // put your setup code here, to run once:
  Serial.begin(115200);    //open serial with PC
  Serial1.begin(115200);   //open serial1 with device
  Serial.println("Looking for PN532...\n");
  
  nfc.begin();
  
  /*
  versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  */
  
  nfc.SAMConfig();    // configure board to read RFID tags
  OrderNum=31;
  StatesNum=0;
}

void loop(void) {

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
  readnfc();
  writenfc();
  delay(1000);
  SerialPrintTime();
  
}
