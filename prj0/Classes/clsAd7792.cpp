// This class provides ...

class clsAd7792
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //static const uint16_t 
   //*********************** end constants *************************************   


   //************************* variables ***************************************


   //*********************** end variables *************************************


   //********************* private functions ***********************************


   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   //clsAd7792()
   //{
      
   //}//end clsAd7792
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   // Function resets ad7792
   void reset(void)
   {
      //In situations where the interface sequence is lost, a write operation of at least 32 serial clock cycles with DIN high
      //returns the ADC to this default state by resetting the entire part.
      // first: reset adc (set mosi line high for more then 32 clk cycles)
      for (int i=0;i<10;i++) 
      {
         while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
         SPI2->DR=0xFF;
      } 
      vTaskDelay(delay7);
      
      // Now adc waits a comm register write operation and all on-chip registers
      // are in their power-on state
      
     // *** configure configuration register:
      // gain = 4  00000<010> external reference <0>00 unbuffered mode <0>0 
      // channel 1 <000>
      // <0000001000000000> = 0x0200 
      
      // (0<0><010><0>00)0x10 - write in the configuration register 
      //(<0000001000000000> = 0x0200)
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x10;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x02;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x00;
      
      // *** configure mode register:
      // continuous conversion mode <000>00000 internal clock <00>00 470 Hz <0001>
      // <0000000000000001> = 0x0001
      
      // (0<0><001><0>00)0x08 - write in the mode register 
      //(value = <0000000000000001> = 0x0001)
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x08;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x00;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      //SPI2->DR=0x01;  // debug conversion rate = 470 Hz
      SPI2->DR=0x02; // debug conversion rate = 242 Hz
      
   }//end reset
   
   // Function reads and checks id register
   int readMode(void)
   {
      uint8_t adResponce;
      // write to the communication register <01001000>
      // the next operation will be a read from mode register
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x48;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      // fictious write in spi (just to start a clock and read mode register h byte
      SPI2->DR=0x55;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      adResponce=SPI2->DR;
      // fictious write in spi (just to start a clock and read mode register l byte
      SPI2->DR=0x55;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      adResponce=SPI2->DR;
      
      // check the value
      if(adResponce == 0x02) // we have got a correct interface
         return 0; // ok
      else
         return 1; // id
   }//end readMode
   
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};