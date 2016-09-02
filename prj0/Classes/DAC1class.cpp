/**
  ******************************************************************************
  * @file    DAC1class.cpp
  * @author  Susloparov A.A.
  * @version V1.0
  * @date    04.09.2012
  * @brief   DAC1 class @ DAC_OUT1=PA.4 pin /w no DMA
  ******************************************************************************
  */

class clsDAC1 
{
public:
   
   clsDAC1() //Конструктор класса
   {

     /* Тактирование порта вывода GPIOA */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* Тактирование ЦАП */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* Конфигурация GPIOA */
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //Пин PA.4 для DAC1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

   
    /* Конфигурация ЦАП */
     
    DAC_DeInit(); //Сброс настроек на стандартные
    
    /*
    DAC_InitTypeDef  DAC_InitStructure;
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);
    */
  
    DAC_Cmd(DAC_Channel_1, ENABLE); //Включение ЦАП, канал 1

   }
   void setData(uint16_t Data) //Послать данные на ЦАП (от 0 до 4095 *Vref)
   {    
     
    DAC_SetChannel1Data(DAC_Align_12b_R, Data);
    
   }
 
};