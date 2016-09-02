#include "main.h"

class clsADC2 //ADC2 class @ PC.0 pin /w Software conversion
{
public:
   uint16_t ADC2ConvValue;   
   clsADC2() //Construct
   {
    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    GPIO_InitTypeDef      GPIO_InitStructure;
    
  /* Enable ADC2 and GPIOC clocks *********************************************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
       
  /* Configure ADC2 Channel10 pin as analog input *****************************/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
    //ADC_DeInit();
    
  /* ADC Common Init **********************************************************/
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    
  /* ADC2 Init ****************************************************************/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC2, &ADC_InitStructure);
    
  /* ADC2 regular channel10 configuration *************************************/
    ADC_RegularChannelConfig(ADC2, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);

    ADC_Cmd(ADC2, ENABLE);
    
   }
   
   uint16_t GetADC2Value(void)
   {
    // Start the conversion
    ADC_SoftwareStartConv(ADC2);
    // Wait until conversion completion
    while(ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
    // Get the conversion value
    ADC2ConvValue = ADC_GetConversionValue(ADC2);
    return ADC2ConvValue;
   }
   
   float GetBatteryVoltage(void)
   {
     const float Vref = 3.3;
     return Vref*((float)GetADC2Value()/4095.0)*2;
   }

};
