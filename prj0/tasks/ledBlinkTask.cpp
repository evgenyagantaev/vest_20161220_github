void ledBlinkTask(void *parameters)
{
   
  
   int i = 0;
   
   bool sw=true;
   
    static int local_call_stack_pointer;
    char task_id = 'i';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
  
        if(common.charging)
        {
            if(common.chargeDone)
            {
              led.blueLedOn();
              led.redLedOff();
              led.greenLedOff();
              vTaskDelay(delay4000);
            }
            else
            {
             if(sw)
             {
                led.blueLedOn();
                led.redLedOff();
                led.greenLedOff(); 
                //i++;
             }
             else
             {
                led.blueLedOff();
                led.redLedOff();
                led.greenLedOff();
             }
             sw=!sw;
             vTaskDelay(delay500); 
            }

        }
        else
        {
            //battery.getChargeCurrent();
            if(!sdio.alarmMode) 
                led.blueLedOff();
            else 
                led.blueLedOn();
            
            led.greenLedOff();
            led.redLedOff();

            if(common.commandMode)
            {
                led.greenLedOn();
                led.redLedOn();
                vTaskDelay(delay500); 
            }
            else if(common.batteryCharge >= 13) // battery norma (green led blink)
            {
                led.redLedOff();

                if(i==0)
                {
                    led.greenLedOn(); 
                    i++;
                    //GPIOB->BSRRL=GPIO_Pin_8;  //NSS pin high
                }
                else    
                {
                    led.greenLedOff();
                    i = 0;
                    //GPIOB->BSRRH=GPIO_Pin_8;  //NSS pin low
                }

                vTaskDelay(delay500); 
            }
            else    // battery low (red led blink)
            {
                led.greenLedOff();

                if(i==0)
                {
                    led.redLedOn();
                    i++;
                    //GPIOB->BSRRL=GPIO_Pin_8;  //NSS pin high
                    }
                else
                {
                    led.redLedOff();
                    i = 0;
                    //GPIOB->BSRRH=GPIO_Pin_8;  //NSS pin low
                }

                vTaskDelay(delay500); 
            }

        }

        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
   }
}