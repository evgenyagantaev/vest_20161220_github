// this task is a part of timer 3 update isr
// task reads data from external adc via spi interface

void timer3UpdateIsrTask(void *pvParameters)
{
   // start timer3
   timer3.startTimer3();
   
   while(1)
   {
      // try to take a semaphore
      // if not timer 3 interrupt is occured yet, task will be blocked
      xSemaphoreTake(ecgTimerSemaphore);
      
      // read data from adc via spi
      uint16_t res=0, Conv=0;
      
      SPI2->DR=0x58;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      res=SPI2->DR;

      while((GPIOB->IDR & GPIO_Pin_14) != (uint32_t)Bit_RESET); //Check MISO, AD data ready
      Conv=0xFFFF;
      SPI2->DR=0xFF;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      Conv=SPI2->DR;
      Conv<<=8;
      SPI2->DR=0xFF;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET );
      Conv|=SPI2->DR;

      common.adc1BufferHead++;
      if(common.adc1BufferHead==ADC1BUFFERLEN) common.adc1BufferHead=0;
      common.adc1Buffer[common.adc1BufferHead] = Conv;
      common.samplesTimer++;
      
   }
}// end timer3UpdateIsrTask