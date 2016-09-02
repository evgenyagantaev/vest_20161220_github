// This class provides functionality of power control (power on and off)

class clsPowerController
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   static const int POWERONINTERVAL = 2000; // miliseconds
   static const int POWEROFFINTERVAL = 2500; // miliseconds
   //*********************** end constants *************************************   


   //************************* variables ***************************************
   
   // flag indicates that power on proceduresuccessfully finished
   int powerOn;
   
   

   //*********************** end variables *************************************


   //********************* private functions ***********************************


   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************  

    long adc_out_of_range_counter;

   //---------------------------------------------------------------------------
   // public constructor
   clsPowerController()
   {
      setAbLow();
      powerOn = 0;

      RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
      PWR_PVDLevelConfig(PWR_PVDLevel_6);
      PWR_PVDCmd(ENABLE);
      
      adc_out_of_range_counter = 0;
   }//end clsPowerController
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   int powerOnInterval()
   { return POWERONINTERVAL;}
   int powerOffInterval()
   { return POWERONINTERVAL;}
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   
   void setOnOffHigh()
   {
     GPIOA->BSRRL=GPIO_Pin_3;
   }
   
   void setOnOffLow()
   {
     GPIOA->BSRRH=GPIO_Pin_3;
   }
   
   void setAbHigh() //Analog board
   {
     GPIOC->BSRRL=GPIO_Pin_3;
   }
   
   void setAbLow()
   {
     GPIOC->BSRRH=GPIO_Pin_3;
   }
   
   uint8_t getKnPin()
   {
     return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2);
   }
   
   void setRstHigh() //bluetooth reset
   {
     GPIOB->BSRRL=GPIO_Pin_5;
   }
   
   void setRstLow()
   {
     GPIOB->BSRRH=GPIO_Pin_5;
   }

void setSdHigh() //sd charge pin
   {
     GPIOB->BSRRL=GPIO_Pin_2;
   }
   
   void setSdLow()
   {
     GPIOB->BSRRH=GPIO_Pin_2;
   }
   
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};