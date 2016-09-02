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
   
   clsDAC1() //����������� ������
   {

     /* ������������ ����� ������ GPIOA */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* ������������ ��� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* ������������ GPIOA */
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //��� PA.4 ��� DAC1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

   
    /* ������������ ��� */
     
    DAC_DeInit(); //����� �������� �� �����������
    
    /*
    DAC_InitTypeDef  DAC_InitStructure;
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);
    */
  
    DAC_Cmd(DAC_Channel_1, ENABLE); //��������� ���, ����� 1

   }
   void setData(uint16_t Data) //������� ������ �� ��� (�� 0 �� 4095 *Vref)
   {    
     
    DAC_SetChannel1Data(DAC_Align_12b_R, Data);
    
   }
 
};