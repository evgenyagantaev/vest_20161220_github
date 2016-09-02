// this class is a container for a global variables, which are used for 
// intertask communications

#define TUNING_VECTOR_LENGTH 300




class clsCommon
{
private:

public:
    
    // movement activity
    int movement;
    
    struct tuning_vector_element_struct
    {
        uint16_t heart_rate;
        long tick_counter;
        uint8_t movement;
    };
    typedef struct tuning_vector_element_struct tuning_vector_element; 
    
    // heart rate vecor for individual tuning
    tuning_vector_element tuning_vector[TUNING_VECTOR_LENGTH];
    //tuning_vector_element tuning_vector1[TUNING_VECTOR_LENGTH];
    int current_tuning_vector_length;
    bool tuning_mode;
    bool stage1_tuning;
    bool stage2_tuning;
    bool stage3_tuning;
    bool tuning_started;
    long timer_tuning_mode;
   
   // counter of ecg samples
   // we use it for measuring of short time intervalsn (1 LSB = 4 ms)
   long samplesTimer;
   
   // counter to measure seconds
   long secondsTimer;
   
   // cyclic buffer for ecg samples which feed a 50 Hz filter
   uint16_t filterBuffer[ADC1BUFFERLEN];
   // global variable "uint16_t filterBufferHead" is an index which points to the 
   // current position for writing a new sample in the buffer 
   uint16_t filterBufferHead;
   
   // cyclic buffer for ecg samples
   // here we collect filtered samples (a 50 Hz filter drops them here)
   uint16_t adc1Buffer[ADC1BUFFERLEN];
   // global variable "uint16_t adc1BufferHead" is an index which points to the 
   // current position for writing a new sample in the buffer 
   uint16_t adc1BufferHead;
   
   uint16_t outputDelay; //output delay in seconds, for task delay = outputDelay*delay1000
   
   // structure for accelerometer samples
   struct acceleration
   {
      int x;
      int y;
      int z;
   };
   // cyclic buffer for accelerometer samples
   acceleration accBuffer[ACCBUFFERLEN];
   // global variable "uint16_t adc1BufferHead" is an index which points to the 
   // current position for writing a new sample in the buffer 
   uint16_t accBufferHead;
   
   static const int INBUFFERLENGTH = 32; //recieve buffer length
   char uartRecieveBuffer[INBUFFERLENGTH];
   int uartRecieveBufferHead;
   
   uint8_t batteryCharge; //battery charge in percents
   
   bool commandMode,sendStatus,sendSamples,sendMarkers,sendAccel; //flags
   
   bool chargeDone, charging;
   
   int needForCharge;   // signals that charge level is below 80%
   
   int heartRate;
   
   long heartRateIntegral;
   int heartRateWeight;
   
   
   // rtc debug and tuning block
   bool lse_start_ok;
   
   
   // *****<<< PUBLIC CONSTRUCTOR >>>*****
   clsCommon()
   {
       movement = 0;
       
       lse_start_ok = false;
       
       current_tuning_vector_length = 0;
       tuning_mode = false;
       tuning_started = false;
       stage1_tuning = false;
       stage2_tuning = false;
       stage3_tuning = false;
       timer_tuning_mode = 0;
       
      commandMode = false;
      
      sendStatus = true;
      
#ifdef SAMPLES
      sendSamples = true;
#else
      sendSamples = false;
#endif
     
#ifdef MARKERS
     sendMarkers = true;
#else
     sendMarkers = false;
#endif
     
#ifdef ACCEL
     sendAccel = true;
#else
     sendAccel = false;
#endif
      
      adc1BufferHead = 0;
      accBufferHead = 0;
      samplesTimer = 0;
      secondsTimer = 0;
      filterBufferHead = 0;
      uartRecieveBufferHead = 0;
      batteryCharge = 83;
      
      outputDelay=1;
      
      for(int i=0;i<ACCBUFFERLEN;i++) //init acelerometer common buffer
      {
        accBuffer[i].x = 0;
        accBuffer[i].y = 0;
        accBuffer[i].z = 0;
      }
      
      memset(&uartRecieveBuffer[0],'_',INBUFFERLENGTH); //fill empty recieve buffer
      
     needForCharge = 0;
     
     heartRate = 444;
     
     heartRateIntegral = 0;
     heartRateWeight = 1;
      
   }

};
