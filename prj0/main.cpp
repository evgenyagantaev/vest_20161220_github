// This main fail for production board version of a project

#define DEBUG                   // includes debug activities in the compiler job

#define INPUTBUFFERLENGTH 64

#define WATCHDOGTIMEOUT 250   //watchdog timeout

#define DEVICENAME "VEST-BIO733"

//uart output definitions, status string is always writing
#define SAMPLES                 // turns on samples output via uart
#define MARKERS                 // turns on markers output via uart
//#define ACCEL                   // turns on accelerometer output via uart
#define HISTOGRAMM              // turns on rr-histogramm output via uart
#define WIDEDIAGNOSIS           // turns on status with Bayevsky and hr-integral


//sdio write definitions, status string is always writing
//#define SDIOSAMPLES
//#define SDIOMARKERS                 
//#define SDIOACCEL

#define SDIOFILELIMIT //500k strings write limit

#define NOSPI1TIMEOUT           // if defined turn off spi2 flag timeout
//#define NOSDIOTIMEOUT           // if defined turn off sdio timeout
#define RAW
//#define FILTER

#define RTCLSE   
/*
use LSE (external) osc as RTC clock source (works with clock battery, power off)
if not defined source is LSI (internal) (works with power on only)
*/

#define USEFLASH 
/*
use flash read/write operations for IAP & powerOff control
don't forget set VECT_TAB_OFFSET in system_stm32f2xx.c to 0x8000 if defined 
or 0x0000 if not defined
don't forget change *.icf in linker settings
*/

#define CHECKIN(X) local_call_stack_pointer=call_stack_check_in((X));
#define CHECKOUT(X) call_stack_check_out((X), local_call_stack_pointer);

//------------------------------------------------------------------------------

#include "main.h"

#include "clsCharger.cpp"

clsCharger charger;

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"

//--------------------------- macro definitions---------------------------------

//--------------------------end macro definitions-------------------------------

//----------------------------global constants----------------------------------
//delay values (in mSeconds)
const portTickType delay1 = (portTickType)(1*(configTICK_RATE_HZ/1000));
const portTickType delay2 = (portTickType)(2*(configTICK_RATE_HZ/1000));
const portTickType delay3 = (portTickType)(3*(configTICK_RATE_HZ/1000));
const portTickType delay3_8 = (portTickType)(3800*(configTICK_RATE_HZ/1000)/1000);
const portTickType delay4 = (portTickType)(4*(configTICK_RATE_HZ/1000));
const portTickType delay7 = (portTickType)(7*(configTICK_RATE_HZ/1000));
const portTickType delay10 = (portTickType)(10*(configTICK_RATE_HZ/1000));
const portTickType delay30 = (portTickType)(30*(configTICK_RATE_HZ/1000));
const portTickType delay100 = (portTickType)(100*(configTICK_RATE_HZ/1000));
const portTickType delay200 = (portTickType)(200*(configTICK_RATE_HZ/1000));
const portTickType delay500 = (portTickType)(500*(configTICK_RATE_HZ/1000));
const portTickType delay1000 = (portTickType)(1000*(configTICK_RATE_HZ/1000));
const portTickType delay2000 = (portTickType)(2000*(configTICK_RATE_HZ/1000));
const portTickType delay3000 = (portTickType)(3000*(configTICK_RATE_HZ/1000));
const portTickType delay4000 = (portTickType)(4000*(configTICK_RATE_HZ/1000));
// time length of freertos tick (microseconds)
const portTickType tickLength = 1000000/configTICK_RATE_HZ;

const long decrementTimeoutRatio = 6700;
//--------------------------end global constants--------------------------------

//----------------------------global variables----------------------------------
#define CALL_STACK_DEPTH 128
char call_stack[CALL_STACK_DEPTH];
int call_stack_pointer =0;

char uartRecieveBuffer[INPUTBUFFERLENGTH];
int uartRecieveBufferHead = 0;

// class-container of common global variables
#include "clsCommon.cpp"
// global object, which contains all variables needed for inter-module
// communications
clsCommon common;

// text buffer for any purposes
char anyMessage32[32];

// mutex that guards call stack
xSemaphoreHandle callStackMutex;
// mutex that guards uart
xSemaphoreHandle uartMutex;
// mutex that guards sdio buffer
xSemaphoreHandle sdioMutex;
// semaphore which signals that ecg timer triggered
xSemaphoreHandle ecgTimerSemaphore;
// semaphore which signals that new ecg sample is saved in a cyclic buffer
xSemaphoreHandle newSampleSemaphore;
// semaphore which signals that new filtered ecg sample is saved in a cyclic buffer
xSemaphoreHandle newFilteredSemaphore;

// semaphore which signals that accelerometer timer triggered
//xSemaphoreHandle accTimerSemaphore;
// semaphore which signals that new acelerometer data is saved in a cyclic buffer
xSemaphoreHandle newAccDataSemaphore;

xSemaphoreHandle uartInputSemaphore;

xSemaphoreHandle sdioOutputSemaphore;

xSemaphoreHandle newEcgSampleTimerSemaphore;

// global flag; when set, signals that output through uart with dma in process
volatile int uartOutputInProcess;
uint16_t adc1Buffer[ADC1BUFFERLEN];
volatile int adc1BufferHead = 0;
// cyclic buffer for ecg samples which feed a 50 Hz filter
uint16_t filterBuffer[ADC1BUFFERLEN];
// global variable "uint16_t filterBufferHead" is an index which points to the 
// current position for writing a new sample in the buffer 
uint16_t filterBufferHead;

// cpu tick counters for profiling purposes
volatile portTickType tickCounter0, tickCounter1, tickCounter2, tickCounter3;
volatile portTickType interval0 = 0, interval1 = 0;
volatile portTickType profTick0, profTick1, profTick2, profTick3;
volatile portTickType profInterval0 = 0, profInterval1 = 0;

// cpu tick counters for timer interrupt control
volatile portTickType timerTickCounter0 = 0, timerTickCounter1 = 0;
volatile portTickType timerInterval = 0;

// cpu tick counters for timeouts
volatile portTickType timeoutInCounter, timeoutOutCounter;
volatile portTickType timeoutInterval = 0;
// timeout flag
int timeoutFlag;

// timer 3 update isr blocking flag
// set by timer 3 isr and clear by ad7792SpiReadDataTask
// if not clear, we get isr bypass
int timer3IsrBypass = 0;
// flag which signals that normal timer3 isr sequence was interrupted
int timer3SeqInterrupted = 1;

// extra sample
uint16_t extraSample0, extraSample1;
int extraSampleCounter = 0;

// vector of coefficients for polynomial approximation
//double a[4];
double a[2];
// vector of constatnt terms of a linear system
//double v[4];
double v[2];
// vector of samples which we want to approximate
//uint16_t y[4] = {20000, 20000, 20200, 20000};
uint16_t y[4];
// vector of points of approximation curve
double yy[4];

uint8_t adResponce;

// adc conversion result
volatile uint16_t Conv;

// adc fault flag
int adcFault = 0;
int adcAbnormal = 0;

// flag which signals that we have to suspend qrs detection procedure
// set when we just detected an r-pick
int qrsSuspend = 0;
// marker which marks a moment when we have found a new r-pick
int qrsSuspendMarker = 0;

//test mode flag
int testFlag=0; 

extern "C" uint8_t sdInitOk = 0, usbDevCfgd = 0;
//--------------------------end global variables--------------------------------

//--------------------------function prototypes---------------------------------
void make_hard_fault(void);
int call_stack_check_in(char id);
void call_stack_check_out(char id, int pointer);
//---------------------------classes includes-----------------------------------


#include "clsWatchdog.cpp"
clsWatchdog watchdog;

#include "clsGPIO.cpp"
clsGPIO gpio;

#include "clsThermometer.cpp"
clsThermometer thermometer;

#include "diskio.c"

#include "clsRtc.cpp"
clsRtc rtc;
#include "clsSdio_ref_tuning.cpp"
clsSdio_ref_tuning sdio;

#include "clsFlash.cpp"
clsFlash flash;


#include "clsPowerController.cpp"
clsPowerController powerController;

#include "LEDclass.cpp"

#include "UARTclass.cpp"
clsUART uart;


#include "clsDiagnost.cpp"
#include "clsMovementDetector.cpp"
#include "clsEcgAnalizer.cpp"



#include "clsSpi1_205.cpp"

#include "clsSpi2_205.cpp"


//#include "clsTimer3.cpp"
#include "clsFilter50Hz.cpp"
#include "clsPolyApproximator.cpp"

//#include "clsAd7792.cpp"
#include "clsTimer4.cpp"        // 250 Hz, ticks for a new ECG sample reading
//#include "clsAdc1.cpp"
#include "clsAds1220.cpp"

#include "clsBattery.cpp"

// This class computes isoline
#include "clsIsolineController.cpp"

#include "clsBayevsky.cpp"
#include "clsRrHistogramm.cpp"


//------------------------end classes includes----------------------------------



//---------------------global objects definitions-------------------------------




clsLED led;


clsIsolineController isolineController; // isoline calculation and adjustment
clsMovementDetector movementDetector(&uart); // detection of motion
clsEcgAnalizer ecgAnalizer; // ecg analysis
clsDiagnost diagnost;
clsFilter50Hz filter50Hz;  // filtration of 50 Hz noice
clsPolyApproximator polyApproximator;

//accel class defines
clsSpi1_205 spi1;
#include "clsSmb380.cpp"
clsSmb380 smb380;

//clsAd7792 ad7792;
clsSpi2_205 spi2;
clsAds1220 ads1220;


clsBattery battery;
//clsAdc1 adc1;

clsBayevsky bayevsky;
clsRrHistogramm rrHistogramm;


clsTimer4 timer4;


//-------------------end global objects definitions-----------------------------

//-------------------------tasks definitions------------------------------------

#include "powerControlTask.cpp"



#include "ecgAnalysisTask.cpp"
#include "isolineAdjustmentTask.cpp"
#include "diagnosticsTask.cpp"

#include "uartOutTask.cpp"


#include "movementDetectionTask.cpp"

#include "smb380SpiReadDataTask.cpp"

//#include "ad7792SpiReadDataTask.cpp"
#include "ads1220SpiReadDataTask.cpp"

#include "filter50HzTask.cpp"

#include "ledBlinkTask.cpp"


#include "batteryTask.cpp"

#include "uartInputTask.cpp"
#include "sdCardTask.cpp"
#include "bayevskyTask.cpp"


//-----------------------end tasks definitions----------------------------------


// SOME NOTABENES

// timeout with increment of counter: 13100 -> 1 milisecond (120 MHz)
//for(volatile long i=0; i<13100; i++);
// timeout with increment of counter: 129 -> 1 microsecond (120 MHz)
//for(volatile long i=0; i<129; i++);
//
//timing control <<<<<<<<<<<<<<<<<<<<<
//GPIOD->BSRRL = GPIO_Pin_5;   //pd5 high
//GPIOD->BSRRH = GPIO_Pin_5;   //pd5 low
//timing control >>>>>>>>>>>>>>>>>>>>>



extern "C" void switchToCommandMode();
extern "C" void switchToBypassMode();

void switchToCommandMode()
{
   //vTaskDelay(delay1000);
   common.sendSamples = false;
   common.sendStatus = false;
   common.sendMarkers = false;
   common.sendAccel = false;
   
   common.commandMode = true;
   sdio.closeFile();
   
   xSemaphoreTake(uartMutex, portMAX_DELAY);
   uart.sendMessage("Command mode\r\n\0");
   xSemaphoreGive(uartMutex);
   vTaskDelay(delay100);
}
void switchToBypassMode()
{
       vTaskDelay(delay3000);
       common.sendStatus = true;
      
#ifdef SAMPLES
      common.sendSamples = true;
#else
      common.sendSamples = false;
#endif
     
#ifdef MARKERS
     common.sendMarkers = true;
#else
     common.sendMarkers = false;
#endif

#ifdef ACCEL
     common.sendAccel = true;
#else
     common.sendAccel = false;
#endif     
     
           common.commandMode = false;
           sdio.openFile();
          // printf("bypass\r\n");
           vTaskDelay(delay100);
}

extern "C" {
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
}

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_dev __ALIGN_END ;





//-----------------------------------------------------------------------------
//*****************************************************************************
//                              MAIN
//*****************************************************************************
//-----------------------------------------------------------------------------

int main()
{
    uint32_t rd = 0;
    
    //powerController.setOnOffHigh(); //power on
    //led.redLedOn(); //device not ready
    //printf("startup\r\n");

  //startup sequence-----------------------------------------------
    
  
#ifdef USEFLASH
    //GPIO_SetBits(GPIOB,GPIO_Pin_15);            //set ON/OFF pin
   rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1); //read last status from flash
   // printf("%lu\r\n",rd);
#endif
   
    switch(rd)
    {
    case 0: //power-off ok, power on proc
      powerOnProcedure();
      break;
    
    case 1: //power-off err, power on
      powerOnProcedure();
      break;
    
    case 2: //iap, power on
      powerOnProcedure();
      break;
      
    default:
      powerOnProcedure();
      break;
    }

    //powerController.setSdLow();
    powerController.setRstHigh(); //bt reset pin
    
   //--------------------------------------------------------------
/*
   while(1)
   {
      // Check if the previous transmission operation complete
      while(!USART_GetFlagStatus(USART1, USART_FLAG_TC))
      // Clear the "Transmission complete" flag
      USART_ClearFlag(USART1, USART_FLAG_TC);
      // Send byte 
      USART_SendData(USART1, 'a');
   }  
    */
   //----------------------- initialization ------------------------------------
   uartOutputInProcess = 0;
   //printf("%d\r\n",uartOutputInProcess);
   //GPIOB->BSRRH=GPIO_Pin_8;  //NSS pin low (start spi communication)
   //-------------------- end initialization -----------------------------------
   
 
   //bluetooth startup configuration------------------------------------------------------------------------------------------------------------>>>>>>>>>>>>>
 
   uart.sendBytes("^#^$^%",0);  //send escape sequence if connection is active
   for(volatile long i=0; i<40000000; i++); //wait some time with no data to enter command mode if connection is active
   
   uart.sendBytes("\r\n",0);  //send escape sequence if connection is active
   for(volatile long i=0; i<40000000; i++); //wait some time with no data to enter command mode if connection is active
   
   uart.sendBytes("AT+AB HostEvent Disable\r\n",0); //disable notification strings, only for this session
   for(volatile long i=0; i<100000; i++);
   
   uart.sendBytes("AT+AB Config PIN=0000\r\n",0); //set pin
   for(volatile long i=0; i<100000; i++);
   
   uart.sendBytes("AT+AB DefaultLocalName "DEVICENAME"\r\n",0); //set device name***************************************************************************
   for(volatile long i=0; i<100000; i++);
   
   uart.sendBytes("AT+AB Bypass\r\n",0); //start connection if connection is still available
   for(volatile long i=0; i<1000000; i++);
   
   //uart.sendBytes("AT+AB Reset\r\n",0); //reset bt
   //for(volatile long i=0; i<80000000; i++);
   
   //------------------------------------------------------------------------------------------------------------------------------------------->>>>>>>>>>>>
   
    //sd card init---------------------------------------------------------------
    sdio.initSdCard();
    //---------------------------------------------------------------------------
   
    USBD_Init(&USB_OTG_dev,
    #ifdef USE_USB_OTG_HS 
    USB_OTG_HS_CORE_ID,
    #else            
    USB_OTG_FS_CORE_ID,
    #endif            
    &USR_desc,
    &USBD_MSC_cb, 
    &USR_cb);
     
      // sanity check of ads1220
      ads1220.reset();
      int check = ads1220.readConfiguration();
      if(check == 0)    // ok
      {
        led.redLedOff();

        for(int i=0; i<5; i++)
        {
          led.greenLedOn(); 
          for(volatile long i=0; i<1000000; i++);
          led.greenLedOff(); 
          for(volatile long i=0; i<1000000; i++);
        }
      }
      else // not ok
      {
        led.greenLedOff();

        for(int i=0; i<5; i++)
        {
          led.redLedOn(); 
          for(volatile long i=0; i<1000000; i++);
          led.redLedOff(); 
          for(volatile long i=0; i<1000000; i++);
        }
      }
    
    led.redLedOff();
    
    for(int i=0; i<2; i++)
    {
      led.greenLedOn(); 
      for(volatile long i=0; i<15000000; i++);
      led.greenLedOff(); 
      for(volatile long i=0; i<15000000; i++);
    }
    
    
    //////////////////////////////////////////////////////////////////////////
    //                     INITIALIZATION COMPLETE
    //////////////////////////////////////////////////////////////////////////
    // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    //make_hard_fault();
    /*
    led.greenLedOn(); 
    float temperature = 0;
    while(1)
    {
        //get temperature
        temperature = thermometer.readThemperature(1);
        sprintf(anyMessage32, "T = %7.2f\r\n", temperature);
        uart.sendBytes(anyMessage32, 0); 
    }
    */
    // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
    
   
    // create mutex for call stack guard
    callStackMutex = xSemaphoreCreateMutex();
    // create mutex for uart guard
    uartMutex = xSemaphoreCreateMutex();
    // create mutex for sdio guard
    sdioMutex = xSemaphoreCreateMutex();
    // create semaphore for ecg timer
    vSemaphoreCreateBinary(ecgTimerSemaphore);
    // create new sample semaphore
    vSemaphoreCreateBinary(newSampleSemaphore);
    // create new filtered sample semaphore
    vSemaphoreCreateBinary(newFilteredSemaphore);

    // create semaphore for accelerometer timer
    //vSemaphoreCreateBinary(accTimerSemaphore);
    // create new acelerometer data semaphore
    vSemaphoreCreateBinary(newAccDataSemaphore);

    vSemaphoreCreateBinary(uartInputSemaphore);
    xSemaphoreTake(uartInputSemaphore, portMAX_DELAY);

    vSemaphoreCreateBinary(sdioOutputSemaphore);
    xSemaphoreTake(sdioOutputSemaphore, portMAX_DELAY);

    vSemaphoreCreateBinary(newEcgSampleTimerSemaphore); 


    // ************************* tasks creation *********************************
    // task performs ecg-analysis
    xTaskCreate(ecgAnalysisTask, "ecg", configMINIMAL_STACK_SIZE, NULL, 1, NULL);                // a
    // task calculates isoline and adjusts isoline level
    xTaskCreate(isolineAdjustmentTask, "isoline", configMINIMAL_STACK_SIZE, NULL, 1, NULL);      // b
    // task performs diagnostics and transmits it via uart
    xTaskCreate(diagnosticsTask, "diagnostics", configMINIMAL_STACK_SIZE, NULL, 1, NULL);        // c
    // task transmits via uart (using DMA) all messages from all tasks
    xTaskCreate(uartOutTask, "uartOut", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);            // d
    // task detects motion activity
    xTaskCreate(movementDetectionTask, "moveDetect", configMINIMAL_STACK_SIZE, NULL, 1, NULL);   // e

    // task gets data from accelerometer
    xTaskCreate(smb380SpiReadDataTask, "smb380SPI", configMINIMAL_STACK_SIZE, NULL, 4, NULL);    // f


    // task gets data from ad7792
    //xTaskCreate(ad7792SpiReadDataTask, "ad7792SPI", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    // task gets data from ads1220
    xTaskCreate(ads1220SpiReadDataTask, "ads1220", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL);     // g

    // task performs a filtration
    xTaskCreate(filter50HzTask, "filter50Hz", configMINIMAL_STACK_SIZE, NULL, 4, NULL);          // h

    //task blinks with leds
    xTaskCreate(ledBlinkTask, "ledBlink", configMINIMAL_STACK_SIZE, NULL, 1, NULL);              // i

    //task controls the battery charge level
    xTaskCreate(batteryTask, "battery", configMINIMAL_STACK_SIZE, NULL, 1, NULL);                // j
    //task receives data from uart and decodes "interval" command
    xTaskCreate(uartInputTask, "uartCommandInput", configMINIMAL_STACK_SIZE*2, NULL, 1, NULL);   // k

    xTaskCreate(powerControlTask, "powerControl", configMINIMAL_STACK_SIZE, NULL, 1, NULL);      // l


    // task which calculates rr variability (bayevsky method)
    xTaskCreate(bayevskyTask, "bayevsky", configMINIMAL_STACK_SIZE * 2, NULL, 1, NULL);          // m



    if(sdio.initOk)
    {
        xTaskCreate(sdCardTask, "sdCard", configMINIMAL_STACK_SIZE, NULL, 1, NULL);                 // n
    }  

    #ifdef USEFLASH  
    flash.writePowerOffErr();
    #endif
    //watchdog.start();          // WATCHDOG START!!! WATCHDOG START!!! WATCHDOG START!!!

    // start scheduler
    vTaskStartScheduler();

    // we can't come here in normal conditions



    while(1)
    {
        for(long k=0;k<10000000;k++);
       
        led.greenLedOn();
        led.redLedOff();
        led.blueLedOff();

        for(long k=0;k<10000000;k++); 
      
        led.redLedOn();
        led.greenLedOff();
        led.blueLedOff();

        for(long k=0;k<10000000;k++); 
      
        led.redLedOff();
        led.greenLedOff();
        led.blueLedOn();
        
    }
  
}

int call_stack_check_in(char id)
{
    call_stack[call_stack_pointer] = id;
    if(call_stack_pointer < (CALL_STACK_DEPTH-1))
    {
        call_stack_pointer++;
        return call_stack_pointer - 1;
    }
    else
        return call_stack_pointer;
}

void call_stack_check_out(char id, int pointer)
{
    int i=0;
    while((call_stack[i] != id) && (call_stack[i] != '\0'))   
        i++;
        
    if(call_stack[i] == id)
    {
        // shift call stack
        for(int j=i; j<call_stack_pointer; j++)
            call_stack[j] = call_stack[j+1];
        // clear a new top of stack
        call_stack[call_stack_pointer] = '\0';
        // decrement call stack pointer
        call_stack_pointer--;
    }
    
}

// extremely useful function
void make_hard_fault()
{
    __asm volatile
    (
        "MOVS r0, #1       \n"
        "LDM r0,{r1-r2}    \n"
        "BX LR             \n"
    );
}