//this class provides SDIO interface

#include "stm32_sdio_common.h"
#include "stm32_sdio_sd.h"

class clsSdio
{
  //public zone----------------------------------------------------------------
  public:
    FIL outputFile; //file handler
    bool initOk; //sd card and fs init control value
    static const int OUTPUTBUFFERLENGTH = 500;
    
    //char outputBuffer[2][OUTPUTBUFFERLENGTH]; //two output buffers
    char buffer1[OUTPUTBUFFERLENGTH];
    char buffer2[OUTPUTBUFFERLENGTH];
    uint8_t activeBuffer;      //active buffer index
    uint16_t bufferHead;
    bool isWriting;
    uint32_t strcount;
    clsSdio() //constructor, common class init
    {
      initOk=false;
      isWriting = false;
      strcount=0;
    }
    
    void resetBuffers()
    {
      activeBuffer=0;
      bufferHead=0;
    }
    void NVIC_Configuration(void) //interrupts processing init
    {
      NVIC_InitTypeDef NVIC_InitStructure;

      /* Configure the NVIC Preemption Priority Bits */
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

      NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);
      NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
      NVIC_Init(&NVIC_InitStructure);  
    }
    
    uint8_t initSdCard() //sd card init proc
    {
      NVIC_Configuration();
      /*------------------------------ SD Init ---------------------------------- */
      
      if(SD_Init()!=SD_OK) //init sd card
      {
        return 0;
      }
      else 
      {
        
        static FATFS fatfs; 
        FRESULT res;
       
        res = f_mount(0, &fatfs); //mount fs
        if(res != FR_OK) return 0;
        
        //printf("mount %d\r\n",res);
        

        initOk = true;
        openFile(); //open main output file
        //printf("sd card init ok\r\n");
        return 1;
      }
    }
    
    DWORD getFreeSpace()
    {
      FATFS *fs;
      DWORD fre_clust;
      FRESULT res;
      res = f_getfree("0:", &fre_clust, &fs);
      //printf("cs %u\r\n",fs->csize);
      if(res==FR_OK) return fre_clust*fs->csize/2;
      else return 0;
    }
    
    FRESULT openFile() //main file open proc
    {
      if(initOk)
      {
      resetBuffers();
      uint16_t num=0;
      
      //rtc.setDateTime(13,2,13,15,31,0);
      set_fattime(rtc.getFatDateTime());
      uint32_t ftime = (uint32_t)get_fattime(); //get timestamp
      
      uint16_t day,month,year,hour,minute,second;
      
      year = (ftime>>25)+1980;
      month = (ftime>>21)&0xF;
      day = (ftime>>16)&0x1F;
      
      hour = (ftime>>11)&0x1F;
      minute = (ftime>>5)&0x3F;
      second = ((ftime)&0xF)*2;
      
      sprintf(outputFileName,"%02u%02u%04u_%02u%02u%02u_%03u.dat",day,month,year,hour,minute,second,num); //set default file name
     
      FILINFO FileInfo;
#if _USE_LFN
      static char lfn[_MAX_LFN + 1];
      FileInfo.lfname = lfn;
      FileInfo.lfsize = sizeof(lfn);
#endif
 
      while(f_stat(outputFileName,&FileInfo)!=FR_NO_FILE) //check if file is exist and set new name
      {
        num++;
        sprintf(outputFileName,"%02u%02u%04u_%02u%02u%02u_%03u.dat",day,month,year,hour,minute,second,num); //set another file name
      }
      

     // printf("\"%s\"\r\n",outputFileName);
     
      
      FRESULT res;
      if( getFreeSpace()>4096 ) 
      {
        res = f_open(&outputFile, outputFileName, FA_OPEN_ALWAYS | FA_WRITE); //open file
        //printf("open file proc\r\n");
        //UINT bw;
        //res = f_write(&outputFile, "<block>", 7, &bw);
        return res;
      }
      else return FR_NOT_ENABLED;
      }
      else return FR_NOT_READY; 
      
    }
    
    char* getActiveBuffer()
    {
      if(activeBuffer==1) return buffer2;
      else return buffer1;
    }
    char* getPassiveBuffer()
    {
      if(activeBuffer==1) return buffer1;
      else return buffer2;
    }
    
    void closeFile() //file close proc @ power off
    {
      //printf("close file proc\r\n");
        if(initOk/*&&(!common.commandMode)*/) 
        {
          UINT bw;
          FRESULT res;
          
          //res = f_write(&outputFile, "<block>", 7, &bw);
          
          res = f_write(&outputFile, getActiveBuffer(), bufferHead, &bw);
          
          res = f_sync(&outputFile); //sync card
          if(res!=FR_OK) //write control
          {
            char msg[32];
            sprintf(msg,"\r\n***write error***\r\n");
            f_write(&outputFile,msg,strlen(msg),&bw);
            f_sync(&outputFile);   
          }  
          f_close(&outputFile);
        }
    }
    
    void writeStringToBuffer(char *str) //write message do active buffer
    {
      if(initOk)
      {
        //while(isWriting);
        if(isWriting) printf("wr err\r\n");
        isWriting=true;
        //xSemaphoreTake(sdioMutex, portMAX_DELAY);
        for(int i=0;i<strlen(str);i++)
        {
          getActiveBuffer()[bufferHead]=str[i];
          bufferHead = (bufferHead + 1)%OUTPUTBUFFERLENGTH;
          if(bufferHead==0) swapBuffers(); //swap filled buffer
        }
        /*
        char tmp[32];
        strcount++;
        sprintf(tmp,"%u \"%s\" ",strcount,str);
        
        for(int i=0;i<strlen(tmp);i++)
        {
          //outputBuffer[activeBuffer][bufferHead]=tmp[i];
          getActiveBuffer()[bufferHead]=tmp[i];
          bufferHead = (bufferHead + 1)%OUTPUTBUFFERLENGTH;
          if(bufferHead==0) swapBuffers(); //swap filled buffer
        }
        */
        isWriting=false;
        //xSemaphoreGive(sdioMutex);
      }
    }
    
    void swapBuffers() //swap active buffer
    {
      if(initOk)
      {
        if(activeBuffer==1) activeBuffer=0;
        else activeBuffer = 1;
        xSemaphoreGive(sdioOutputSemaphore); //take semaphore to write task    
      }  
    }
    
    void writeToFile()
    {
      watchdog.reload();
      if(initOk&&(getFreeSpace()>4096))
      {
        UINT bw;
        FRESULT res;
        
        
        /*
        for(int i = 0;i<32;i++)
        {
          printf("%c",getPassiveBuffer()[i]);
        }
        printf("---\r\n---");
        */

        //res = f_write(&outputFile, "<block>", 7, &bw);
        
        //for(int i=0;i<1000;i++) res = f_write(&outputFile, "<test>\r\n", 8, &bw);
          
        //res = f_write(&outputFile, "_", 1, &bw);
        
        res = f_write(&outputFile, getPassiveBuffer(), OUTPUTBUFFERLENGTH, &bw);
        
        res = f_sync(&outputFile); //sync card
        if(res!=FR_OK) //write control
        {
          char msg[32];
          sprintf(msg,"\r\n***write error***\r\n");
          f_write(&outputFile,msg,strlen(msg),&bw);
          f_sync(&outputFile);   
        }
        //printf("%d\r\n",res);
        //printf("writed %d bytes\r\n",bw);
      }
    }
    
  void removeFile(uint16_t num)
  {
    if(initOk)
      {
     FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, "0:/");                       /* Open the directory */
    if (res == FR_OK) {
      
     if(num==0)
     {
       for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
      f_unlink (fn);
     }
     }
    else
    { 
      
        for (int i=0;i<num;i++) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
             
        }
            /* It is a file. */
            //printf("\"%s\"\r\n", fn);
            watchdog.reload();
            f_unlink (fn);
    }
    }
      } 
  }
  
  
     
    
  //private zone---------------------------------------------------------------
  private:
    
    char outputFileName[32];
};

extern "C" void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
extern "C" void SD_SDIO_DMA_IRQHANDLER(void)
{
  /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
  SD_ProcessDMAIRQ();
}