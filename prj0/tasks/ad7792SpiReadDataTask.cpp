// This task reads data from AD7792 via SPI interface

void ad7792SpiReadDataTask(void *pvParameters)
{
   //debug
   static int emergencyFlag = 0;
   static int emergencyCounter = 5;
   
   uint16_t res=0;
   uint8_t ID=0; 
   // debug
   // text buffer for output via uart
   char sampleTxtBuffer[24];
   char sampleTxtBuffer1[24];
   char sampleTxtBuffer2[24];
   
   //debug
   int timeoutIndicator;
   
   //timing debug
   int pinHigh = 1;
   int timingResetFlag = 0;
   //--------------------------------------------------------------------------------------------------
   //ad7792

   //The communications register is an 8-bit write-only register. 
   //All communications to the part must start with a write operation to the communications register. 
   //The data written to the communications register determines whether the next operation is a read or write operation, and to which 
   //register this operation takes place. 
   //For read or write operations, once the subsequent read or write operation to the selected register is complete, the interface 
   //returns to where it expects a write operation to the communications register. 
   //This is the default state of the interface and, on power-up or after a reset, the ADC is in this default state waiting for a write 
   //operation to the communications register



   //In situations where the interface sequence is lost, a write operation of at least 32 serial clock cycles with DIN high returns the 
   //ADC to this default state by resetting the entire part.

   // 00xxx000 - write
   // 01xxx000 - read
   // 01011000 (0x58) - read of data register
   // 


   //***** configure ADC *****

   ad7792.reset();
   // delay for an adc to start conversion sequence
   vTaskDelay(delay1000);
   //vTaskDelay(delay4000);
   //vTaskDelay(delay4000);
   
   if ((SPI2->SR & SPI_I2S_FLAG_RXNE) != RESET )
      res=SPI2->DR;
   

   //phase adjustment
   //while((GPIOB->IDR & GPIO_Pin_14) != (uint32_t)Bit_RESET);
   
   //watchdog.start();          // WATCHDOG START!!! WATCHDOG START!!! WATCHDOG START!!!
   while(1)
   {
      watchdog.reload();
     
      
      // read data from adc via spi
      
      while((GPIOB->IDR & GPIO_Pin_14) != (uint32_t)Bit_RESET);
      
      if ((SPI2->SR & SPI_I2S_FLAG_RXNE) != RESET )
      res=SPI2->DR;
      // wait for spi transmitter readiness
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x58;
      // wait while a transmission complete
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      res=SPI2->DR;
      
      // wait for spi transmitter readiness
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0xFF;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      res=SPI2->DR;
      Conv=(uint16_t)res;
      
      // wait for spi transmitter readiness
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0xFF;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      res=SPI2->DR;
      Conv<<=8;
      Conv|=((uint16_t)res & 0x00ff);
      
   
      //*
      filterBufferHead++;
      if(filterBufferHead==ADC1BUFFERLEN) filterBufferHead=0;
      filterBuffer[filterBufferHead] = Conv;
      // increment a sample counter
      common.samplesTimer++;
      // increment a seconds timer
      if(common.samplesTimer%ecgAnalizer.getSamplingFrequency() == 0)
         common.secondsTimer++;
   
      xSemaphoreGive(newSampleSemaphore);
      //*/

      vTaskDelay(delay3_8);
      
   }
   
   while(1)
   {
      // take a new sample semaphore to block a task that analizes ecg (and waits
      // for a new sapmle)
      //xSemaphoreTake(newSampleSemaphore, portMAX_DELAY);
      
      // try to take a semaphore
      // if not timer 3 interrupt is occured yet, task will be blocked
      //xSemaphoreTake(ecgTimerSemaphore, portMAX_DELAY);
      
      //***** time interval control *****
      tickCounter1 = xTaskGetTickCount();
      interval0 = (tickCounter1 - tickCounter0)*tickLength;
      tickCounter0 = tickCounter1;
      //if((common.samplesTimer >1000) && ((interval0 < 3000) || (interval0 > 5000)))
      //if((common.samplesTimer >1000) && ((interval0 < 1500) || (interval0 > 2700)))
      if(0)
      {
         //time interval dramatically out of range
         //emergency!!!
         //timer3.stopTimer3();
         while(1)
         {
            printf("handler interval = %ld\r\n", interval0);
            vTaskDelay(delay1000);
            printf("ISR handler Time ALARM!\r\n");
            vTaskDelay(delay1000);
         }
      }
      //              end
      //***** time interval control *****
      
      //debug
      /*
      if(common.samplesTimer >= 100)
      {
         printf("time interval = %ld\r\n", timerInterval);
         vTaskDelay(delay1000);
         //debug stop
         while(1);
      }
      */
      
      //*** PROFILING ***
      // cpu tick counters for profiling purposes
      //tickCounter0 = xTaskGetTickCount();
      //*** PROFILING ***
      
      //***** read data from adc via spi *****
      
      //wait untill an ad7792 dout/rdy pin (rb14) goes low
      //vTaskDelay(delay2000);
      timeoutFlag = 1;
      timeoutIndicator = 0;
      timeoutInCounter = xTaskGetTickCount();
      while(((GPIOB->IDR & GPIO_Pin_14) != (uint32_t)Bit_RESET) && timeoutFlag )
      {
         timeoutOutCounter = xTaskGetTickCount();
         timeoutInterval = (timeoutOutCounter - timeoutInCounter)*tickLength;
         if(timeoutInterval >= timeoutPeriod)
            timeoutFlag = 0;
      }
      
      //*
      if(timeoutFlag) // if no timeout occured on the previous step
      { 
         timeoutIndicator++;
         // now ad data ready and can be read from the data register of the adc
         uint16_t res=0, Conv=0;
         // write in communication register (0<1><011><0>00). We command that 
         // the next operation to be a read from data register (<011>)
         SPI2->DR=0x58;
         timeoutInCounter = xTaskGetTickCount();
         while (((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET) && timeoutFlag )
         {
            timeoutOutCounter = xTaskGetTickCount();
            timeoutInterval = (timeoutOutCounter - timeoutInCounter)*tickLength;
            if(timeoutInterval >= timeoutPeriod)
               timeoutFlag = 0;
         } 
         res=SPI2->DR;
      }
      //*/
      
      if(timeoutFlag) // if no timeout occured on the previous step
      { 
         timeoutIndicator++;
         // fictious write in spi (just to start a clock and read a high byte 
         // of adc data)
         SPI2->DR=0x00;
         timeoutInCounter = xTaskGetTickCount();
         while (((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET) && timeoutFlag )
         {
            timeoutOutCounter = xTaskGetTickCount();
            timeoutInterval = (timeoutOutCounter - timeoutInCounter)*tickLength;
            if(timeoutInterval >= timeoutPeriod)
               timeoutFlag = 0;
         } 
      }
      
      if(timeoutFlag) // if no timeout occured on the previous step
      { 
         timeoutIndicator++;
         // now a high byte of data is in a spi buffer
         Conv=SPI2->DR;
         Conv<<=8;
         // fictious write in spi (just to start a clock and read a low byte 
         // of adc data)
         SPI2->DR=0x00;
         timeoutInCounter = xTaskGetTickCount();
         while (((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET) && timeoutFlag )
         {
            timeoutOutCounter = xTaskGetTickCount();
            timeoutInterval = (timeoutOutCounter - timeoutInCounter)*tickLength;
            if(timeoutInterval >= timeoutPeriod)
               timeoutFlag = 0;
         } 
      }
      
      if(timeoutFlag) // if no timeout occured on the previous step
      { 
         // now a low byte of data is in an spi buffer
         Conv|=SPI2->DR;
      }
      else // timeout occured
      {
         // timeout occured; emergency situation
         printf("SPI Timeout! %d\r\n", timeoutIndicator);
         //debug stop
         while(1);
      }  
    
      
      //*** PROFILING ***
      /*
      tickCounter2 = xTaskGetTickCount();
      if(common.samplesTimer >= 1000)
      {
         interval0 = tickCounter2 - tickCounter0;
         interval1 = tickCounter1 - tickCounter0;
         printf("whole = %ld; waiting = %ld;\r\n", interval0*tickLength, 
                tickLength*interval1);
         vTaskDelay(delay1000);
         //debug stop
         while(1);
      }
      */
      //*** PROFILING ***

      /*
      common.adc1BufferHead++;
      if(common.adc1BufferHead==ADC1BUFFERLEN) common.adc1BufferHead=0;
      //---------------------------------------------------------
      NVIC_DisableIRQ(TIM3_IRQn);
      uint16_t ind, buf;
      ind = common.adc1BufferHead;
      buf = common.adc1Buffer[common.adc1BufferHead - 1];
      common.adc1Buffer[common.adc1BufferHead] = Conv;
      NVIC_EnableIRQ(TIM3_IRQn);
      //---------------------------------------------------------
      
      
      if(Conv != common.adc1Buffer[common.adc1BufferHead])
      {
         printf("conv = %d\r\n", Conv);
         printf("buffer = %d\r\n", common.adc1Buffer[common.adc1BufferHead]);
         printf("index = %d\r\n", common.adc1BufferHead);
         printf("saved index = %d\r\n", ind);
         printf("buffer -3 = %d\r\n", common.adc1Buffer[ind - 3]);
         printf("buffer -2 = %d\r\n", common.adc1Buffer[ind - 2]);
         printf("buffer -1 = %d\r\n", common.adc1Buffer[ind - 1]);
         printf("saved buffer = %d\r\n", buf);
         printf("buffer1 = %d\r\n", common.adc1Buffer[ind + 1]);
         printf("buffer2 = %d\r\n", common.adc1Buffer[ind + 2]);
         printf("buffer3 = %d\r\n", common.adc1Buffer[ind + 3]);
         vTaskDelay(delay1000);
         
         //debug stop
         while(1);
      }
      
      */
      
      /*
      adc1BufferHead++;
      if(adc1BufferHead==ADC1BUFFERLEN) adc1BufferHead=0;
      //---------------------------------------------------------
      //NVIC_DisableIRQ(TIM3_IRQn);
      uint16_t ind, buf;
      ind = adc1BufferHead;
      buf = adc1Buffer[adc1BufferHead];
      adc1Buffer[adc1BufferHead] = Conv;
      //NVIC_EnableIRQ(TIM3_IRQn);
      //---------------------------------------------------------
      common.samplesTimer++;
      
      if(Conv != adc1Buffer[adc1BufferHead])
      {
         printf("conv = %d\r\n", Conv);
         printf("buffer = %d\r\n", common.adc1Buffer[adc1BufferHead]);
         printf("index = %d\r\n", adc1BufferHead);
         printf("saved index = %d\r\n", ind);
         printf("buffer -3 = %d\r\n", common.adc1Buffer[ind - 3]);
         printf("buffer -2 = %d\r\n", common.adc1Buffer[ind - 2]);
         printf("buffer -1 = %d\r\n", common.adc1Buffer[ind - 1]);
         printf("saved buffer = %d\r\n", buf);
         printf("buffer1 = %d\r\n", common.adc1Buffer[ind + 1]);
         printf("buffer2 = %d\r\n", common.adc1Buffer[ind + 2]);
         printf("buffer3 = %d\r\n", common.adc1Buffer[ind + 3]);
         vTaskDelay(delay1000);
         
         //debug stop
         while(1);
      }
      */
      
      
      //filterBufferHead++;
      //if(filterBufferHead==ADC1BUFFERLEN) filterBufferHead=0;
      //filterBuffer[filterBufferHead] = Conv;
      if(common.samplesTimer%3 == 0)
      {
         filterBufferHead++;
         if(filterBufferHead==ADC1BUFFERLEN) filterBufferHead=0;
         //filterBuffer[filterBufferHead] = (Conv + extraSample0 + extraSample1)/3;
         // fill a vector of points which we want to approximate
         if(filterBufferHead > 0)
            y[0] = filterBuffer[filterBufferHead-1];
         else
            y[0] = filterBuffer[ADC1BUFFERLEN-1];
         y[1] = extraSample0;
         y[2] = extraSample1;
         y[3] = Conv;
         filterBuffer[filterBufferHead] = polyApproximator.approximation(y, 4);
         //filterBuffer[filterBufferHead] = Conv;
         extraSampleCounter = 0;
      }
      else
      {
         if(extraSampleCounter == 0)
         {
            extraSample0 = Conv;
            extraSampleCounter++;
         }
         else
            extraSample1 = Conv;
      }
      
      // give semaphore to the task which utilizes a new sample
      //xSemaphoreGive(newSampleSemaphore);
      
      //debug breakepoint with data output
      //if(emergencyFlag)
      if(0)
      {
         if(emergencyCounter > 0)
         {
            emergencyCounter--;
            // yield control to scheduler
            taskYIELD();
         }
         while(1);
      }
      const int gap = 0;
      int testIndex = filterBufferHead - gap;
      if(testIndex < 0)
        testIndex =  ADC1BUFFERLEN + testIndex;
      
      //debug
      /*
      if(common.samplesTimer > 5000)
      {
         if(adc1Buffer[testIndex] < 30000)
         //if(Conv < 32300)
         //if(0)
         {
            int index = testIndex;
            while(adc1Buffer[index] < 30000)
            {
               printf("buffer = %d\r\n", adc1Buffer[index]);
               printf("index = %d\r\n", index);
               index++;
            }
            
            vTaskDelay(delay1000);
            
            //debug stop
            while(1);
         }  
      }
      */
      
      //*** PROFILING ***
      //tickCounter2 = xTaskGetTickCount();
      //*** PROFILING ***
   
      
      //debug
      if(common.samplesTimer%3 == 0)
      {
         //sprintf(sampleTxtBuffer, "%dI%d\r\n", Conv, Conv);
         //sprintf(sampleTxtBuffer, "%dI%d\r\n", extraSample1, extraSample1);
         
         //sprintf(sampleTxtBuffer, "%dI%d\r\n", adc1Buffer[testIndex],
                 //adc1Buffer[testIndex]);
         sprintf(sampleTxtBuffer, "%dI%d\r\n", filterBuffer[testIndex],
                 filterBuffer[testIndex]);
         xSemaphoreTake(uartMutex, portMAX_DELAY);
         uart.sendMessage(sampleTxtBuffer);
         xSemaphoreGive(uartMutex);
      }
      
      //*** PROFILING ***
        /*
      tickCounter3 = xTaskGetTickCount();
      interval1 += tickCounter3 - tickCounter2;
      if(common.samplesTimer >= 1000)
      {
            interval1 /= common.samplesTimer; 
         printf("interval sprintf = %ld;\r\n", interval1*tickLength); 
         vTaskDelay(delay1000);
         //debug stop
         while(1);
      }
        */
      //*** PROFILING ***
      
      // clear flag, which signals that we stopped interrupt service procedure
      timer3IsrBypass = 0;
      
      // yield control to scheduler
      //taskYIELD();
      vTaskDelay(delay1);
      
   }//end while(1) (superloop)
   
}// end ad7792SpiReadDataTask





