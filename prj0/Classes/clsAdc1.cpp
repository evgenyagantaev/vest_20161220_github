class clsAdc1 //ADC1 class @ PA.1 pin /w Software conversion for battery processing
{
private:
public:
   uint16_t adc1ConvValue;   
   clsAdc1() //Construct
   {
    adc1ConvValue=0;
     
    initAdc1();
    

   }
   
   void initAdc1()
   {
      ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;
    
  /* Enable ADC1 and GPIOA clocks *********************************************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
       
  /* Configure ADC1 Channel1 pin as analog input *****************************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; //analog mode
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
    //ADC_DeInit();
    
  /* ADC Common Init **********************************************************/
    
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    
    
    
  /* ADC1 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
  /* ADC1 regular channel1 configuration *************************************/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);
   
    ADC_Cmd(ADC1, ENABLE);
   }
   
   uint16_t getAdc1Value(void)
   {
     
      bool timeoutFlag = false; //timeout flag, true if timeout interval > period
   
      portTickType toIn = 0;
      portTickType toOut = 0;
      portTickType toInterval = 0;
    
      //timing control <<<<<<<<<<<<<<<<<<<<<
      //GPIOD->BSRRL = GPIO_Pin_5;   //pd5 high
      
      /*
      // Start the conversion
      ADC_SoftwareStartConv(ADC1);
      // Wait until conversion completion
      while((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) );
      // Get the conversion value
      adc1ConvValue = ADC_GetConversionValue(ADC1);
      //*/
      
      //adc1 force setup
      
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //adc1 clock enable
      ADC->CCR=0x00;  //common control
      ADC1->SMPR1=0; //sample time
      ADC1->SMPR2=0;
      ADC1->SQR1=0;  //sequence
      ADC1->SQR2=0;
      ADC1->SQR3=1;
      ADC1->CR1=0x00; //control 1
      ADC1->CR2=0x01; //control 2
      //*
      // Start the conversion
      // Enable the selected ADC conversion for regular group 
      ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;
      // Wait until conversion completion
      
      uint16_t timeout=0xFF; //number of tryies
      
      // Check the status of the specified ADC flag 
      while (((ADC1->SR & ADC_FLAG_EOC) == (uint8_t)RESET)&&(timeout>0))
      {
        timeout--;
        taskYIELD();
      }
      //printf("to %u\r\n",timeout);
      // Get the conversion value  
      adc1ConvValue = (uint16_t) ADC1->DR;
      //*/

      RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE); //adc1 clock disable
      
    //GPIOD->BSRRH = GPIO_Pin_5;   //pd5 low
    //timing control >>>>>>>>>>>>>>>>>>>>>
    
    //printf("%d\r\n", adc1ConvValue);
    return adc1ConvValue;
   }
   
   uint16_t getBatteryVoltagePercent(void)
   {
     const float Vup = 4.0,     //100%
                 Vdown = 3.5,   //0%
                 Vref = 3.3,    //reference
                 Vpoff = 3.2;   //power-off voltage
     

     double Vcurr;
     Vcurr = (double)getAdc1Value();
     Vcurr = Vcurr * 2 * (Vref/4096.0); //current battery voltage
     //debug
     //printf("%f\r\n", Vcurr);
     
    
     if(Vcurr<Vpoff) //power-off
     {
        sdio.closeFile();
        //uint32_t rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1);
        //if(rd!=2) flash.writePowerOffOk();
        GPIO_ResetBits(GPIOB,GPIO_Pin_15);
     } 
     
     if (Vcurr>Vup) 
        Vcurr=Vup; 
     else if (Vcurr<Vdown) 
        Vcurr=Vdown;
     
     uint16_t percent = (uint8_t)((Vcurr-Vdown)/(Vup-Vdown)*100);
     
     //debug
     //percent = (uint16_t)(Vcurr*10);
     
     return percent;
   }

};