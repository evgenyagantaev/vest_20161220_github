// This task reads data from AD7792 via SPI interface

void ads1220SpiReadDataTask(void *pvParameters)
{
    
   //debug
   static int counter = 0;
   
   uint8_t aux;
   uint32_t adc_data;
   

   //timing debug
   int pinHigh = 1;
   int timingResetFlag = 0;
   

   ads1220.reset();
   watchdog.start();    // WATCHDOG START!!! WATCHDOG START!!! WATCHDOG START!!!
   timer4.startTimer4();
   
   if ((SPI2->SR & SPI_I2S_FLAG_RXNE) != RESET )
      aux=SPI2->DR;
   
   
    int high = 0;
    
    
    static int local_call_stack_pointer;
    char task_id = 'g';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
       
        // block on waiting of 250 Hz timer tick semaphore
        xSemaphoreTake(newEcgSampleTimerSemaphore, portMAX_DELAY);

        watchdog.reload();
        
        //read data
        adc_data = 0;
        while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
        // fictious write in spi (just to start a clock and read data
        SPI2->DR=0x00;
        while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
        aux=SPI2->DR;
        adc_data |= aux;
        adc_data <<= 8;
        // fictious write in spi (just to start a clock and read data
        SPI2->DR=0x00;
        while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
        aux=SPI2->DR;
        adc_data |= aux;
        adc_data <<= 8;
        // fictious write in spi (just to start a clock and read data
        SPI2->DR=0x00;
        while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
        aux=SPI2->DR;
        adc_data |= aux;
        
        //DEBUG
        /*
        if(counter == 0)
        {
            counter = 1;
            led.blueLedOff(); 
        }
        else
        {
            counter = 0;
            led.blueLedOn(); 
        }
        */
        //DEBUG
        
        
        Conv = (0xffffff - adc_data) >> 8;
        
        
        filterBufferHead++;
        if(filterBufferHead >= ADC1BUFFERLEN) 
            filterBufferHead=0;
        filterBuffer[filterBufferHead] = Conv;
        // increment a sample counter
        common.samplesTimer++;
        // increment a seconds timer
        if((common.samplesTimer % (long)(ecgAnalizer.getSamplingFrequency())) == 0)
         common.secondsTimer++;

        xSemaphoreGive(newSampleSemaphore);
        
        
        //DEBUG **************************************************
        //DEBUG **************************************************
        
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
      
    }
   

   
}// end ads1220SpiReadDataTask





