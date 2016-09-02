void powerOffProcedure()
{
   
   const int numberOfIntervals = 100;
   long pause;
   pause = (powerController.powerOffInterval()/numberOfIntervals)*decrementTimeoutRatio;
   
   int buttonPressed = 1;
   
   led.redLedOn(); // power button pressed
   led.greenLedOff(); 
   
   volatile int i = 0;
   while((i<numberOfIntervals) && buttonPressed)
   {
      watchdog.reload();
      // pause ************************
      int x=5, y=6;
      volatile long timeoutCounter = pause;
      while((x != y) && (timeoutCounter > 0))
      {
         x = 5;
         timeoutCounter--;
      }
      //******************************
      
      if(!powerController.getKnPin()) 
      {
        //button is not pressed
         buttonPressed = 0;
      }
      
      i++;
   }
   
   if(buttonPressed && (i >= numberOfIntervals)) // power off
   {
       NVIC_DisableIRQ(TIM4_IRQn);
       
       watchdog.reload();
      sdio.closeFile();
      watchdog.reload();
      
#ifdef USEFLASH
      uint32_t rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1);
      if(rd!=2) flash.writePowerOffOk();
#endif
      
      // blink twice with red and green leds
      for(int i=0; i<2; i++)
      {
         led.greenLedOn(); 
         led.redLedOff(); 
         for(volatile long i=0; i<1500000; i++)
            watchdog.reload();
         led.greenLedOff(); 
         led.redLedOn(); 
         for(volatile long i=0; i<1500000; i++)
            watchdog.reload();
      }
      
      led.redLedOff(); 
      led.greenLedOff(); 
      //GPIO_ResetBits(GPIOB,GPIO_Pin_15);       //reset ON/OFF pin
      powerController.setOnOffLow(); //power off
      while(1);
   }
   else // early releise of button
   {
      led.greenLedOn(); 
      led.redLedOff(); 
   }
}


void powerControlTask(void *parameters)
{
 
    static int local_call_stack_pointer;
    char task_id = 'l';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
       
        if(powerController.getKnPin()) 
        {
            //button is pressed
            // start power off procedure
            powerOffProcedure();
        }

        if(PWR_GetFlagStatus(PWR_FLAG_PVDO)==SET)
        {
            powerController.setOnOffLow();
        }

        // check out of body event
        if(isolineController.currentIsoline == 32768)
        {
            if(powerController.adc_out_of_range_counter == 0) // firstly appeared
                powerController.adc_out_of_range_counter = common.secondsTimer;   // fix start out of body time
            else
            {
                if((common.secondsTimer - powerController.adc_out_of_range_counter) >= 600) // 10 minutes out of body
                {
                    powerController.setRstLow(); // bluetooth off
                    powerController.setAbLow();  // analog board off
                    led.greenLedOn(); 
                    led.redLedOn(); 
                    led.blueLedOn(); 
                    // pause
                    for(volatile long i=0; i<15000000; i++);
                    for(volatile long i=0; i<15000000; i++);
                    for(volatile long i=0; i<15000000; i++);
                    powerController.setOnOffLow(); //power off
                }
            }
        }
        else
            powerController.adc_out_of_range_counter = 0;

        // yield control to scheduler
        vTaskDelay(delay500);

        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
    }
}

void powerOnProcedure()
{
   //GPIO_SetBits(GPIOB,GPIO_Pin_15);            //set ON/OFF pin  
   //GPIO_ResetBits(GPIOB,GPIO_Pin_14);          //reset Delta R pin
   powerController.setOnOffHigh(); //power on
   
    if(PWR_GetFlagStatus(PWR_FLAG_PVDO)==SET)
    {
        powerController.setOnOffLow();
    }
   
   const int numberOfIntervals = 100;
   long pause;
   pause = (powerController.powerOnInterval()/numberOfIntervals)*decrementTimeoutRatio;
   
   int buttonPressed = 1;
   
   int counter = 0;
   for(int i=0; i<numberOfIntervals; i++)
   {
      counter++;
      
      // pause ************************
      int x=5, y=6;
      volatile long timeoutCounter = pause;
      while((x != y) && (timeoutCounter > 0))
      {
         x = 5;
         timeoutCounter--;
      }
      //******************************
      
      if(!powerController.getKnPin()) 
      {
        //button is not pressed
         buttonPressed = 0;
      }
   }
   
   if(!buttonPressed)
   {
     //GPIO_ResetBits(GPIOB,GPIO_Pin_15); 
        powerController.setOnOffLow();
   }
   else
   {

      //GPIO_SetBits(GPIOB,GPIO_Pin_15);       //set ON/OFF pin
      powerController.setOnOffHigh();
      powerController.setAbHigh();  // analog board
      powerController.setRstHigh(); // bluetooth
      // blink twice with red and green leds
      for(int i=0; i<2; i++)
      {
         led.greenLedOn(); 
         led.redLedOff(); 
         for(volatile long i=0; i<3000000; i++);
         led.greenLedOff(); 
         led.redLedOn(); 
         for(volatile long i=0; i<3000000; i++);
      }
      
      led.redLedOff(); 
      led.greenLedOn(); 
   }
}