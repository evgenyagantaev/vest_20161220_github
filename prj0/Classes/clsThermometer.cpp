// This class provides ...

class clsThermometer
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //static const uint16_t 
   //*********************** end constants *************************************   


   //************************* variables ***************************************
   GPIO_InitTypeDef GPIO_InitStructure; 
   
   uint8_t code[8];

   //*********************** end variables *************************************


   //********************* private functions ***********************************

   // Function reset
   inline void reset(uint16_t GPIO_Pin)
   {
      ////////////////// RESET /////////////////////
      // wait 500 uSec
      for(volatile long i=0; i<6550; i++);
      // pin low
      GPIOB->BSRRH=GPIO_Pin;
      // wait 500 uSec
      for(volatile long i=0; i<6550; i++);
      // pin high
      GPIOB->BSRRL=GPIO_Pin;
      // wait 500 uSec
      for(volatile long i=0; i<6550; i++);
      //********************************************
      
   }//end reset
   
   // Function write0
   inline void write0(uint16_t GPIO_Pin)
   {
      ///////////////// WRITE 0 ///////////////////
      // wait 10 uSec
      for(volatile long i=0; i<129; i++);
      // pin low
      GPIOB->BSRRH=GPIO_Pin;
      // wait 60 uSec
      for(volatile long i=0; i<774; i++);
      // pin high
      GPIOB->BSRRL=GPIO_Pin;
      //********************************************
      
   }//end write0
   
   // Function write1
   inline void write1(uint16_t GPIO_Pin)
   {
      ///////////////// WRITE 1 ///////////////////
      // wait 10 uSec
      for(volatile long i=0; i<129; i++);
      // pin low
      GPIOB->BSRRH=GPIO_Pin;
      // wait 10 uSec
      for(volatile long i=0; i<129; i++);
      // pin high
      GPIOB->BSRRL=GPIO_Pin;
      // wait 50 uSec
      for(volatile long i=0; i<645; i++);
      //********************************************
      
   }//end write1
   
   // Function writeByte
   inline void writeByte(uint8_t data, uint16_t GPIO_Pin)
   {
      uint8_t localData = data;
      ///////////////// WRITE BYTE ///////////////////
      for(int i=0; i<8; i++)
      {
         if((localData & (uint8_t)0x01) == (uint8_t)0x01)
            write1(GPIO_Pin);
         else
            write0(GPIO_Pin);
         localData >>=1;
      }
      //********************************************
      
   }//end writeByte
   
   // Function read1bit
   inline uint16_t read1bit(uint16_t GPIO_Pin)
   {
      uint16_t LSB = 0x0000;
      
      ///////////////// READ 1 bit ///////////////////
      // wait 10 uSec
      for(volatile long i=0; i<129; i++);
      // pin low
      GPIOB->BSRRH=GPIO_Pin;
      // wait 5 uSec
      for(volatile long i=0; i<65; i++);
      // pin high
      GPIOB->BSRRL=GPIO_Pin;
      // wait 5 uSec
      for(volatile long i=0; i<65; i++);
      // read pin condition
      if((GPIOB->IDR & GPIO_Pin) != (uint32_t)Bit_RESET)
         LSB = 0x0001;
      // wait 60 uSec
      for(volatile long i=0; i<774; i++);
      //********************************************
      
      return LSB;
      
   }//end read1bit
   
   // Function readByte
   inline uint8_t readByte(uint16_t GPIO_Pin)
   {
      uint8_t data = 0;
      
      ///////////////// READ 1 byte ///////////////////
      for(int i=0; i<8; i++)
      {
         data += (read1bit(GPIO_Pin)) << i;
      }
      //********************************************
      
      return data;
      
   }//end readByte
   
   // Function readCode
   inline void readCode(uint8_t *code, uint16_t GPIO_Pin)
   {
      for(int i=0; i<8; i++)
      {
         code[i] = readByte(GPIO_Pin);
      }
      
   }//end readCode
   
   // Function writeCode
   inline void writeCode(uint8_t *code, uint16_t GPIO_Pin)
   {
      for(int i=0; i<8; i++)
      {
         writeByte(code[i], GPIO_Pin);
      }
      
   }//end writeCode
   
   // Function read2bytes
   inline uint16_t read2bytes(uint16_t GPIO_Pin)
   {
      uint16_t data = 0;
      
      ///////////////// READ 2 bytes ///////////////////
      for(int i=0; i<16; i++)
      {
         data += (read1bit(GPIO_Pin)) << i;
      }
      //********************************************
      
      return data;
      
   }//end read2bytes
   
   // Function skipRom
   inline void skipRom(uint16_t GPIO_Pin)
   {
      writeByte(0xcc, GPIO_Pin);
      
   }//end skipRom
   
   inline void readRom(uint16_t GPIO_Pin)
   {
      writeByte(0x33, GPIO_Pin);
      
   }//end readRom
   
   inline void matchRom(uint16_t GPIO_Pin)
   {
      writeByte(0x55, GPIO_Pin);
      
   }//end readRom

   // Function convertT
   inline void convertT(uint16_t GPIO_Pin)
   {
      writeByte(0x44, GPIO_Pin);
      
   }//end convertT
   
   // Function readT
   inline uint16_t readT(uint16_t GPIO_Pin)
   {
      writeByte(0xbe, GPIO_Pin);
      return read2bytes(GPIO_Pin);
      
   }//end readT
   
   // Function configure9bit
   inline void configure9bit(uint16_t GPIO_Pin)
   {
      writeByte(0x4e, GPIO_Pin);  // write scratchpad
      writeByte(0x00, GPIO_Pin);  // alarm
      writeByte(0x00, GPIO_Pin);  // alarm
      writeByte(0x10, GPIO_Pin);  // configuration
      
   }//end configure9bit

   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      
   int16_t thermoData;
   //---------------------------------------------------------------------------
   // public constructor
   clsThermometer()
   {
      // Configure thermometer data line
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;  //open drain
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
      GPIO_Init(GPIOB, &GPIO_InitStructure);
      
      // Give supply to thermometer
      GPIOB->BSRRL=GPIO_Pin_2;
      
      reset(GPIO_Pin_2);
      
   }//end clsThermometer
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   float readThemperature(int thermo_sensor)
   {
      float themperature;
      uint16_t GPIO_Pin;
      
      if(thermo_sensor ==1)
          GPIO_Pin = GPIO_Pin_2; // PB2
      else
          GPIO_Pin = GPIO_Pin_1; // PB1
          
      
      reset(GPIO_Pin);
      skipRom(GPIO_Pin);
      //*
      convertT(GPIO_Pin);
      ///////////// wait 0.1 sec ///////////////////
      for(volatile int i=0; i<1000; i++)
      {
         // wait 1 mSec
         for( volatile int j=0; j<13100; j++);
      }
      //******************************************
      //*/
      reset(GPIO_Pin);
      //matchRom();
      skipRom(GPIO_Pin);
      //writeCode(code);
      uint16_t tAux = readT(GPIO_Pin);
      reset(GPIO_Pin);
      
      themperature = (float)((int16_t)tAux)/16.0;
      
      
      return themperature;
      
   }//end readThemperature
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};