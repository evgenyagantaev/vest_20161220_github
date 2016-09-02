// This class provides functionality of gpio control

class clsGPIO
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //static const uint16_t 
   //*********************** end constants *************************************   


   //************************* variables ***************************************


   //*********************** end variables *************************************


   //********************* private functions ***********************************


   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsGPIO()
   {
      // Enable the GPIOA clock
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
      // Enable the GPIOB clock
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
      // Enable the GPIOC clock 
      RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

      GPIO_InitTypeDef GPIO_InitStructure; 

      
      //ON/OFF pin (PA.3) output
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;             
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOA, &GPIO_InitStructure); 
      
      //KN pin (button) (PA.2) input
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;             
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOA, &GPIO_InitStructure); 
      
      //RST bluetooth reset pin PB.5
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      
      //analog board pin PC.3
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOC, &GPIO_InitStructure); 
      
      //DEBUG
      //GPIOC->BSRRL=GPIO_Pin_3;  //analog board
      //GPIOA->BSRRL=GPIO_Pin_3;  //on/off
      //GPIOB->BSRRL=GPIO_Pin_5;  //bluetooth reset
      //while(1);
        
      //DEBUG
      
      //ST1 and ST2 pin PB.0-1
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      
      // Configure thermometer data line
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  //open drain
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      
      // Give supply to thermometer
      GPIOB->BSRRL=GPIO_Pin_2;
      
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      

   }//end clsGPIO
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   
   
   // Function sets pin PB9
 /*  void high(void)
   {
      GPIOB->BSRRL=GPIO_Pin_9;  //pb9 high
   }//end high
   
   // Function resets pin PB9
   void low(void)
   {
      GPIOB->BSRRH=GPIO_Pin_9;  //pb9 low
   }//end high
   
   // Function sets pin PB15
   void setPB15(void)
   {
      GPIOB->BSRRL=GPIO_Pin_15;  //pb15 high
   }
   
   // Function resets pin PB15
   void resetPB15(void)
   {
      GPIOB->BSRRH=GPIO_Pin_15;  //pb15 low
   }
   
   */
   
   void lock(GPIO_TypeDef* GPIOx)
   {
     __IO uint32_t tmp = 0x00010000;

  tmp |= 0xFFFF;
  /* Set LCKK bit */
  GPIOx->LCKR = tmp;
  /* Reset LCKK bit */
  GPIOx->LCKR =  0xFFFF;
  /* Set LCKK bit */
  GPIOx->LCKR = tmp;
  /* Read LCKK bit*/
  tmp = GPIOx->LCKR;
  /* Read LCKK bit*/
  tmp = GPIOx->LCKR;
   }
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};