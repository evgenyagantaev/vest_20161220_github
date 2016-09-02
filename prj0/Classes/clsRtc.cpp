//this class provides RTC clock manage
class clsRtc 
{
   
private://**********************************************************************


public://***********************************************************************

   //----------------------- public constructor -------------------------------
  clsRtc() 
  {
        //*
        RTC_InitTypeDef RTC_InitStructure;
        //* Enable write access to the RTC
        //* Enable the PWR clock 
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        
        //* Allow access to RTC 
        PWR_BackupAccessCmd(ENABLE);
        //RCC_BackupResetCmd(ENABLE); //reset settings
        //* Configure the RTC clock source 
        
       
#ifdef RTCLSE
        RCC_LSICmd(DISABLE);
        // Enable the LSE OSC 
        RCC_LSEConfig(RCC_LSE_ON);

        // Wait till LSE is ready  (with 5 sec timeout) 
        profInterval0 = 0;
        profTick0 = xTaskGetTickCount();
        while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (profInterval0 <= 5))
        {
            profInterval0 = ((xTaskGetTickCount() - profTick0)*tickLength)/1000000;
        }
        
        if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)  // lse startoval ok
        {
            // Select the RTC Clock Source
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
            common.lse_start_ok = true;
        }
        else // lse ne startoval 
        {
            // Disable the LSE OSC 
            RCC_LSEConfig(RCC_LSE_OFF);
            // ispolzuem lsi v kachestve istochnika taktirovaniya rtc
            // Enable the LSI OSC 
            RCC_LSICmd(ENABLE);
             // Wait till LSI is ready  
            while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
            {
            }
                
            // Select the RTC Clock Source 
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); 
            common.lse_start_ok = false;
        }
#else
         // Enable the LSI OSC 
        RCC_LSICmd(ENABLE);
         // Wait till LSI is ready  
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
        }
            
        // Select the RTC Clock Source 
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); 
        
#endif    
        

        //* Enable the RTC Clock 
        RCC_RTCCLKCmd(ENABLE);
        //* Wait for RTC APB registers synchronisation 
        RTC_WaitForSynchro(); 
        
        // coarse calibration
        RTC_CoarseCalibConfig(RTC_CalibSign_Positive, 1); // 
        RTC_CoarseCalibCmd(ENABLE);
        
        //* Calendar Configuration with LSE supposed at 32768Hz 
        //RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
        //RTC_InitStructure.RTC_SynchPrediv	=  0xFF; //* (32KHz / 128) - 1 = 0xFF
        RTC_InitStructure.RTC_AsynchPrediv = 109;
        RTC_InitStructure.RTC_SynchPrediv	=  297; // corrected value
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        RTC_Init(&RTC_InitStructure); 
        
        
    
        //* Disable access to RTC 
        //PWR_BackupAccessCmd(DISABLE);
        
        //*/
    
  }
  
  ErrorStatus setDateTime(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t second)
  {
    ErrorStatus er;
    er = setTime(hour,minute,second);
    er = setDate(day,month,year);
    return er;
  }
  ErrorStatus setTime(uint8_t hour,uint8_t minute,uint8_t second)
  {
    ErrorStatus er;
    RTC_TimeTypeDef RTC_TimeStructure;
    /* Set the Time */
    RTC_TimeStructure.RTC_Hours = hour;
    RTC_TimeStructure.RTC_Minutes = minute;
    RTC_TimeStructure.RTC_Seconds = second;
    er = RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure); 
    return er;
  }
  ErrorStatus setDate(uint8_t day,uint8_t month,uint8_t year)
  {
    ErrorStatus er;
    RTC_DateTypeDef RTC_DateStructure;
    /* Set the Date */
    RTC_DateStructure.RTC_Month = month;
    RTC_DateStructure.RTC_Date = day; 
    RTC_DateStructure.RTC_Year = year; 
    RTC_DateStructure.RTC_WeekDay=1;
    er = RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure); 
    //printf("set date st %d\r\n",er);
    return er;
  }
  
  DWORD getFatDateTime()
  {
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;
    
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure); 
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure); 
    
   // printf("%u %u %u_%u %u %u\r\n",RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Year,
    //       RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
    
   return ((uint32_t)(2000+RTC_DateStructure.RTC_Year-1980) << 25)      
      | ((uint32_t)RTC_DateStructure.RTC_Month << 21)       
      | ((uint32_t)RTC_DateStructure.RTC_Date << 16)       
      | ((uint32_t)RTC_TimeStructure.RTC_Hours << 11)       
      | ((uint32_t)RTC_TimeStructure.RTC_Minutes << 5)       
      | ((uint32_t)RTC_TimeStructure.RTC_Seconds >> 1)       
      ;
  }
  char* getTimeStamp()
  {
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;
    
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure); 
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure); 
    
    // use global char buffer
    sprintf(anyMessage32,"%02u.%02u.%04u %02u:%02u:%02u\r\n",RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Year+2000,
           RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
    return anyMessage32;
  }
  
};