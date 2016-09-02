#include "main.h"

class clsLED //LED class @ PC.0-2
{
public:
   
   clsLED() //Construct
   {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //Enable GPIOC port

    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;  //PC.0-2 pin for rgb led           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //no pull resistor
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    redLedOff();
    greenLedOff();
    blueLedOff();
   }
   
   void LED_On(void) //on/off led, default led is green
   {
    greenLedOn();
   }
   void LED_Off(void)
   {
    greenLedOff();
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
   
};