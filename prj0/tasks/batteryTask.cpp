//This task provides battery charge read via ADC1

void batteryTask(void *pvParameters)
{
  /*
  int i=0;
  
  common.charging = false;
  common.chargeDone = false; 
  */
  static int local_call_stack_pointer;
   char task_id = 'j';
   while(1)
   {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
    
        //common.batteryCharge = /*adc1.getBatteryVoltagePercent();*/11;

        common.batteryCharge = battery.getBatteryCharge();

        if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))
        {
            vTaskDelay(delay3000);
            if(!usbDevCfgd)
            {

                sdio.closeFile();
                 
                #ifdef USEFLASH
                uint32_t rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1);
                if(rd!=2) flash.writePowerOffOk();
                #endif
                 
                NVIC_SystemReset();
            }
        }
        else
        {
         common.charging = false;
         common.chargeDone = false; 
         
        }



        vTaskDelay(delay4000); //delay 4s
   
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
  }
}