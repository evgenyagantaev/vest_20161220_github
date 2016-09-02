# define STATUSSTRINGLENGTH 128

// This task performs diagnostics
void diagnosticsTask(void *parameters)
{
   
   char statusString[STATUSSTRINGLENGTH];
   
   // debug ***
   char timestamp[32];
   // debug ***
   
   FIL logFile;
   UINT bytesWritten;
   
   static int local_call_stack_pointer;
   char task_id = 'c';
   while(1)
   {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
        
      if(common.heartRate != 444)
      {
         diagnost.makeDiagnosis(common.heartRate, 
            movementDetector.getWalkingStatus(), movementDetector.getRunningStatus(),
            movementDetector.getPosition());
         
         if(common.heartRate <= 85)
            common.heartRateWeight = 1;
         else
            common.heartRateWeight = (int)( 1.0 + 0.044*((double)(common.heartRate) - 85.0));

         // подсчитываем интегральную чсс
         long dI = (long)((double)common.heartRate * common.heartRateWeight);
         common.heartRateIntegral += dI;
      }
      
      int motion = movementDetector.getPosition();
      int battery = common.batteryCharge;
      
      // send status string via uart
      
      if(adcFault)
      {
         // no correct data from ADC
         sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                 777, motion, battery);
      }
      else if(adcAbnormal)
      {
         adcAbnormal = 0;
         // no correct data from ADC
         sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                 555, motion, battery);
      }
      else if(diagnost.badAdcRange)
      {
         
         //*
         if(diagnost.getBadAdcRangeFlag() == 0) // 444 appear right now
         {
            diagnost.setBadAdcRangeFlag();
            diagnost.setBadAdcRangeMarker(common.secondsTimer);
         
            sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", diagnost.getStatus(), 
            diagnost.getLastDisplayedPulse(), movementDetector.getPosition(), 
            common.batteryCharge);
         }
         else // 444 appear in some moment in the past
         {
            if((common.secondsTimer - diagnost.getBadAdcRangeMarker()) > 10 ) // 444 more then 10 seconds
            {
               // adc input is out of range
               sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                       444, motion, battery);
               common.heartRate = 444;
               
               //sprintf(debugstatus1, "Iso = %d\r\n", diagnost.badIsoline);
               //xSemaphoreTake(uartMutex, portMAX_DELAY);
               //uart.sendMessage(debugstatus1);
               //xSemaphoreGive(uartMutex);
               //vTaskDelay(delay10);
            }
            else // 444 less then 10 seconds
            {
               sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", diagnost.getStatus(), 
               diagnost.getLastDisplayedPulse(), movementDetector.getPosition(), 
               common.batteryCharge);
            }
         }
         //*/
         
         
      }
      else if(ecgAnalizer.getHeartRate() > 205)
      {
         // high noice in data
         if(diagnost.getLastDisplayedPulse() == 444) // no normal pulse yet
         {
            sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                    444, motion, battery);
         }
         else
         {
            sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                    333, motion, battery);
         }
      }
      else if(isolineController.currentIsoline == 32768)    // no patient
      {
          sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
                    444, motion, battery);
      }
      else
      {
         // everything ok
         diagnost.resetBadAdcRangeFlag();
         
         if(common.heartRate != 444)
         {
             
            //////////////////////////////////////////////////////////////////////////
            // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
            //make_hard_fault();
            // DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
             
             
             
            diagnost.setLastDisplayedPulse(common.heartRate);
#ifndef WIDEDIAGNOSIS
            // print classic form of status string
            sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", diagnost.getStatus(), 
               common.heartRate, movementDetector.getPosition(), 
               common.batteryCharge);
#else
            // print form of status string with Bayevsky tension index and hr integral
            common.movement = movementDetector.getPosition();
            sprintf(statusString, "c%dp%03dm%dv%03db%03di%08ldG\r\n", diagnost.getStatus(), 
               common.heartRate, movementDetector.getPosition(), 
               common.batteryCharge, (int)(rrHistogramm.getTensionIndex()), common.heartRateIntegral);
            
            
#endif
         }
         else
         {
            sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", 0, 
               444, movementDetector.getPosition(), 
               common.batteryCharge);
         }
         //sprintf(statusString, "c%dp%03dm%dv%03db%dG\r\n", diagnost.getStatus(), 
            //common.heartRate, movementDetector.getPosition(), 
            //common.batteryCharge, (int)(rrHistogramm.getTensionIndex()));
         
         // debug recreation curve
         //sprintf(statusString, "c%dp%03dm%dv%03dG\r\n", diagnost.getStatus(), 
            //ecgAnalizer.getHeartRate(), movementDetector.getPosition(), 
            //diagnost.getTachiThreshold());
      }
      
      //output diagnosys
      long interval = diagnost.getInterval();
      if((common.secondsTimer%interval) == 0)
      {
          // clip timestamp to status
          //strncpy(timestamp, rtc.getTimeStamp(), 32);
          //int statusLen = strlen(statusString);
          //strncpy(statusString + statusLen - 2, "   ", 3);
          //strncpy(statusString + statusLen + 1, timestamp, 32);
          
          
         xSemaphoreTake(sdioMutex, portMAX_DELAY);
         if(!common.commandMode) sdio.writeStringToBuffer(statusString);
         xSemaphoreGive(sdioMutex);
           
         if(common.sendStatus)
         {
             
             //strcat(statusString, timestamp);
             // debug RTC
             /*
             if(common.lse_start_ok)  // lse startoval ok
                strcat(statusString, "LSE OK\r\n");
             else
                strcat(statusString, "LSE BAD\r\n"); 
             
             if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)  // current lse ok
                strcat(statusString, "Current LSE OK\r\n");
             else
                strcat(statusString, "Current LSE BAD\r\n"); 
             
             if(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != RESET)  // current lsi ok
                strcat(statusString, "Current LSI OK\r\n");
             else
                strcat(statusString, "Current LSI BAD\r\n"); 
             //*/
             
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            uart.sendMessage(statusString);
            //debug ***
            //sprintf(debugstatus1, "^^^^^^^^^%ld^^^^%ld\r\n", common.secondsTimer, interval);
            //uart.sendMessage(debugstatus1);
            //debug ***
            //debug ***
            /*
            rrvectorString[0] = 0;
            for(int i=128; i >= 1; i--)
            {
               sprintf(rrString, "%d ", bayevsky.getRRbyIndex(600 - i));
               strcat(rrvectorString, rrString);
            }
            sprintf(rrString, "\r\n");
            strcat(rrvectorString, rrString);
            
            uart.sendMessage(rrvectorString);
            //*/
            //debug ***
            xSemaphoreGive(uartMutex);
            //printf("%s", statusString);
         }
      }
      //debug************
      /*
      else
      {
         sprintf(debugstatus2, "**********%ld****%ld\r\n", common.secondsTimer, interval);
         xSemaphoreTake(uartMutex, portMAX_DELAY);
         uart.sendMessage(debugstatus2);
         xSemaphoreGive(uartMutex);
      }
      //*/
      
      //sleep for a known interval
      //vTaskDelay(diagnost.getInterval() * configTICK_RATE_HZ);
      vTaskDelay(delay1000);
      
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
   }
}