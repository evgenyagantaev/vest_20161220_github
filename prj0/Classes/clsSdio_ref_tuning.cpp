//this class provides SDIO interface, refactoring

#include "stm32_sdio_common.h"
#include "stm32_sdio_sd.h"

class clsSdio_ref_tuning
{
  
public:
  
  //var------------------------------------------------------------------------
    bool initOk; //correct init variable
    bool fileIsOpen; //file is open variable
    bool isWriting;  //write in process
    
    bool alarmMode;
    
    uint32_t alarmStringCount;
    
    FATFS fatfs; //fatfs handler
    FIL outputFile; //file handler
    FIL logFile, logFileTmp;
    
    // ************ TUNING ************
    FIL tuningFile; // tuning file handler
    
    uint8_t activeBuffer; //active buffer index
    uint16_t bufferHead; //buffer head index
    
    static const uint16_t maxFiles = 200;
    static const DWORD requiredFreeSpace = 102400/*7804400*/; //ruquired free space for i/o operations const in kbytes, 100MiB
    static const int OUTPUTBUFFERLENGTH = 500; //buffer length const, must be <512
    static const uint32_t fileStringsLimit = 500000; //limit of strings in file def 500000
    //static const uint32_t fileStringsLimit = 80000; 
    
    static const uint32_t preAlarmStringCount = 57000; //5 min
    static const uint32_t postAlarmStringCount = 57000; //5 min
    
    uint32_t fileLimitCount;
    
    char buffer1[OUTPUTBUFFERLENGTH]; //buffer 1
    char buffer2[OUTPUTBUFFERLENGTH]; //buffer 2
    // ************ TUNING ************
    char tuningBuffer[128]; // tuning buffer 
    
    
    char outputFileName[32], lastOpenedFile[32];
    char any_purpose_string32[32];
    // ************ TUNING ************
    char tuningFileName[32];
    

    
    clsSdio_ref_tuning() //class init proc
    {
      initOk = false;
      fileIsOpen = false;
      isWriting = false;
      alarmMode = false;
      alarmStringCount=0;
      fileLimitCount = 0;
      resetBuffers();
      
      // ************ TUNING ************
      strcpy(tuningFileName, "tuning.txt");
    }
    
  //proc-----------------------------------------------------------------------

    uint8_t initSdCard() //sd card init proc
    {
      NVIC_Configuration(); //config interrupts
      /*---------------------------- SD Init ------------------------------- */
      SD_Error sderr = SD_Init();
      if(sderr!=SD_OK) //init sd card
      {
        //printf("%d\r\n",sderr);
        return 0;
      }
      else 
      {    
        FRESULT res = f_mount(&fatfs,(TCHAR*)"0:\0", 1); //mount fs
        //res = f_mkfs("", 0, 0);
         //printf("%u\r\n",res);
        if(res != FR_OK) return 0;
        initOk = true;
        sdInitOk = 1;
        res = f_open(&outputFile, "tmp012345678.tmp\0", FA_CREATE_ALWAYS | FA_WRITE);
        closeFile();
        
        openFile(); //open main output file
      
        return 1;
      }
    }
    
    /*
    FRESULT appendToLog(char *fn)
    {
        UINT bw;
        FRESULT res;
        
        //printf("%32s",fn);
        
        char tmpstr[40];
        strcpy(tmpstr,fn);
        strcat(tmpstr,"\r\n");
        
        res = f_open(&logFile, "filelist.log", FA_OPEN_ALWAYS | FA_WRITE);
        res = f_lseek(&logFile, f_size(&logFile));
        res = f_write(&logFile, tmpstr, strlen(tmpstr), &bw);
        if(res!=FR_OK) return FR_DENIED;
        res = f_sync(&logFile);
        res = f_close(&logFile);
        return res;
    }
    
   */
    
    FRESULT resetFile()
    {
      if(initOk && fileIsOpen) 
        {
          closeFile();
          openFile();
          //printf("reset file\r\n");
        }
      return FR_OK;
    }
    
    char* createTimestamp()
    {
        //prepare timestamp
        set_fattime(rtc.getFatDateTime());
        uint32_t ftime = (uint32_t)get_fattime(); //get timestamp

        uint16_t day,month,year,hour,minute,second;

        year = (ftime>>25)+1980;
        month = (ftime>>21)&0xF;
        day = (ftime>>16)&0x1F;

        hour = (ftime>>11)&0x1F;
        minute = (ftime>>5)&0x3F;
        second = ((ftime)&0xF)*2;

        sprintf(any_purpose_string32,"%04u.%02u.%02u_%02u:%02u:%02u", year, month, day, hour, minute, second); // set timestamp
        
        return any_purpose_string32;
    }
    
    FRESULT writeTuning(char *buffer, int length)
    {
        FRESULT res;
        UINT bytesWritten;
        
        res = f_open(&tuningFile, tuningFileName, FA_CREATE_ALWAYS | FA_WRITE);
        res = f_write(&tuningFile, buffer, length, &bytesWritten);
        res = f_close(&tuningFile);
        
        return res;
    }
    
    FRESULT openAppendTuning()
    {
        FRESULT res;
        
        res = f_open(&tuningFile, tuningFileName, FA_OPEN_ALWAYS | FA_WRITE);
        res = f_lseek(&tuningFile, f_size(&tuningFile));
        
        return res;
    }
    
    FRESULT openCreateTuning()
    {
        FRESULT res;
        
        res = f_open(&tuningFile, tuningFileName, FA_CREATE_ALWAYS | FA_WRITE);
        
        return res;
    }

    FRESULT closeTuning()
    {
        FRESULT res;
        
        res = f_close(&tuningFile);
        
        return res;
    }

    FRESULT appendTuning(char *buffer, int length)
    {
        FRESULT res;
        UINT bytesWritten;
        
        res = f_write(&tuningFile, buffer, length, &bytesWritten);
        
        return res;
    }
    
    FRESULT closeFile() //file close proc
    {
        if(initOk && fileIsOpen) 
        {
          UINT bw;
          FRESULT res;
     
          res = f_write(&outputFile, getActiveBuffer(), bufferHead, &bw);          
          res = f_sync(&outputFile); //sync card
          
          if(res != FR_OK) //write control
          {
            char msg[32];
            sprintf(msg,"\r\n***write error***\r\n");
            res = f_write(&outputFile,msg,strlen(msg),&bw);
            res = f_sync(&outputFile);   
          }
          res = f_close(&outputFile);
          if (res == FR_OK)
          {
            fileIsOpen = false;
            return res;
          }
          else return res;
        }
        else return FR_OK;
    }
   
    FRESULT openFile() //main file open proc
    {
      FRESULT res;
      fileIsOpen = false;
      if(initOk){
      res = closeFile();
      
      fileLimitCount = 0;
      //f_chdir("0:");
 
      if(res == FR_OK)
      {
        resetBuffers();
        res = prepareFreeSpace();
        if(res == FR_OK)
        {
        
          //prepare unique file name /w timestamp
          set_fattime(rtc.getFatDateTime());
          uint32_t ftime = (uint32_t)get_fattime(); //get timestamp
      
          uint16_t day,month,year,hour,minute,second;
      
          year = (ftime>>25)+1980;
          month = (ftime>>21)&0xF;
          day = (ftime>>16)&0x1F;
      
          hour = (ftime>>11)&0x1F;
          minute = (ftime>>5)&0x3F;
          second = ((ftime)&0xF)*2;
      
          uint16_t num=0;
          
          sprintf(outputFileName,"%02u%02u%04u_%02u%02u%02u_%03u.dat\0",day,month,year,hour,minute,second,num); //set default file name
     /*
          FILINFO FileInfo;
#if _USE_LFN
          static char lfn[_MAX_LFN + 1];
          FileInfo.lfname = lfn;
          FileInfo.lfsize = sizeof(lfn);
#endif
 
          while(f_stat(outputFileName,&FileInfo)!=FR_NO_FILE) //check if file is exist and set new name
          {
            sprintf(outputFileName,"%02u%02u%04u_%02u%02u%02u_%03u.dat\0",day,month,year,hour,minute,second,num); //set another file name
            printf(outputFileName);
            //if(num>maxFiles) removeFirstFile();
            num++;
          }
      */
          res = f_open(&outputFile, (TCHAR const*)outputFileName, FA_CREATE_ALWAYS | FA_WRITE); //open file
          if (res == FR_OK)
          {
            
            
            UINT bw;
            res = f_write(&outputFile, outputFileName, strlen(outputFileName), &bw);
            res = f_write(&outputFile, " "DEVICENAME"\r\n", 3+strlen(DEVICENAME), &bw);
            
            //res = appendToLog(outputFileName);
            if(res==FR_OK)
            {
              fileIsOpen = true;
              strcpy(lastOpenedFile,outputFileName);
              //printf("lof %32s\r\n",lastOpenedFile);
              //printf("fn %32s\r\n",outputFileName);
            return res;
            }
            else return FR_DENIED;
          }
          else
          {
            return FR_DENIED;
          }
        }
        else return FR_DENIED;
      }
      else return FR_DENIED;
      }
      return FR_DENIED;
    }
    
    void alarm(DWORD t)
    {
      //printf("lof %32s\r\n",lastOpenedFile);
      //resetFile();
      if(initOk){
      closeFile();
      
      FRESULT res;
      TCHAR tmpbuf[256];
      uint32_t strcnt=0;
      FIL preAlarmFile, lastFile;
      watchdog.reload();
      res = f_open(&lastFile, (TCHAR const*)lastOpenedFile, FA_OPEN_EXISTING | FA_READ); //open file
      
          
      
      if(res==FR_OK)
      {
        while(!f_eof(&lastFile))
        {
          if(f_gets (tmpbuf,32,&lastFile)) strcnt++;
          watchdog.reload();
        }
        
        //printf("strcnt %u\r\n",strcnt);
      }
      
      f_lseek(&lastFile,0);
      
      UINT bb;
      
      if(strcnt>(preAlarmStringCount+postAlarmStringCount))
      {
        for(uint32_t i=0;i<(strcnt-(preAlarmStringCount+postAlarmStringCount));i++) 
        {
          f_gets (tmpbuf,32,&lastFile);
          watchdog.reload();
        }
      }
      
      
    uint32_t ftime = (uint32_t)t; //get timestamp
    
    uint16_t day,month,year,hour,minute,second;
    
    year = (ftime>>25)+1980;
    month = (ftime>>21)&0xF;
    day = (ftime>>16)&0x1F;
    
    hour = (ftime>>11)&0x1F;
    minute = (ftime>>5)&0x3F;
    second = ((ftime)&0xF)*2;
   
    sprintf(outputFileName,"alrm%02u%02u%04u_%02u%02u%02u.dat\0",day,month,year,hour,minute,second); //set default file name
      
      
      res = f_open(&preAlarmFile, /*"prealarm.dat\0"*/outputFileName, FA_CREATE_ALWAYS | FA_WRITE);
      
      
            res = f_write(&preAlarmFile, outputFileName, strlen(outputFileName), &bb);
            res = f_write(&preAlarmFile, "\r\n", 2, &bb);
      
      while(!f_eof(&lastFile))
      {
        f_read(&lastFile,tmpbuf,256,&bb);
        f_write(&preAlarmFile,tmpbuf,bb,&bb);
        watchdog.reload();
      }
      
      f_close(&preAlarmFile);
      f_close(&lastFile); 
   
      openFile();   
      }
    }
    
    void resetBuffers() //reset buffers proc
    {
      activeBuffer=0;
      bufferHead=0;
    }
    
    char* getActiveBuffer() //active buffer return proc
    {
      if(activeBuffer==1) return buffer2;
      else return buffer1;
    }
    
    char* getPassiveBuffer() //passive buffer return proc
    {
      if(activeBuffer==1) return buffer1;
      else return buffer2;
    }
    
    DWORD getFreeSpace() //return free space in kbytes proc
    {
      /*FATFS *fs;
      DWORD fre_clust;
       //BYTE zz;
      FRESULT res;
      //zz=&fatfs->csize;
      fs=&fatfs;
      res = f_getfree("0:", &fre_clust, &fs);
      printf("getfree space cs %u fc %u\r\n",fs->csize,fre_clust);
      if(res==FR_OK) return fre_clust*fs->csize/2;
      else return FR_DENIED;*/
      
      if(initOk){
      FATFS *fs;
      DWORD fre_clust, fre_sect/*, tot_sect*/;
      FRESULT res;

      // Get volume information and free clusters of drive 0
      res = f_getfree((TCHAR const*)"0:", &fre_clust, &fs);
      //printf("%u\r\n",res);
      if (res==FR_OK)
      {

        // Get free sectors 
        //printf("fc %lu fs %lu cs %lu av %lu\r\n",fre_clust,fs->fsize,fs->csize,
        //       fre_clust * fs->csize/2);
        
        fre_sect = fre_clust * fs->csize;

        // Print free space in unit of KB (assuming 512 bytes/sector) 
        //printf("%lu KB available.\n", fre_sect / 2);
        return fre_sect/2;
      }
      else return 0;
      }
      return 0;
    }
    
    FRESULT prepareFreeSpace() //delete files if free space is not available
    {
      if(initOk){
      DWORD free = getFreeSpace();
      //printf("free %lu\n", free);
      while(free < requiredFreeSpace) 
      {
        //if(removeFirstFile()!=FR_OK) return FR_DENIED;
        //removeFile(0);
        removeFirstFile();
        free = getFreeSpace();
      }
      }
      return FR_OK;
    }
    
    FRESULT removeFirstFile()
    {
      
      if(initOk)
      {
        closeFile();
   
    
        FRESULT res;
        FILINFO fno;
        DIR dir;
        //TCHAR *fn;
#if _USE_LFN
    static TCHAR lfn[(_MAX_LFN + 1)*2];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif
    bool fileSearched = false;

    TCHAR fn[(_MAX_LFN + 1)*2];
        //uint16_t count = 1, rmval=1;
         
    res = f_opendir(&dir, "/\0");                       /* Open the directory */
                       
        DWORD tmpdt = 0xFFFFFFFF;
        
        if (res == FR_OK) 
        {

          for (;;) 
          {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            //printf("\"%s\" %lu\r\n\0",fno.lfname,fno.fsize);
            if (res != FR_OK || ((fno.lfname[0] == 0))&&(fno.fname[0] == 0)) break;  /* Break on error or end of dir */
            if (fno.lfname[0] == '.') continue;             /* Ignore dot entry */
            
            
            
              fileSearched = true;
            
            if((((DWORD)fno.fdate<<16)+fno.ftime)<tmpdt) 
            {
            /*#if _USE_LFN
              fn = *fno.lfname ? fno.lfname : fno.fname;
            #else
              fn = fno.fname;
            #endif
              */
              if(fno.lfname[0] != 0)
              {
              for(uint16_t i=0;i<((_MAX_LFN + 1)*2);i++)
              {fn[i]=fno.lfname[i];}
              }
              else
              {for(uint16_t i=0;i<13;i++)
              {fn[i]=fno.fname[i];}}
              //rmval = count;
              tmpdt = ((DWORD)fno.fdate<<16)+fno.ftime;
            }
            
            //count++;
            
          }
     
       }
       res = f_closedir(&dir);
       if(fileSearched)
       {
       //printf("rem %s\r\n",fn);
       watchdog.reload();
       f_chmod(fn, 0, AM_RDO);
       res = f_unlink (fn);
       //getFreeSpace();
       //removeFile(rmval);
       }
       return res; 
     
 
      }   
    }
    
    void writeStringToBuffer(char *str) //write message to active buffer
    {
      if(initOk)
      {
        //if(isWriting) printf("wr err\r\n");
        //isWriting=true;
        //xSemaphoreTake(sdioMutex, portMAX_DELAY);
        for(int i=0;i<strlen(str);i++)
        {
          getActiveBuffer()[bufferHead]=str[i];
          bufferHead = (bufferHead + 1)%OUTPUTBUFFERLENGTH;
          if(bufferHead==0) swapBuffers(); //swap filled buffer
        }
#ifdef SDIOFILELIMIT
        fileLimitCount++;
        if(fileLimitCount>=fileStringsLimit) resetFile();
#endif
        if(alarmMode) alarmStringCount++;
        //isWriting=false;
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
      if(initOk)
      {
        if(fileIsOpen&&(prepareFreeSpace()==FR_OK))
      {
        UINT bw;
        FRESULT res;
        
        
       /* for(int i = 0;i<32;i++)
        {
          printf("%c",getPassiveBuffer()[i]);
        }
        printf("---\r\n---");
        */
        
        res = f_write(&outputFile, getPassiveBuffer(), OUTPUTBUFFERLENGTH, &bw);
        
        //printf("res %u\r\n",res);
        
        res = f_sync(&outputFile); //sync card
        
        //printf("res %u\r\n",res);
        
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
      else if(!fileIsOpen) openFile();
      }
    }
    
    void removeFile(uint16_t num)
    {
      if(initOk)
      {
        closeFile();
        
        FRESULT res;
        FILINFO fno;
        DIR dir;
        TCHAR *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
        static TCHAR lfn[_MAX_LFN + 1];
        fno.lfname = lfn;
        fno.lfsize = sizeof lfn;
#endif

        res = f_opendir(&dir, (TCHAR const*)"0:/");                       /* Open the directory */
        if (res == FR_OK) 
        {
        if(num==0)
        {
          for (;;) 
          {
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
          for (int i=0;i<num;i++) 
          {
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
            f_chmod(fn, 0, AM_RDO);
            f_unlink (fn);
        }
       }
     } 
  }
  
private:
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