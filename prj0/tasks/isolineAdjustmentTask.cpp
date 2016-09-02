// This task computes isoline and performs it's adjustment


void isolineAdjustmentTask(void *parameters)
{
   
   
   // task reads ecg samples from global cyclic buffer - 
   // uint16_t adc1Buffer[ADC1BUFFERLEN]
   // global variable "uint16_t adc1BufferHead" is an index which points to the
   // current position where a new sample was just written 
   
   // adc1BufferTail is a local copy of adc1BufferHead which points on the 
   //current position for reading a sample from buffer
   uint16_t adc1BufferTail;
   
   // first of all We copy the global index in the local variable
   adc1BufferTail = adc1BufferHead;
   
   // text buffer for output via uart
  
   
    static int local_call_stack_pointer;
    char task_id = 'b';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
           
        // new sample is written in buffer since we had read last time
        while(adc1BufferHead != adc1BufferTail) 
        {
            // cyclic increment of index
            adc1BufferTail = (adc1BufferTail + 1)%ADC1BUFFERLEN;
            // get new value of isoline level
            isolineController.getIsoline(adc1Buffer[adc1BufferTail]);
            
            if(isolineController.currentIsoline == 32768)
                diagnost.badAdcRange = 1;

            // yield control to scheduler
            taskYIELD();
        };
      
        // sleep for 4 mSeconds
        vTaskDelay(delay4);

        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
      
   }
   
}


