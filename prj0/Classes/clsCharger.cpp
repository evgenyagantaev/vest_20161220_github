class clsCharger
{
public:
  clsCharger()
  {
    
    gpioInit();
    setSdLow();
    
    ledInit();
    
    bool st1 = false, st2 = false;
    uint16_t cycleCounter = 0;
    
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))
    {
    rccInit();
    while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))
    {
     //setSdLow();
      st2 = (bool)(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)); //st2
      st1 = (bool)(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)); //st1
      if(st1&&(!st2)) //charging
      {
          for(volatile long k=0;k<100000;k++); 
          
            greenLedOff();
            redLedOff();
            blueLedOn();
          
          for(volatile long k=0;k<100000;k++); 
          
            greenLedOff();
            redLedOff();
            blueLedOff();
          

      }
      else if((!st1)&&st2) //charge done
      {
            greenLedOff();
            redLedOff();
            blueLedOn();
            for(volatile long k=0;k<100000;k++);
            for(volatile long k=0;k<100000;k++);
      }
      else if((!st1)&&(!st2)) //standby
      {
            greenLedOn();
            redLedOn();
            blueLedOff();
            for(volatile long k=0;k<100000;k++);
            for(volatile long k=0;k<100000;k++);
      }
      else //error
      {
            greenLedOn();
            redLedOff();
            blueLedOn();
            for(volatile long k=0;k<100000;k++);
            for(volatile long k=0;k<100000;k++);
      }
      
      
      
      if(cycleCounter>1200)  //reset charger chip
      {
        setSdHigh();
        for(volatile long i=0; i<1000; i++);
        setSdLow();
        //for(volatile long i=0; i<100000; i++);
        cycleCounter=0;
      }
      else
      {
        cycleCounter++;
      }
      
      
    }
  }
  else return;
    
  }

  void ledInit()
  {
    redLedOff();
    greenLedOff();
    blueLedOff();
  }
  
  void greenLedOn(void)
   {
     GPIOC->BSRRH = GPIO_Pin_1;
   }
   void greenLedOff(void)
   {
     GPIOC->BSRRL = GPIO_Pin_1;
   }
   
   void redLedOn(void)
   {
     GPIOC->BSRRH = GPIO_Pin_2;
   }
   void redLedOff(void)
   {
     GPIOC->BSRRL = GPIO_Pin_2;
   }
   void blueLedOn(void)
   {
     GPIOC->BSRRH = GPIO_Pin_0;
   }
   void blueLedOff(void)
   {
     GPIOC->BSRRL = GPIO_Pin_0;
   }
  
  void gpioInit()
  {
    
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE); 
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, DISABLE);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
    
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS_ULPI, DISABLE);
    
    // Enable the GPIOA clock
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, DISABLE);
      
      
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, DISABLE);
      
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, DISABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, DISABLE); 
      
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
      
      GPIO_InitTypeDef GPIO_InitStructure;


      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;  //PC.0-2 pin for rgb led           
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //no pull resistor
      GPIO_Init(GPIOC, &GPIO_InitStructure);
    
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       //usb in pin      
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOA, &GPIO_InitStructure);
      
      //ST1 and ST2 pin PB.0-1
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      
      //SD reset pin PB.2
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
  }
  
   void setSdHigh() //sd charge pin
   {
     GPIOB->BSRRL=GPIO_Pin_2;
   }
   
   void setSdLow()
   {
     GPIOB->BSRRH=GPIO_Pin_2;
   }
  
  void rccInit()
  {
    // PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N 
    #define cPLL_M 16 // For HSE value equal to 25 MHz 
    #define cPLL_N 240
    // SYSCLK = PLL_VCO / PLL_P 
    #define cPLL_P 8
    // USB OTG FS, SDIO and RNG Clock = PLL_VCO / PLLQ
    #define cPLL_Q 15


    
    RCC_DeInit();
    
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

    FLASH_SetLatency(FLASH_Latency_0);
    FLASH_PrefetchBufferCmd(DISABLE);
    FLASH_InstructionCacheCmd(DISABLE);
    FLASH_DataCacheCmd(DISABLE);

    RCC_HCLKConfig(RCC_SYSCLK_Div16);

    RCC_PCLK2Config(RCC_HCLK_Div16);

    RCC_PCLK1Config(RCC_HCLK_Div16);

    RCC_PLLConfig(RCC_PLLSource_HSI, cPLL_M, cPLL_N, cPLL_P, cPLL_Q);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

    while (RCC_GetSYSCLKSource() != 0x00) {}
}

  
};
