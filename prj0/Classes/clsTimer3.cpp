// This class provides functionality of timer TIM3

class clsTimer3
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //*********************** end constants *************************************   


   //************************* variables ***************************************


   //*********************** end variables ************************************


   //********************* private functions **********************************
   void init_timer3(int period)
   {
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
      TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
      TIM_TimeBaseStructure.TIM_Prescaler = 600;
      TIM_TimeBaseStructure.TIM_Period = period;   //213 -> 470 Hz  (400 -> 250 Hz)
      TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
      TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

      //TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

      //TIM_Cmd(TIM3, ENABLE);
      //NVIC_EnableIRQ(TIM3_IRQn);
   }// end init_tim3

   
   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsTimer3()
   {
      // timer initialization:
      init_timer3(260);
   }//end clsTimer3
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   // function starts timer3 and enables timer 3 IRQs
   void startTimer3(void)
   {
      TIM_Cmd(TIM3, ENABLE);
      NVIC_EnableIRQ(TIM3_IRQn);
   }
   
   // function stops timer3 and disables timer 3 IRQs
   void stopTimer3(void)
   {
      TIM_Cmd(TIM3, DISABLE);
      NVIC_DisableIRQ(TIM3_IRQn);
   }
   
   void setPeriod(int period)
   {
      init_timer3(period);
   }
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Method ...
   void someMethod(void)
   {
      
   }//end someMethod
   //--------------------------end----------------------------------------------
   
      
};

// timer 3 interrupt service procedure
extern "C" void TIM3_IRQHandler(void)
{
   //if(timer3IsrBypass)
   if(0)
   {
      // previous interrupt is not processed yet
      // we have to bypass an interrupt procedure
      // and set flag (abnormal isr)
      timer3SeqInterrupted = 1;
   }
   else
   {
      // set flag, which signals that we started interrupt service procedure
      timer3IsrBypass = 1;
      
      portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
      
      //***** time interval control *****
      /*
      timerTickCounter1 = xTaskGetTickCountFromISR();
      if(timer3SeqInterrupted) // normal isr sequence was interrupted
      {
         // clear flag
         timer3SeqInterrupted = 0;
         // set "good" interval value
         //timerInterval = 4000;
         timerInterval = 2130;
      }
      else
      {
         timerInterval = (timerTickCounter1 - timerTickCounter0)*tickLength;
      }
      timerTickCounter0 = timerTickCounter1;
      //if((common.samplesTimer >1000) && ((timerInterval < 3000) || (timerInterval > 5000)))
      if((common.samplesTimer >1000) && ((timerInterval < 1500) || (timerInterval > 2700)))
      {
         //time interval dramatically out of range
         //emergency!!!
         while(1)
         {
            printf("timer3 isr time control\r\n");
            printf("time interval = %ld\r\n", timerInterval);
            vTaskDelay(delay1000);
            printf("ALARM!\r\n");
            vTaskDelay(delay1000);
         }
      }
      */
      //              end
      //***** time interval control *****

      // clear bit
      if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
         TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
      
      // give semaphore to the ISR handler task
      xSemaphoreGiveFromISR(ecgTimerSemaphore, &xHigherPriorityTaskWoken);
      //GPIOB->BSRRH=GPIO_Pin_9;  //pb9 low
      
      // do the context switch, if necessary
      portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
      
   }//end if(timer3IsrBypass)
}
