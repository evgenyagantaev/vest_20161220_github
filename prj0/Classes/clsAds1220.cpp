// This class provides ...

class clsAds1220
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
   clsAds1220()
   {
      
   }//end clsAds1220
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   // Function resets ads1220
   void reset(void)
   {
       uint8_t aux;
       
       if ((SPI2->SR & SPI_I2S_FLAG_RXNE) != RESET )
        aux=SPI2->DR;
       
       // write RESET command (0x06)
       while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
       SPI2->DR=0x06;
       // fictious reading
       while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
       aux = SPI2->DR;
       
       
        volatile long i;
        for(i=0; i<1500000; i++);
       
       
      // *** configure configuration register:
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x43;    // WREG 4 registers starting from address 00
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      
      // write in the configuration registers
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x64;    // + ain1; - ain0 gain 4
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x94;    // turbo, 660 sps
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x00;    // default
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x00;    // default
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      
      // START/SYNC command
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x08;    // START/SYNC
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      
      
   }//end reset
   
   // Function reads and checks id register
   int readConfiguration(void)
   {
       static uint8_t aux;
       
       static volatile uint8_t configuration[4];
      
      if ((SPI2->SR & SPI_I2S_FLAG_RXNE) != RESET )
        aux=SPI2->DR;
       
      // send read configuration registers command:
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      SPI2->DR=0x23;    // PREG 4 registers starting from address 00
      // fictious reading
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      aux = SPI2->DR;
      
      // read registers
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      // fictious write in spi (just to start a clock and read data
      SPI2->DR=0x00;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      configuration[0]=SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      // fictious write in spi (just to start a clock and read data
      SPI2->DR=0x00;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      configuration[1]=SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      // fictious write in spi (just to start a clock and read data
      SPI2->DR=0x00;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      configuration[2]=SPI2->DR;
      while ((SPI2->SR & SPI_I2S_FLAG_TXE) == RESET );
      // fictious write in spi (just to start a clock and read data
      SPI2->DR=0x00;
      while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == RESET);
      configuration[3]=SPI2->DR;
      
      
      // check the value
      if(configuration[0] == 0x64 && configuration[1] == 0x94) // we have got a correct interface
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