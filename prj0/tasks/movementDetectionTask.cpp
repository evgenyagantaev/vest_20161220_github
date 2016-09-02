void movementDetectionTask(void *parameters)
{
   
   // task reads accelerometer samples from global cyclic buffer - 
   // uint16_t accBuffer[ADC1BUFFERLEN]
   // global variable "uint16_t accBufferHead" is an index which points to the
   // current position where a new sample was just written 
   
   // accBufferTail is a local copy of accBufferHead which points on the 
   //current position for reading a sample from buffer
   uint16_t accBufferTail;
   
   // text buffer for output via uart
   char sampleTxtBuffer[80];
   
   // first of all We copy the global index in the local variable
   accBufferTail = common.accBufferHead;
   
   //clear semaphore before the forever loop
   xSemaphoreTake(newAccDataSemaphore, portMAX_DELAY);
   
   static int local_call_stack_pointer;
   char task_id = 'e';
   while(1)
   {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
           
      // try to take a semaphore
      // if not a new sample is written in a cyclic buffer yet, task will be blocked
      xSemaphoreTake(newAccDataSemaphore, portMAX_DELAY);
      
      // new sample is written in buffer since we had read last time
      while(common.accBufferHead != accBufferTail) 
      {
         // cyclic increment of index
         accBufferTail = (accBufferTail + 1)%ACCBUFFERLEN;
         
         movementDetector.addAccSamples(common.accBuffer[accBufferTail].x, 
              common.accBuffer[accBufferTail].y, common.accBuffer[accBufferTail].z);
         
         movementDetector.movementDetection();
         
         //sprintf(sampleTxtBuffer, "%dA\r\n", common.accBuffer[accBufferTail].x);

         sprintf(sampleTxtBuffer, "Ax%dy%dz%d\r\n", 
                 common.accBuffer[accBufferTail].x,
                 common.accBuffer[accBufferTail].y,
                 common.accBuffer[accBufferTail].z);
         

         if(common.sendAccel) //write accel samples via uart if output enabled
         {
         
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(sampleTxtBuffer);
            xSemaphoreGive(uartMutex);
         
         }

#ifdef SDIOACCEL
         xSemaphoreTake(sdioMutex, portMAX_DELAY);
         if(!common.commandMode) sdio.writeStringToBuffer(sampleTxtBuffer); //write accel samples to sdio
         xSemaphoreGive(sdioMutex);
#endif         
         // yield control to scheduler
         taskYIELD();
      }
      
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
      
   }
}