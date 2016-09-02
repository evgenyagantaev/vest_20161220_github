
#define BATTERY_I2C_TIMEOUT 0x0
#define BATTERY_I2C_OK 0x1
#define BATTERY_I2C_TIMEOUT_DELAY 120000 //default 120000
#define BATTERY_I2C_HARD_TIMEOUT 0x0
#define BATTERY_I2C_SOFT_TIMEOUT 0x1
#define BATTERY_AVERAGE_RANGE 50
#define BATTERY_AVERAGE_FILTER 8
#define BATTERY_AVERAGE_FILTER_COUNT 3

class clsBattery
{
public:

  static const uint8_t SAddr = 0x6C;   //max17047 slave address
  
  uint8_t averageBtArray[BATTERY_AVERAGE_RANGE];
  uint8_t averageBtValue;
  uint8_t averageBtCount;
  uint8_t averageBtIndex;
  uint8_t avlast;
  bool notfirst;
  uint8_t filtercount;
  
  clsBattery()
  {
    //init av
    for(uint8_t i=0; i<BATTERY_AVERAGE_RANGE; i++)
    {
      averageBtArray[i] = 0;
    }
    filtercount = 0;
    notfirst = false;  
    avlast = 0;
    averageBtIndex = 0;
    averageBtCount = 0;
    averageBtValue = 0;
    
    init();
    
  
    
    uint16_t misc = readRegister(0x2B, BATTERY_I2C_HARD_TIMEOUT);
    misc &= 0xF7FF;
    writeRegister(0x2B,misc, BATTERY_I2C_HARD_TIMEOUT);  //disable battery insertion detect
    
    writeRegister(0x0,0x0, BATTERY_I2C_HARD_TIMEOUT);    //reset por reg 0x0
    
    writeRegister(0x1D,0x2120, BATTERY_I2C_HARD_TIMEOUT); //0x2120
    
    writeRegister(0x10,0x640, BATTERY_I2C_HARD_TIMEOUT);  //0x640
    writeRegister(0x18,0x640, BATTERY_I2C_HARD_TIMEOUT);  //0x640
    writeRegister(0x1E,0x80, BATTERY_I2C_HARD_TIMEOUT);   //0x80
    

  }
  
  void init()
  {
    //watchdog.start();
    
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* Enable I2C2 and I2C2_PORT & Alternate Function clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //GPIOB->BSRRL=GPIO_Pin_12; //high
    //GPIOB->BSRRH=GPIO_Pin_12; //low
    
    
    /* Alternate function remapping */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);

    /* I2C2 SCL and SDA pins configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Reset I2C2*/
    I2C_DeInit(I2C2);

     /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 400000;

    /*!< I2C Initialize */
    I2C_Init(I2C2, &I2C_InitStructure);

    /* I2C ENABLE */
    I2C_Cmd(I2C2, ENABLE); 
  }
  
  void addBtArrayValue(uint8_t val)
  {
    if(averageBtCount < BATTERY_AVERAGE_RANGE) averageBtCount++;
    if(averageBtIndex >= BATTERY_AVERAGE_RANGE) averageBtIndex = 0;
    
    averageBtIndex++;
    
    averageBtArray[averageBtIndex] = val;
  }
  
  uint8_t getAverageValue()
  {
    
    uint16_t sum=0;
    
    for(uint8_t i=0; i<=averageBtCount; i++)
    {  
      sum+=averageBtArray[i];
    }
    
    uint8_t av = (uint8_t)(sum/averageBtCount);
    
    //printf("c %u i %u s %u av %u\r\n",averageBtCount,averageBtIndex,sum,av);
    
    return av; 
  }
  
  
  uint8_t getBatteryCharge()
  {
     

    //uint16_t misc = readRegister(0x9,BATTERY_I2C_SOFT_TIMEOUT);
    //printf("vx: %x v: %f\r\n",misc,(float)(misc>>3)*0.625);
   
   // int16_t curr = (int16_t)readRegister(0xA);
   // printf("curr: %f\r\n",(float)(curr*1.5625/10.0)); 
    
    //uint16_t reg = readRegister(0x9);
    //printf("reg: %x\r\n",reg); 
  
    
    uint16_t batc = readRegister(0x6,BATTERY_I2C_SOFT_TIMEOUT);
    
    //printf("c: %u\r\n",batc);
    //float c = (float)(batc>>8)+(float)(batc&0xFF)*0.0039;
    
    //printf("c: %f\r\n",c);
    uint8_t tmp = (uint8_t)(batc>>8);
    
    if(tmp>100) tmp=100;
    
    //printf("read %u ",tmp);
    
    //filter
    int delta = ((int)avlast) - ((int)tmp);
    if( ((delta>BATTERY_AVERAGE_FILTER)||(delta<(-BATTERY_AVERAGE_FILTER)))&&notfirst)
    {
      if(filtercount<BATTERY_AVERAGE_FILTER_COUNT)
      {
        filtercount++;
        tmp=avlast;
      }
    }
    else
    {
      filtercount = 0;
    }
    
    addBtArrayValue(tmp);
    
    uint8_t tmpav = getAverageValue();
    
    avlast = tmpav;
    
    notfirst = true;
    
    //printf("average %u fc %u\r\n",tmpav,filtercount);
    
    return tmpav;
    //return 99;
  }

  uint16_t readRegister(uint8_t MAddr, uint8_t to_type)
  { 
    /*
    
    I2C_AcknowledgeConfig(I2C2, ENABLE);
  //----------------------------- Transmission Phase -------------------------

    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));  

    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
  
    I2C_SendData(I2C2, MAddr);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

    I2C_GenerateSTOP(I2C2, ENABLE);
    
  
  //------------------------------- Reception Phase --------------------------
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));  

    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));  
    
    uint8_t  Data1 = 0, Data2 = 0; //received data bytes
    
    
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));  
    Data1 = I2C_ReceiveData(I2C2);
    
    I2C_AcknowledgeConfig(I2C2, DISABLE);
    I2C_NACKPositionConfig(I2C2,I2C_NACKPosition_Next); 
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));   
    Data2 = I2C_ReceiveData(I2C2);
    
    I2C_GenerateSTOP(I2C2, ENABLE);
    while (I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE) == RESET);
    
    
    uint16_t tmp = ((uint16_t)Data2<<8)+Data1;
    
    return tmp;
    */
    
    
    uint8_t stat = BATTERY_I2C_OK;
    
    I2C_AcknowledgeConfig(I2C2, ENABLE);
  //----------------------------- Transmission Phase -------------------------

    I2C_GenerateSTART(I2C2, ENABLE);
    stat = waitForEvent(I2C_EVENT_MASTER_MODE_SELECT,to_type);  

    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Transmitter);
    stat = waitForEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,to_type); 
  
    I2C_SendData(I2C2, MAddr);
    stat = waitForEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED,to_type); 

    I2C_GenerateSTOP(I2C2, ENABLE);
    
  
  //------------------------------- Reception Phase --------------------------
    I2C_GenerateSTART(I2C2, ENABLE);
    stat = waitForEvent(I2C_EVENT_MASTER_MODE_SELECT,to_type);  

    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Receiver);
    stat = waitForEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED,to_type);  
    
    uint8_t  Data1 = 0, Data2 = 0; //received data bytes
    
    
    stat = waitForEvent(I2C_EVENT_MASTER_BYTE_RECEIVED,to_type);  
    Data1 = I2C_ReceiveData(I2C2);
    
    I2C_AcknowledgeConfig(I2C2, DISABLE);
    I2C_NACKPositionConfig(I2C2,I2C_NACKPosition_Next); 
    stat = waitForEvent(I2C_EVENT_MASTER_BYTE_RECEIVED,to_type);   
    Data2 = I2C_ReceiveData(I2C2);
    
    stat = waitForFlag(I2C_FLAG_RXNE,RESET,to_type);
    I2C_GenerateSTOP(I2C2, ENABLE);
    //stat = waitForFlag(I2C_FLAG_RXNE,RESET,to_type);
    
    /*
     if(to_type==BATTERY_I2C_SOFT_TIMEOUT)
     {
        vTaskDelay(100);
     }
    */
    
    if(stat!=BATTERY_I2C_OK)
    {
      //printf("i2c read error\r\n");
      init();
      return 0;
    }
    
    uint16_t tmp = ((uint16_t)Data2<<8)+Data1;
    
    return tmp;
  }
  
  void writeRegister(uint8_t MAddr, uint16_t val, uint8_t to_type)
  {
    /*
   
    I2C_AcknowledgeConfig(I2C2, ENABLE);
  //----------------------------- Transmission Phase -------------------------

    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));  

    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
  
    I2C_SendData(I2C2, MAddr);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 
    
    
    uint8_t  Data1 = (val>>8), Data2 = (uint8_t)(val&0xFF); //received data bytes
    
    //printf("d1 %x d2 %x\r\n",Data1,Data2);
    
    I2C_SendData(I2C2, Data2);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 
    
    //I2C_NACKPositionConfig(I2C2,I2C_NACKPosition_Next);
    
    I2C_SendData(I2C2, Data1);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 
    
    I2C_GenerateSTOP(I2C2, ENABLE);
    while (I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) == RESET);   
  */
  
    uint8_t stat = BATTERY_I2C_OK;
    
    I2C_AcknowledgeConfig(I2C2, ENABLE);
  //----------------------------- Transmission Phase -------------------------

    I2C_GenerateSTART(I2C2, ENABLE);
    
    stat = waitForEvent(I2C_EVENT_MASTER_MODE_SELECT,to_type);
  
    I2C_Send7bitAddress(I2C2, SAddr, I2C_Direction_Transmitter);
    stat = waitForEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,to_type); 
  
    I2C_SendData(I2C2, MAddr);
    stat = waitForEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED,to_type); 
    
    
    uint8_t  Data1 = (val>>8), Data2 = (uint8_t)(val&0xFF); //received data bytes
    
    I2C_SendData(I2C2, Data2);
    stat = waitForEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED,to_type); 
       
    I2C_SendData(I2C2, Data1);
    stat = waitForEvent( I2C_EVENT_MASTER_BYTE_TRANSMITTED,to_type); 
    
    I2C_GenerateSTOP(I2C2, ENABLE);
    stat = waitForFlag(I2C_FLAG_TXE,RESET,to_type); 
    
    if(stat!=BATTERY_I2C_OK)
    {
      //printf("i2c write error\r\n");
      init();
      return;
    }
    
    
  }
  
  float getChargeCurrent()
  {
    int16_t curr = (int16_t)readRegister(0xA,BATTERY_I2C_SOFT_TIMEOUT);
    //printf("curr: %f\r\n",(float)(curr*1.5625/10.0)); 
    
    return ((float)(curr*1.5625/10.0));
  }

//------------------------------------------------------------------------------  
  uint8_t waitForFlag(uint32_t flag, FlagStatus fs, uint8_t to_type)
  {
    uint32_t to = 0; 
    

    while(I2C_GetFlagStatus(I2C2, flag) == fs)
    {
      if(to>=BATTERY_I2C_TIMEOUT_DELAY)
      {
        return BATTERY_I2C_TIMEOUT;
      }
      else
      {
         if(to_type==BATTERY_I2C_SOFT_TIMEOUT)
         {
            vTaskDelay((portTickType)(1*(configTICK_RATE_HZ/1000000)));
            to++;
         }
        else
        {
            to++;
        }         
      }
      watchdog.reload();
    }
    //printf("f ok\r\n");
    return BATTERY_I2C_OK;  
  }
  
  uint8_t waitForEvent(uint32_t event, uint8_t to_type)
  {
    uint32_t to = 0;
      
    while(!I2C_CheckEvent(I2C2, event))
    {
      if(to>=BATTERY_I2C_TIMEOUT_DELAY)
      {
        return BATTERY_I2C_TIMEOUT;
      }
      else
      {
        if(to_type==BATTERY_I2C_SOFT_TIMEOUT)
         {
            vTaskDelay((portTickType)(1*(configTICK_RATE_HZ/1000000)));
            to++;
         }
        else
        {
            to++;
        }
      }
      watchdog.reload();
    }
    return BATTERY_I2C_OK;
  }
 
};
