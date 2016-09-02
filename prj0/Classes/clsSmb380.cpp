// This class provides functionality of smb380 accelerometer

class clsSmb380
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //static const uint16_t 
   //*********************** end constants *************************************   


   //************************* variables ***************************************
bool readInProcess;
   //*********************** end variables ************************************


   //********************* private functions **********************************
void setCsToLow()
  {
    GPIOA->BSRRH = GPIO_Pin_4;
    //printf("cs low\r\n");
  }
  void setCsToHigh()
  {
    GPIOA->BSRRL = GPIO_Pin_4;
    //printf("cs high\r\n");
  }
  
  void waitForFlag(uint16_t flag) //wait for flag method
  {

    //printf("wf %u\r\n",flag);
#ifdef NOSPI1TIMEOUT
    //no timeout method-------------------------------------------------------
    while (SPI_I2S_GetFlagStatus(SPI1,flag) == RESET)
    {
      //printf("sr %u\r\n",SPI1->SR);
    };
    //------------------------------------------------------------------------
#else    
    //timeout method----------------------------------------------------------
    
    bool timeoutFlag = false; //timeout flag, true if timeout interval > period
   
    portTickType toIn = 0;
    portTickType toOut = 0;
    portTickType toInterval = 0;
    
    toIn = xTaskGetTickCount();
    while ((SPI_I2S_GetFlagStatus(SPI1,flag) == RESET) && (!timeoutFlag) )
    {
      toOut = xTaskGetTickCount();
      toInterval = (toOut - toIn)*tickLength;
      if(toInterval >= timeoutPeriod) timeoutFlag = true;
    }  
    if(timeoutFlag) //timeout event
    {   
      //do something
#ifdef DEBUG      
      printf("SPI1 Acc Timeout!\r\n");
#endif
      //debug stop
      while(1);
    }
    
     //-------------------------------------------------------------------------
#endif    
    
  }

   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsSmb380()
   {
      spi1.configSpi1();
      setCsToHigh(); //Set CS pin to high
      accWriteSettings(); //set accelerometer settings at startup
   }//end clsSmb380
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   bool getReadInProcess(){return readInProcess;}
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   void accReadData() //read acceleration and temperature
  {
    readInProcess = true;
    
    int16_t rawX=0, rawY=0, rawZ=0;
    //int16_t rawT=0;
    //float temperature=0.0;
    uint8_t  buf[6];
    
    char lsb;
      
    for(int i=0; i<6; i++) //clear buffer
        buf[i] = 0;
   
    accReadXyzRawData(buf);
                   
    rawX = buf[1]; rawX <<= 8; lsb = buf[0] & 0xC0;
    rawX += lsb;
    rawX /= 64;     // >> 6

    rawY = buf[3]; rawY <<= 8; lsb = buf[2] & 0xC0;
    rawY += lsb;
    rawY /= 64;     // >> 6

    rawZ = buf[5]; rawZ <<= 8; lsb = buf[4] & 0xC0;
    rawZ += lsb;
    rawZ /= 64;     // >> 6

    /*     
    rawT=accReadByte(0x08);
    rawT=(rawT>>1) - 30;
    if(rawT&0x01) temperature = rawT + 0.5;
    */               
/*
#ifdef DEBUG
    printf("%d  %d  %d\r\n", rawX, rawY, rawZ);
#endif    
*/     
    // save accelerometer data in cyclic buffer
    common.accBufferHead++;
    if(common.accBufferHead >= ACCBUFFERLEN) 
        common.accBufferHead=0;
    /*
    (common.accBuffer + common.accBufferHead)->x = rawX;
    (common.accBuffer + common.accBufferHead)->y = rawY;
    (common.accBuffer + common.accBufferHead)->z = rawZ;
    */
    
    
    common.accBuffer[common.accBufferHead].x = rawX;
    common.accBuffer[common.accBufferHead].y = rawY;
    common.accBuffer[common.accBufferHead].z = rawZ;
    
    
    
   /*
#ifdef DEBUG
    int xx = common.accBuffer[common.accBufferHead].x;
    int yy = common.accBuffer[common.accBufferHead].y;
    int zz = common.accBuffer[common.accBufferHead].z;
    printf("%d  %d  %d at %d\r\n", xx, yy, zz,common.accBufferHead);
#endif    
   */
    
    readInProcess = false;
  }
   
   void accWriteSettings() //write smb380 accelerometer settings method
  {
    //printf("wr sets\r\n");
    uint8_t tmp;
    
    
    accWriteByte(0x80,0x15);      //write settings at 0x15 register, bit 7 for SPI4
                                  //bit 3 for not blocking MSB, bit 4 for int latch
    
    tmp = accReadByte(0x14);      //read calibration data from 0x14 register
    tmp=(tmp&0xe0)+0x0e;          //save calibration data and write range (4g) and bandwidth(1500Hz)
    accWriteByte(tmp,0x14);
    
    accWriteByte(0x00,0x0b);      //write int settings at 0x0b register
  }
  
  uint8_t accReadByte(uint8_t addr) //read one byte from smb380 at addr register
  {
    //printf("rb\r\n");
    setCsToLow();
      
      uint8_t tmp = 0;

      waitForFlag(SPI_I2S_FLAG_TXE);  
      SPI1->DR = 0x80|addr;  //address with first RW bit for read mode
      waitForFlag(SPI_I2S_FLAG_RXNE);
      tmp = SPI1->DR;
   
      waitForFlag(SPI_I2S_FLAG_TXE);
      SPI1->DR = 0xFF;       //send dummy byte
      waitForFlag(SPI_I2S_FLAG_RXNE);
      tmp = SPI1->DR;
    
    
    setCsToHigh();
    
    return tmp;
  }
  
  void accWriteByte(uint8_t data, uint8_t addr) //write smb380 byte to addr register
  {
    //printf("wb\r\n");
    setCsToLow();
    
      uint8_t tmp=0;
      
      waitForFlag(SPI_I2S_FLAG_TXE);
      SPI1->DR = addr; //address without first RW bit for write mode
      waitForFlag(SPI_I2S_FLAG_RXNE);
      tmp = SPI1->DR;
      
      waitForFlag(SPI_I2S_FLAG_TXE);
      SPI1->DR = data; 
      waitForFlag(SPI_I2S_FLAG_RXNE);
      tmp = SPI1->DR;
    
    setCsToHigh();
  }
  
  void accReadXyzRawData(uint8_t *data) //read smb380 xyz h and l registers to buffer
  {
    //printf("raw\r\n");
    setCsToLow();
      
      uint8_t tmp=0;
      
      waitForFlag(SPI_I2S_FLAG_TXE);
      SPI1->DR = 0x82; //0x02 register with RW bit
      waitForFlag(SPI_I2S_FLAG_RXNE);
      tmp = SPI1->DR;
            
      for(int i=0;i<6;i++) //XYZ h and l registers, total 6
      {
        waitForFlag(SPI_I2S_FLAG_TXE);
        SPI1->DR = 0xFF; //send dummy byte
        waitForFlag(SPI_I2S_FLAG_RXNE);
        data[i]=(uint8_t)SPI1->DR;
      }
      
    setCsToHigh();
  }
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   //--------------------------end----------------------------------------------
   
      
};