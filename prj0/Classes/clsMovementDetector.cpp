// This class provides getting data from accelerometer and detection of 
// movement parameters

class clsMovementDetector
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   static const uint16_t ACCELBUFFERLENGTH = 10;
   static const uint16_t highActivityThreshold = 300;
   static const uint16_t lowActivityThreshold = 90;
   static const uint16_t runThreshold = 200;
   static const uint16_t walkThreshold = 30;
   static const uint16_t noMovementThreshold = 3;
   static const uint16_t meanXlevel = 127;
   
   static const long stepInterval = 80;	        // �������� ����� ������ (���������� � ���������� ����� �������� ���: 4 �����������)
   static const long runStepInterval = 60;	// �������� ����� ������ ��� ���� (���������� � ���������� ����� �������� ���: 4 �����������)
   static const long noLocomotionInterval = 3;  // �������� (�������) ���������� �����������, ����� �������� ������������ ����� ���� � ���� !!!!!!!!!!!!
   
   static const uint16_t numberOfStepsToDetect = 3;
   //*********************** end constants *************************************   
   
   
   //************************* variables ***************************************
   
   char sampleTxtBuffer[24];
   
   // buffers for averaging of accelerometer values
   int averagingXbuffer[ACCELBUFFERLENGTH];	
   int averagingYbuffer[ACCELBUFFERLENGTH];	
   int averagingZbuffer[ACCELBUFFERLENGTH];
   
   int bufferIndex;
   
   int xIntegral;
   int yIntegral;
   int zIntegral;
   
   int meanX;
   int meanY;
   int meanZ;
   int maxX;
   int maxY;
   int maxZ;
   int minX;
   int minY;
   int minZ;
   int spanX;
   int spanY;
   int spanZ;
   
   int highMovementDetected;
   int lowMovementDetected;
   
   int position, lastPosition;
   
   int newX;
   
   int walkStepCounter;		// ������� ����� ��� ����
   int runStepCounter;		// ������� ����� ��� ����
   long lastStepTimerMarker;	// ��������� ������	���������� ������������������� ���� (������������ ��� ���������� ���������� ���������� ����� ������)
   long lastRunStepTimerMarker;	// ��������� ������	���������� ������������������� ���� (������������ ��� ���������� ���������� ���������� ����� ������)
   long noLocomotionMarker;	// ��������� ������ ���������� �����������

   int walkStepDetected;
   int runStepDetected;
   // �����
   int runningDetected;
   int walkingDetected;
   int noLocomotionDetected;
   int detectWalking;
   
   clsUART *uart;

   //*********************** end variables ************************************
   
   //********************* private functions **********************************
   
   // method returns a max value of integer array
   int maxArray(int* intArray, int arrayLength)
   {
           int maxValue = intArray[0];		
           int i;

           for(i=1; i<arrayLength; i++)
           {
                   if(intArray[i]>maxValue)
                           maxValue = intArray[i];
           }

           return maxValue;
   }

   // method returns a min value of integer array
   int minArray(int* intArray, int arrayLength)
   {
           int minValue = intArray[0];		
           int i;

           for(i=1; i<arrayLength; i++)
           {
                   if(intArray[i]<minValue)
                           minValue = intArray[i];
           }

           return minValue;
   }
   
   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsMovementDetector(clsUART *uartPointer)
   {
      uart = uartPointer;
      
      bufferIndex = 0;
      
      walkingDetected = runningDetected = 0;
      highMovementDetected = lowMovementDetected = 0;
      position = lastPosition = 5; // head up

      xIntegral = yIntegral = zIntegral = 0;
      
      runStepDetected = 0;
      
      // ���������� ����� �������������� ��������
      walkStepDetected = 0;
      runStepDetected = 0;
      runningDetected = 0;
      walkingDetected = 0;
      // ������������� ���� ���������� �����������
      noLocomotionDetected = 1;
      // ������������� ���� �������������� ����
      detectWalking = 1;

      // �������� ��������
      walkStepCounter = 0;		// ������� ����� ��� ����
      runStepCounter = 0;			// ������� ����� ��� ����
      lastStepTimerMarker = 0;	// ��������� ������	���������� ������������������� ����
      lastRunStepTimerMarker = 0;	// ��������� ������	���������� ������������������� ����
      noLocomotionMarker = 0;
      
   }//end clsMovementDetector
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   int getWalkingStatus()
   {return walkingDetected;}
   
   int getRunningStatus()
   {return runningDetected;}
   
   int getPosition()
   {return position;}
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   // Method adds samples in buffers for analysis
   void addAccSamples(int x, int y, int z)
   {
      // update value of buffers integrals
      xIntegral += (x - averagingXbuffer[0]);
      yIntegral += (y - averagingYbuffer[0]);
      zIntegral += (z - averagingZbuffer[0]);
      
      // shift buffers
      for(int i=0; i<(ACCELBUFFERLENGTH - 1); i++)
      {
         averagingXbuffer[i] = averagingXbuffer[i + 1];	
         averagingYbuffer[i] = averagingYbuffer[i + 1];	
         averagingZbuffer[i] = averagingZbuffer[i + 1];
      }
      // add new samples
      averagingXbuffer[ACCELBUFFERLENGTH - 1] = x;	
      averagingYbuffer[ACCELBUFFERLENGTH - 1] = y;	
      averagingZbuffer[ACCELBUFFERLENGTH - 1] = z;
      
   }//end addAccSamples
   
   // Method detects a movement
   void movementDetection(void)
   {
      // calculate mean values of acceleration 
      meanX = xIntegral/ACCELBUFFERLENGTH;
      meanY = yIntegral/ACCELBUFFERLENGTH;
      meanZ = zIntegral/ACCELBUFFERLENGTH;
      
      // calculate min values of acceleration 
      minX = minArray(averagingXbuffer, ACCELBUFFERLENGTH);
      minY = minArray(averagingYbuffer, ACCELBUFFERLENGTH);
      minZ = minArray(averagingZbuffer, ACCELBUFFERLENGTH);
      
      // calculate max values of acceleration 
      maxX = maxArray(averagingXbuffer, ACCELBUFFERLENGTH);
      maxY = maxArray(averagingYbuffer, ACCELBUFFERLENGTH);
      maxZ = maxArray(averagingZbuffer, ACCELBUFFERLENGTH);
      
      // calculate span on every axis
      spanX = abs(maxX - minX);
      spanY = abs(maxY - minY);
      spanZ = abs(maxZ - minZ);
      
      // detect motion
      //****************************************************
      if(!runningDetected && !walkingDetected)
      {
         if((spanX>highActivityThreshold)||(spanY>highActivityThreshold)||
            (spanZ>highActivityThreshold)) // high activity detected
         {
                 // ������������� ������� ������������ ����������
                 // � ������� ������ � ���� ������ ������ �� ������
                 
                 //*****************************************************
                 // ���������� ������� �������� ���������� �������
                 //common.highMovementMarker = common.secondsTimer;
                 // drop low activity flag
                 lowMovementDetected = 0;
                 // set high activity flag
                 highMovementDetected = 1;
                 position = 8;
                 
         }
         else if((spanX>lowActivityThreshold)||(spanY>lowActivityThreshold)||
            (spanZ>lowActivityThreshold)) // low activity detected
         {
                 if(!highMovementDetected) //���� ����� �� ���� ���������������� ������� ������������ ����������
                 {
                         // ������������� ������ ������������ ����������
                         // � ������� ������ � ���� ������ ������ �� ������

                         //*******************************************************
                         // ���������� ������� �������� ���������� �������
                         //common.lowMovementMarker = common.secondsTimer;
                         // ������������� ���� ������ ������������ ����������
                         // set low activity flag
                        lowMovementDetected = 1;
                        // drop high activity flag
                        highMovementDetected = 0;
                        position = 7;
                 }
         }
         else
         {
            // drop low activity flag
            lowMovementDetected = 0;
            // drop high activity flag
            highMovementDetected = 0;
         }
      }//end if(!runningDetected && !walkingDetected)
      //******************************************************
      // end detect motion
      
      // detect walking/running
      //***********************************************************************
      // ����������� ���/���
      //*****************************************************************************
      
      newX = averagingXbuffer[ACCELBUFFERLENGTH - 1];

      if((abs(newX-meanXlevel) > runThreshold) && !runStepDetected)
      {
            if((common.samplesTimer - lastRunStepTimerMarker) > runStepInterval)
            {
                    runStepCounter++;
                    noLocomotionDetected = 0;
                    noLocomotionMarker = common.secondsTimer;
                    detectWalking = 0;

                    //debug (blue marker)
                    //sprintf(sampleTxtBuffer, "B\r\n");
                    //xSemaphoreTake(uartMutex, portMAX_DELAY);
                    //uart->sendMessage(sampleTxtBuffer);
                    //xSemaphoreGive(uartMutex);
            }

            runStepDetected = 1;
            lastRunStepTimerMarker = common.samplesTimer;

            if(walkStepDetected)
            {
                    walkStepDetected = 0;
                    walkStepCounter = 0;

                    //debug
                    //printf("A%d=%d=%d\r\n", positionCalculating.walkStepCounter, positionCalculating.walkingDetected, common.position);
            }

            
      }
      //else if(abs(newX - meanXlevel) <= noMovementThreshold)
      else if((newX - meanXlevel) <= noMovementThreshold)
            runStepDetected = 0;

      if(detectWalking)	// ���� �� ������������ ������� ���, ����������� ����� ���
      {
            if((abs(newX-meanXlevel) > walkThreshold) && !walkStepDetected)
            {
                    if((common.samplesTimer - lastStepTimerMarker) > stepInterval)
                    {
                        walkStepCounter++;
                        noLocomotionDetected = 0;
                        noLocomotionMarker = common.secondsTimer;

                        //debug (white marker)
                        //sprintf(sampleTxtBuffer, "W\r\n");
                        //xSemaphoreTake(uartMutex, portMAX_DELAY);
                        //uart->sendMessage(sampleTxtBuffer);
                        //xSemaphoreGive(uartMutex);
                    }
                    walkStepDetected = 1;
                    lastStepTimerMarker = common.samplesTimer;


            }
            //else if(abs(newX-meanXlevel) <= noMovementThreshold)
            else if((newX-meanXlevel) <= noMovementThreshold)
                    walkStepDetected = 0;
      }

      if((walkStepCounter >= numberOfStepsToDetect) && !walkingDetected)
      {
            walkingDetected = 1;
            position = 9;
            if(runningDetected)
            {
                    runningDetected = 0;
                    runStepCounter = 0;
                    //debug
                    //printf("L%d=%d\r\n", positionCalculating.runStepCounter, positionCalculating.runningDetected);
            }
      }
      else if((walkStepCounter < numberOfStepsToDetect) && walkingDetected)
      {
            walkingDetected = 0;
      }
      else if((runStepCounter >= numberOfStepsToDetect) && !runningDetected)
      {
            runningDetected = 1;
            position = 0;

            if(walkingDetected)
            {
                    walkingDetected = 0;
                    walkStepCounter = 0;
                    //debug
                    //printf("A%d=%d=%d\r\n", positionCalculating.walkStepCounter, positionCalculating.walkingDetected, common.position);
            }
      }
      else if((runStepCounter < numberOfStepsToDetect) && runningDetected)
      {
            runningDetected = 0;
      }

      // ���� ���������� ����� �� ���������������� �����������
      if(((common.secondsTimer - noLocomotionMarker) > noLocomotionInterval) && !noLocomotionDetected)
      {
            if(walkingDetected || (walkStepCounter > 0))
            {
                    walkingDetected = 0;
                    walkStepCounter = 0;

                    //debug
                    //printf("A%d=%d=%d\r\n", positionCalculating.walkStepCounter, positionCalculating.walkingDetected, common.position);
            }
            if(runningDetected || (runStepCounter > 0))
            {
                    runningDetected = 0;
                    runStepCounter = 0;

                            //debug
                            //printf("L%d=%d\r\n", positionCalculating.runStepCounter, positionCalculating.runningDetected);
            }

            noLocomotionDetected = 1;
            detectWalking = 1;
      }
      // ��������� �������������� ����/����
      
      //***********************************************************************
      // end detect walking/running
      
      // calculate position
      if(!highMovementDetected && !lowMovementDetected && !runningDetected && !walkingDetected)
      {
             //debug
             //printf("%d  %d  %d\r\n", common.newX, common.newY, common.newZ);
             //printf ("Activity:  none\r\n");

             if((abs(meanZ)>abs(meanX)) && (abs(meanZ)>abs(meanY))) 
             {
                     if(meanZ<0)
                     {
                             // face down
                             position = 1;
                     }
                     else
                             {
                             // face up
                             position = 2;
                     }
             }
             else if((abs(meanY)>abs(meanZ)) && (abs(meanY)>abs(meanX))) 
             {
                     if(meanY<0)
                     {
                             // left side
                             position = 3;
                     }
                     else
                             {
                             // right side
                             position = 4;
                     }
             }
             else if((abs(meanX)>abs(meanZ)) && (abs(meanX)>abs(meanY))) 
             {
                     if(meanX<0)
                     {
                             // head down
                             position = 6;
                     }
                     else
                             {
                             // head up
                             position = 5;
                     }
             }

             // ���������� ��������� ��������� � ��������� �������������
             lastPosition = position;

      }// end ���� ������������ ���������� �� �������������
      
      
   }//end movementDetection
   
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Method ...
   void someMethod(void)
   {
      
   }//end someMethod
   //--------------------------end----------------------------------------------
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
      
};