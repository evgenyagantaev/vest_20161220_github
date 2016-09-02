//this task provides analysis of received from uart input command

void uartInputTask(void *pvParameters)
{
   
   uart.initUartRecieve();
   
  
  
    static int local_call_stack_pointer;
    char task_id = 'k';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
      
        xSemaphoreTake(uartInputSemaphore, portMAX_DELAY);
        //printf("\"%32s\"\r\n",common.uartRecieveBuffer);
        /*
        for(int i=0;i<32;i++)
        {
          printf("%c",common.uartRecieveBuffer[i]);
        }
        printf("\r\n");*/
        vTaskDelay(delay200);

        if(common.commandMode)
        {
            char cmd[10][9]={"bypass\0","list\0","time\0","date\0","rem\0","file\0","fault\0","format\0","iap\0","tune\0"}; //command name

            for(int ci=0;ci<10;ci++)
            {
                int val=0;
                if(uart.processCommand(cmd[ci],&val))
                {
                    //printf("%d\r\n",ci);
                    switch(ci)
                    {
                    case 0: //bypass
                    {
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
                        //printf("bypass\r\n");
                        break;
                    }
                    case 1: //list
                    { 
                        //printf("list ci %d\r\n",ci);

                        xSemaphoreTake(uartMutex, portMAX_DELAY);
                        uart.sendMessage("File list\r\nnum/name/size(bytes)\r\n\0");
                        xSemaphoreGive(uartMutex);

                        uart.list();

                        break;
                    }
                    case 2: //time
                    {
                        if(val==0) 
                        {

                            xSemaphoreTake(uartMutex, portMAX_DELAY);
                            uart.sendMessage(rtc.getTimeStamp());
                            xSemaphoreGive(uartMutex);
                            //printf("time %s\r\n",rtc.getTimeStamp());

                        }
                        else 
                        {
                            if(rtc.setTime((uint8_t)(val/10000%100),(uint8_t)(val/100%100),(uint8_t)(val%100))!=SUCCESS)
                            {
                                xSemaphoreTake(uartMutex, portMAX_DELAY);
                                uart.sendMessage("Time set error\0");
                                xSemaphoreGive(uartMutex);
                            }
                        }
                        break;
                    }
                    case 3: //date
                    {
                        if(val==0) 
                        {

                            xSemaphoreTake(uartMutex, portMAX_DELAY);
                            uart.sendMessage(rtc.getTimeStamp());
                            xSemaphoreGive(uartMutex);
                            //printf("date %s\r\n",rtc.getTimeStamp());

                        }
                        else 
                        {
                            if(rtc.setDate((uint8_t)(val/10000%100),(uint8_t)(val/100%100),(uint8_t)(val%100))!=SUCCESS)
                            {
                                xSemaphoreTake(uartMutex, portMAX_DELAY);
                                uart.sendMessage("Date set error\0");
                                xSemaphoreGive(uartMutex);
                            }
                        }
                        break;
                    }
                    case 4: //rem
                    {
                        //printf("rem ci %d\r\n",ci);

                        sdio.removeFile((uint16_t)val);

                        xSemaphoreTake(uartMutex, portMAX_DELAY);
                        uart.sendMessage("New file list\r\nnum/name/size(bytes)\r\n\0");
                        xSemaphoreGive(uartMutex);

                        uart.list();

                        break;
                    }
                    case 5: //file
                    {
                        //printf("file ci %d\r\n",ci);

                        uart.sendFile((uint16_t)val);

                        break;
                    }
                    case 6: //fault
                    {
                        //printf("file ci %d\r\n",ci);

                        for(;;);

                        break;
                    }
                    case 7: //format
                    {
                        /*    FRESULT f_mkfs (
                        BYTE  Drive,            // Logical drive number 
                        BYTE  PartitioningRule, // Partitioning rule 
                        UINT  AllocSize         // Size of the allocation unit 
                        );*/
                        xSemaphoreTake(uartMutex, portMAX_DELAY);
                        uart.sendMessage("Formatting SD card...\r\n\0");
                        xSemaphoreGive(uartMutex);

                        vTaskDelay(delay10*5);

                        f_mkfs((TCHAR*)"0:\0",0,4096);

                        xSemaphoreTake(uartMutex, portMAX_DELAY);
                        uart.sendMessage("New file list\r\nnum/name/size(bytes)\r\n\0");
                        xSemaphoreGive(uartMutex);

                        uart.list();

                        break;
                    }
                    case 8: //iap
                    {
                      
                        flash.writeIap();

                        //NVIC_SystemReset();

                        break;
                    }
                    case 9: //tune (individualnaya nastroika krivoi vosstanovleniya)
                    {
                        // turn on tuning mode
                        common.tuning_mode = true;
                        // start tuning mode timer
                        common.timer_tuning_mode = xTaskGetTickCount();

                        xSemaphoreTake(uartMutex, portMAX_DELAY);
                        uart.sendMessage("Ready for individual tuning\r\n");
                        xSemaphoreGive(uartMutex);

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
                        //printf("bypass\r\n");
                        break;
                    }
                    };
                }//end if(uart.processCommand(cmd[ci],&val))
            }// end for(int ci=0;ci<10;ci++)
        }// end if(common.commandMode)
        else
        {
            char cmd[3][10]={"interval\0","command\0","test\0"}; //command name

            for(int ci=0;ci<3;ci++)
            {
                int val=0;
                if(uart.processCommand(cmd[ci],&val))
                {
                    switch(ci)
                    {
                    case 0:
                    { 
                       if(val<1) val=1;
                       if(val>30) val=30; 
                       diagnost.setInterval(val);
                       //printf("interval %d\r\n",diagnost.getInterval());
                       break;
                    }
                    case 1:
                    { 
                      common.sendSamples = false;
                      common.sendStatus = false;
                      common.sendMarkers = false;
                      common.sendAccel = false;
                      
                      common.commandMode = true;
                      sdio.closeFile();
                      
                      xSemaphoreTake(uartMutex, portMAX_DELAY);
                      uart.sendMessage("Command mode\r\n\0");
                      xSemaphoreGive(uartMutex);
                      
                      //printf("command %d\r\n",val);
                     break;
                    }
                    case 2:
                    { 
                       //testFlag = 1;
                       common.sendSamples = true;
                       common.sendMarkers = true;
                       break;
                    }
                    };

                }//end if(uart.processCommand(cmd[ci],&val))
            }// end for(int ci=0;ci<3;ci++)
        }// end else
        
        taskYIELD();

        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
    }
}