// This task performs analysis of ecg samples
void ecgAnalysisTask(void *parameters)
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
   char sampleTxtBuffer[24];

   // on the start of job We have to collect some set of samples in the buffer 
   // (>=250)
#define MINIMALNUMBEROFSAMPLES 250
   // if there not yet minimal collection of samples, block for 1000 ms. 
   while(adc1BufferTail < MINIMALNUMBEROFSAMPLES)
   {
      vTaskDelay(delay1000);
      adc1BufferTail = adc1BufferHead;
   }
   
   // if We came here, then We have collected minimal number of samples and
   // We can start an analysis now
   
   //debug
   //printf("start index = %d\r\n", adc1BufferTail);
   adc1BufferTail = adc1BufferHead;
   //clear semaphore before the forever loop
   xSemaphoreTake(newFilteredSemaphore, portMAX_DELAY);
   
   static int local_call_stack_pointer;
   char task_id = 'a';
   while(1) // superloop
   {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
      
      // try to take a semaphore
      // if not a new sample is written in a cyclic buffer yet, task will be blocked
      xSemaphoreTake(newFilteredSemaphore, portMAX_DELAY);
      
      // new sample is written in buffer since we had read last time
      while(adc1BufferHead != adc1BufferTail) 
      {
         
         // cyclic increment of index
         adc1BufferTail = (adc1BufferTail + 1)%ADC1BUFFERLEN;
         
         // increment of asystoly counter
         ecgAnalizer.asystolyShiftCounterInc();
         
         //debug 
         //sprintf(sampleTxtBuffer, "%dI%d\r\n", Conv,
                 //isolineController.currentIsoline);
         //sprintf(sampleTxtBuffer, "%dI%d\r\n", adc1Buffer[adc1BufferTail],
                 //isolineController.currentIsoline);
         sprintf(sampleTxtBuffer, "%dI%d\r\n", ecgAnalizer.oldSample()/9,
                 ecgAnalizer.oldIsoline()/9);
         
        
   
         if(common.sendSamples)
         {
         
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(sampleTxtBuffer);
            xSemaphoreGive(uartMutex);
         }

     #ifdef SDIOSAMPLES    
         xSemaphoreTake(sdioMutex, portMAX_DELAY);     
         if(!common.commandMode) sdio.writeStringToBuffer(sampleTxtBuffer);
         xSemaphoreGive(sdioMutex); 
     #endif   
         
         // add sample and isoline level to local windows for analysis
         ecgAnalizer.addSample(adc1Buffer[adc1BufferTail], 
               isolineController.currentIsoline, adc1BufferTail);
         
         uint16_t isoline;
         isoline = isolineController.currentIsoline;
         int isolineOutOfRange = (isoline < diagnost.LOWBOUNDADCDIAPAZONE) ||
                                 (isoline > diagnost.UPPERBOUNDADCDIAPAZONE);
         
         if(isolineOutOfRange)
         {
            diagnost.badAdcRange = 1;
            diagnost.adcRangeTimer = common.secondsTimer;
            diagnost.badIsoline = isoline;
         }
         else // isoline within range
         {
            if((common.secondsTimer - diagnost.adcRangeTimer) > diagnost.ADCRANGEPERIOD)
               diagnost.badAdcRange = 0;
         }
         
         // detect r-pick
         ecgAnalizer.qrsDetect();
         
         // check if a new regular rr interval present
         if(ecgAnalizer.getNewRegularHeartRateReady())
         {
            bayevsky.saveNewRR(ecgAnalizer.getLastRR());
            ecgAnalizer.resetNewRegularHeartRateReady();
         }
         

         //debug*************************
         // draw vertical markers
         char *wMark = "W\r\n";         // white marker
         char *rMark = "R\r\n";         // red marker
         char *bMark = "B\r\n";         // blue marker
         
         int marker = ecgAnalizer.marker();
         
#ifdef SDIOMARKERS   
         xSemaphoreTake(sdioMutex, portMAX_DELAY);
         switch(marker)
         {
         case 1:
            if(!common.commandMode) sdio.writeStringToBuffer(wMark);
            break;
         case 2:
            if(!common.commandMode) sdio.writeStringToBuffer(rMark);
            break;
         case 3:
            if(!common.commandMode) sdio.writeStringToBuffer(bMark);
            break;
         }
         xSemaphoreGive(sdioMutex);
 #endif      
         
         if(common.sendMarkers)
         {
         switch(marker)
         {
         case 1:
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(wMark);
            xSemaphoreGive(uartMutex);
            break;
         case 2:
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(rMark);
            xSemaphoreGive(uartMutex);
            //debug
            //printf("%s", rMark);
            break;
         case 3:
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(bMark);
            xSemaphoreGive(uartMutex);
            break;
         }
         }
         //end debug*************************

         
         
         //vTaskDelay(delay500);
         taskYIELD();
         
      }//end while(adc1BufferHead != adc1BufferTail) 
      
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
      
   }// end while superloop
   
}
