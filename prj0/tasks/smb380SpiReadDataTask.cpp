// This task reads data from accelerometer SMB380 via SPI2 interface

void smb380SpiReadDataTask(void *pvParameters)
{
  
  //timer4.startTimer4();  //start accelerometer timer

  portTickType tc0=0,tc1=0;    //time mark
  int errCount = 0;            //time errors counter
  tc1 = xTaskGetTickCount();
  
  
   static int local_call_stack_pointer;
   char task_id = 'f';
   while(1)
   {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
           
        smb380.accReadData();        //read accelerometer data to common buffer
        vTaskDelay(delay10);       // sleep for a 10 ms     
        xSemaphoreGive(newAccDataSemaphore);
     
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
  }
   

}