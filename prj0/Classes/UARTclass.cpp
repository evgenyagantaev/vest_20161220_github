#include "main.h"


class clsUART //UART1 class /w DMA transmit, interrupt recieve
{
   
private://**********************************************************************

   static const int OUTBUFFERLENGTH = 128;
   
   
   
   // buffer of strings which we have to out via uart
   char *outStringsBuffer[OUTBUFFERLENGTH];
   // index of current position for writing in out buffer
   int outBufferTail;
   
   char emergencyMessage[64];
   

   
public://***********************************************************************

   //----------------------- public constructor -------------------------------
  clsUART() 
  {

     /* --------------------------- System Clocks Configuration -----------------*/
     
  
     /* DMA2 clock enable */
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
     
     /* UART1 clock enable */
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
    
     
     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
     

       /*----------------------------NVIC Configuration-----------------------*/
     //NVIC_InitTypeDef NVIC_InitStructure;
     /* Configure the Priority Group to 2 bits */
     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
     /* Enable the UART1 TX DMA Interrupt */
     /*NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);*/
     
     NVIC_EnableIRQ(DMA2_Stream7_IRQn);
       
     /* Connect USART pins to AF */
     GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //usart1 tx
     GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);  //usart1 rx
     
     /*----------------------- GPIO Configuration ----------------------------*/
     GPIO_InitTypeDef GPIO_InitStructure;
     
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOB, &GPIO_InitStructure);  
     
    
     USART_InitTypeDef USART_InitStructure;
    
     /* USARTx configuration ----------------------------------------------------*/
     /* USARTx configured as follow:
           - BaudRate = 115200 baud
           - Word Length = 8 Bits
           - One Stop Bit
           - No parity
           - Hardware flow control disabled (RTS and CTS signals)
           - Receive and transmit enabled
     */
     USART_InitStructure.USART_BaudRate = 115200;
     USART_InitStructure.USART_WordLength = USART_WordLength_8b;
     USART_InitStructure.USART_StopBits = USART_StopBits_1;
     USART_InitStructure.USART_Parity = USART_Parity_No;
     USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   
     USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
     USART_Init(USART1, &USART_InitStructure);

     USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
     
     USART_Cmd(USART1, ENABLE);
     //USART1->CR1 &= (uint16_t)~((uint16_t)USART_CR1_UE);
    
     
     
     //------------------ buffer initialization --------------------------------
     
     for(int i=0; i<OUTBUFFERLENGTH; i++)
        outStringsBuffer[i] = 0;
     outBufferTail = 0;
  //   
  }// end clsUART
  
  
  // properties ******************************************
  
  int getOutBufferTail(void)
  { return outBufferTail; }
  
  // end properties **************************************
  
  
    void sendMessage(char *outData)
    {
        if(outBufferTail < OUTBUFFERLENGTH) // there is room in buffer yet
        {
            outStringsBuffer[outBufferTail++] = outData;
            //debug
            //sprintf(emergencyMessage, "tail = %d\r\n", outBufferTail);
            //transmitMessage(emergencyMessage);
        }
        else // no room in buffer (it's an emergency situation)
        {
            //outStringsBuffer[OUTBUFFERLENGTH - 1] = emergencyMessage;
            //*
            // DEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOG
            /*
            sprintf(emergencyMessage, "c---uart out strings buffer overflow!!!---G\r\n");
            transmitMessage(emergencyMessage);
            
            sprintf(emergencyMessage, "c---pointer %d---G\r\n", call_stack_pointer);
            transmitMessage(emergencyMessage);
            
            sprintf(emergencyMessage, "c---");
            transmitMessage(emergencyMessage);
            transmitMessage(call_stack);
            sprintf(emergencyMessage, "---G\r\n");
            transmitMessage(emergencyMessage);
            */
            // DEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOGDEBUGLOG
            
            // clean buffer
            for(int i=0; i<OUTBUFFERLENGTH; i++)
                outStringsBuffer[i] = 0;
            outBufferTail = 0;
            //vTaskDelay(delay1000);
            //*/
        }
    //sdio.writeStringToBuffer(outData);
    }// end sendMessage
  
     void outStringViaDma(void)
    {
     //debug
     //printf("%s", Buffer);
     
     if(outStringsBuffer[0] != 0) // there is string to transmit yet
     {
        if(uartOutputInProcess == 0) // previous transmission is finished
        {
          //if(!common.commandMode) sdio.writeStringToBuffer(outStringsBuffer[0]);
           /*------------------------DMA Configuration------------------------*/
           DMA_InitTypeDef  DMA_InitStructure;
          
           DMA_DeInit(DMA2_Stream7);  //dma2 stream 7 channel 4
          
           DMA_InitStructure.DMA_Channel = DMA_Channel_4;
           DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // Transmit mem to periph
           DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)outStringsBuffer[0];
           DMA_InitStructure.DMA_BufferSize = (uint16_t)strlen(outStringsBuffer[0]);
           DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //usart 1
           DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
           DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
           DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
           DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
           DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
           DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
           DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
           DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
           DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
           DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
          
           DMA_Init(DMA2_Stream7, &DMA_InitStructure);
          
          
           /* Enable DMA Stream Transfer Complete interrupt */
           DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
          
           /* Enable the USART Tx DMA request */
           USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
           
           /* Enable the DMA RX Stream */
           DMA_Cmd(DMA2_Stream7, ENABLE);
           
           // shift byffer of strings
           xSemaphoreTake(uartMutex, portMAX_DELAY);
           int doJob = 1;
           int i = 1;
           while(doJob)
           {
              outStringsBuffer[i-1] = outStringsBuffer[i];
              if((outStringsBuffer[i] != 0) && (i<OUTBUFFERLENGTH))
                 i++;
              else
                 doJob = 0;
           }
           xSemaphoreGive(uartMutex);
           //
           
           // set flag of transmission in progress
           uartOutputInProcess = 1;
           
           //decrement index of tail
           outBufferTail--;
        }
        
     }// end if(outStringsBuffer[0] != 0)
  
  }// end outStringViaDma
  
  void sendBytes(char *buff,uint16_t len) //send buffer to uart3 with no freertos processing via dma
  {
           while(uartOutputInProcess!=0); //wait for end transmission
           uartOutputInProcess = 1; //set flag of transmission in progress
           
           if(len==0) len = (uint16_t)strlen(buff);
    /*------------------------DMA Configuration------------------------*/
           DMA_InitTypeDef  DMA_InitStructure;
          
           DMA_DeInit(DMA2_Stream7);  //dma2 stream 7 channel 4
          
           DMA_InitStructure.DMA_Channel = DMA_Channel_4;
           DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // Transmit mem to periph
           DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&buff[0];
           DMA_InitStructure.DMA_BufferSize = len;
           DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; //usart 1
           DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
           DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
           DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
           DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
           DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
           DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
           DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
           DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
           DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
           DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
          
           DMA_Init(DMA2_Stream7, &DMA_InitStructure);
          
          
           /* Enable DMA Stream Transfer Complete interrupt */
           DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
          
           /* Enable the USART Tx DMA request */
           USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
           
           /* Enable the DMA RX Stream */
           DMA_Cmd(DMA2_Stream7, ENABLE);
    
    
  }
  
  // function transmits one byte through usart1 without dma
  void transmitByte(char data)
   {
      // Check if the previous transmission operation complete
      while(!USART_GetFlagStatus(USART1, USART_FLAG_TC))
         taskYIELD();
      // Clear the "Transmission complete" flag
      USART_ClearFlag(USART1, USART_FLAG_TC);
      // Send byte 
      USART_SendData(USART1, (uint16_t)data);
   }
  
  // function transmits asciiz message through usart1 without dma
  void transmitMessage(char *message)
   {
      int j = 0;
      while(message[j])
      {
         transmitByte(message[j]);
         j++;
      }
   }
  
  void initUartRecieve()
  {
 //    NVIC_InitTypeDef NVIC_InitStructure;
     /* Configure the Priority Group to 2 bits */
   //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
     /* Enable the UART3 RX DMA Interrupt */
 /*    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);
   */  
     USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
     NVIC_EnableIRQ(USART1_IRQn);
  }
  
  bool processCommand(char *command, int *val) //process command, get command and value var
  {
     //find command with value method
    
    int spaceIndex=common.uartRecieveBufferHead;  //index of space char
    int commandEndIndex=common.uartRecieveBufferHead; //index of command end
    const int maxValueLength=7;                    //max command value length
    bool isSpace=false;                            //flag, true if there is space char
    int valueLength=0;
    
    //printf("cmde %d\r\n",commandEndIndex);
    
    for(int i=0;i<maxValueLength;i++) //find space char index
    {
      spaceIndex--;
      if(spaceIndex<0)spaceIndex=common.INBUFFERLENGTH-1; //chack range
      valueLength++;
      if(common.uartRecieveBuffer[spaceIndex]==' ') {isSpace=true; break;}
    }
    
    if(isSpace) //find command name 
    {
      commandEndIndex=spaceIndex; //command last char index
    } 
    else
    {
      //commandEndIndex = (commandEndIndex + 1)%common.INBUFFERLENGTH; //command last char index
      //printf("cmde2 %d\r\n",commandEndIndex);
    }  
      
    bool isCommand=false;
    
  
      //find command name 
   
      for(int i=strlen(command)-1;i>-1;i--)
      {
        commandEndIndex--;
        if(commandEndIndex<0)commandEndIndex=common.INBUFFERLENGTH-1; //check range
      //  printf("urb \"%c\" cmd \"%c\" cmde %d\r\n",common.uartRecieveBuffer[commandEndIndex],
       //        command[i], commandEndIndex);
        if(command[i]==common.uartRecieveBuffer[commandEndIndex]) isCommand=true;
        else {isCommand=false; break;}
      }
    
    
    if(isSpace&&isCommand) //check space & command
    {
    
      char valbuffer[maxValueLength+1];
    
      for(int i=0;i<valueLength;i++) //fill value buffer
      {      
          spaceIndex = (spaceIndex + 1)%common.INBUFFERLENGTH;
          valbuffer[i]=common.uartRecieveBuffer[spaceIndex];
      }
    
      valbuffer[valueLength]='\0'; //end of buffer
    
      
    
      if(sscanf(valbuffer,"%d",val))  //read value
      {
        memset(&common.uartRecieveBuffer[0],'_',common.INBUFFERLENGTH);
        //printf("cmd %s ok\r\n",command);
        return true;
      }
      else return false;
    }
    else if((!isSpace)&&isCommand)
    {
      memset(&common.uartRecieveBuffer[0],'_',common.INBUFFERLENGTH);
      val=0;
      //printf("cmd %s ok\r\n",command);
      return true;
    }
    return false;
       
  }
 
  void list()
  {
    if(sdio.initOk)
      {
   FRESULT res;
    FILINFO fno;
    DIR dir;
    uint16_t i=1;
    char *fn;   // This function is assuming non-Unicode cfg. 
    char buff[32];
#if _USE_LFN
    char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, "0:/");                       // Open the directory 
    if (res == FR_OK) {
       // i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   // Read a directory item 
            if (res != FR_OK || fno.fname[0] == 0) break;  // Break on error or end of dir 
            if (fno.fname[0] == '.') continue;             // Ignore dot entry 
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
                                             // It is a file. 
            
            sprintf(buff,"%u \"%s\" %lu\r\n\0",i, fn,fno.fsize);
            //printf(buff);
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            sendMessage(buff);
            xSemaphoreGive(uartMutex);
            vTaskDelay(delay10*5);
            i++;
            
        }
    }
            sprintf(buff,"%lu kbytes free\r\n\0",sdio.getFreeSpace()),
            //printf(buff);
            xSemaphoreTake(uartMutex, portMAX_DELAY);
            sendMessage(buff);
            xSemaphoreGive(uartMutex);
      }
  }
  
  void sendFile(uint16_t num)
  {
    if(sdio.initOk)
      {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;   // This function is assuming non-Unicode cfg. 
#if _USE_LFN
    char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, "0:/");                       // Open the directory 
    if (res == FR_OK) {
       // i = strlen(path);
        for (int i=0;i<num;i++) {
            res = f_readdir(&dir, &fno);                   // Read a directory item 
            if (res != FR_OK || fno.fname[0] == 0) break;  // Break on error or end of dir 
            if (fno.fname[0] == '.') continue;             // Ignore dot entry 
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
             
        }
            // It is a file. 
           // printf("\"%s\"\r\n", fn);
        if(fn[0]!=0){
        sendBytes(fn,0);   
        
        FIL file;
        res = f_open(&file, fn, FA_READ);
        if (res == FR_OK) 
        {
          
          char sendBuffer[1024];
          UINT br;
          char tmp[32];
          sprintf(tmp,"\r\n<start file size %lu>\r\n\0",file.fsize);
          sendBytes(tmp,0);
          while(!f_eof(&file))
          {
              res = f_read(&file,sendBuffer,1024,&br);
              watchdog.reload();
              if(res==FR_OK)
              {
              //printf("send %u bytes res %d\r\n",br,res);
              xSemaphoreTake(uartMutex, portMAX_DELAY);
              sendBytes(sendBuffer,br);
              xSemaphoreGive(uartMutex);
              vTaskDelay(delay10*5); //wait for transmission
              sprintf(tmp,"<transmit block size %u>\0",br);
              xSemaphoreTake(uartMutex, portMAX_DELAY);
              sendBytes(tmp,0);
              xSemaphoreGive(uartMutex);
              }
              else
              {
                sendBytes("<read error>\r\n\0",0);
              }
          }
          sendBytes("<end file>\r\n\0",0);
          f_close(&file);
        }
        else
        {
          sendBytes("File error\r\n\0",0);
        }
        
        
   
    }
    else
    {
      sendBytes("No file\r\n\0",0);
    }
    }
      }
  }
  
};

extern "C" void DMA2_Stream7_IRQHandler(void)
{
 
 //printf("dma iq\r\n");
   // Test on DMA Stream Transfer Complete interrupt 
   if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7))
   {
      // clear global flag of transmission in progress (signal that transmission
      // is over)
      uartOutputInProcess = 0;
      // Clear DMA Stream Transfer Complete interrupt pending bit 
      DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
   }
}

extern "C" void USART1_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
  if (USART_GetITStatus(USART1, USART_IT_RXNE)==SET)
  {
    

    //char r = (char)USART_ReceiveData(USART3); //recieve byte
    
    char r= (char)(USART1->DR & (uint16_t)0x01FF);
    //printf("%c",r);
    if(/*(r==10)||*/(r==13)) //LF and CR codes, CR only
    {
      
      xSemaphoreGiveFromISR(uartInputSemaphore, &xHigherPriorityTaskWoken);
    }
    else
    {
      common.uartRecieveBufferHead = (common.uartRecieveBufferHead + 1)%common.INBUFFERLENGTH; //cyclic bufer proceed
      common.uartRecieveBuffer[common.uartRecieveBufferHead] = r;
    }
    
    //USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    
  }  
  portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  
}
