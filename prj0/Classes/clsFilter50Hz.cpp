// This class provides ...

class clsFilter50Hz
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   static const uint16_t localBufferLength = 20;
   //*********************** end constants *************************************   


   //************************* variables ***************************************
   //portTickType oldTickCounter = xTaskGetTickCount();
   portTickType tickMarkerMax1; // time markers of local maxs and mins
   portTickType tickMarkerMin1;
   portTickType tickMarkerMax2;
   portTickType tickMarkerMin2;
   portTickType tickMarkerMax3;
   portTickType tickMarkerMin3;
   
   // time interval between local max and local min
   portTickType timeSpan;
   
   // local buffers
   uint16_t upperBound[localBufferLength];
   uint16_t buffer[localBufferLength];
   uint16_t lowerBound[localBufferLength];
   //indices that poit in the position (in the local buffers)
   // of last found extremum
   int lastMinPosition, lastMaxPosition;
   
   // amplitude values of maxs and mins
   uint16_t lastMax, lastMin; 
   
   //raw data samples
   uint16_t rawSample;
   uint16_t previousSample;
   
   //filtered data samples
   uint16_t filteredSample;
   
   //span of noice amplitude
   uint16_t noiceSpan;
   
  
   // local extremum finding regime
   int lookForMax;
   int maxFound;
   int inflectionFound;
   int inflectionSuspect;
   int lookForMin;
   int minFound;
   
   int less, greater, equal;
   
   

   //*********************** end variables ************************************


   //********************* private functions **********************************
   void shiftBuffers(void)
   {
      for(int i=0;i<(localBufferLength-1);i++)
      {
         upperBound[i] = upperBound[i+1];
         buffer[i] = buffer[i+1];
         //----------------------------------
         //debug
         //buffer[localBufferLength-1] = rawSample;
         //----------------------------------
         lowerBound[i] = lowerBound[i+1];
         
         lastMinPosition--; 
         lastMaxPosition--;
         
         //debug
         //printf("MAX %d\r\n", lastMaxPosition);
      }
   }

   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsFilter50Hz()
   {
     
      // local extremum finding regime
      lookForMax = 1;
      maxFound = 0;
      inflectionFound = 0;
      inflectionSuspect = 0;
      lookForMin = 0;
      minFound = 1;
      
      less = greater = 0;
      equal = 1;
      
      //initialize local buffers
      for(int i=0; i<localBufferLength; i++)
      {
         upperBound[i] = buffer[i] = lowerBound[i] = 0;
      }
      lastMinPosition = lastMaxPosition = localBufferLength -1;
      
   }//end clsFilter50Hz()
   
   // function performs 50 hz filtration
   void filtration(void)
   {
      // variables for interpolation calculation
      int x1, x2, y1, y2;
      double K;
      
      
      if(maxFound)
      {
         // add fresh max in upper bound buffer
         upperBound[localBufferLength-1] = lastMax;
         
         
         //compute linear interpolation
         if(lastMaxPosition < 0 )
            x1 = 0;
         else
            x1 = lastMaxPosition;
         x2 = localBufferLength -1;
         y1 = upperBound[x1];
         y2 = upperBound[x2];
         if(x1 != x2)
         {
            K = double(y2 - y1)/double(x2 - x1);
            for(int i=1; (x1+i)<x2; i++)
               upperBound[x1+i] = uint16_t(K*i + upperBound[x1]);
            
            //debug 
            //printf("%d  %f  %d\r\n", (uint16_t)(upperBound[x1] + (int)(K*(x2-x1))), K, x2-x1);
            //printf("%d  %d\r\n", lastMaxPosition, x2-x1);
         }
         
         
         // compute filtered samples as a mean of upper bound and lower bound
         //     -------^--------_---------^
         if((lastMinPosition >= 0) && (lastMaxPosition >= 0) && 
         (lastMinPosition > lastMaxPosition))
         {
            for(int i=0; i<(lastMinPosition-lastMaxPosition+1); i++)
            {
               int j = lastMaxPosition + i;
               buffer[j] = (upperBound[j] + lowerBound[j])/2;
            }
         }
         
         
         
         // point index of buffer to the last found max
         lastMaxPosition = localBufferLength -1;
      }
      else if(minFound)
      {
         // add fresh min in lower bound buffer
         lowerBound[localBufferLength-1] = lastMin;
         
         
         //compute linear interpolation
         if(lastMinPosition < 0 )
            x1 = 0;
         else
            x1 = lastMinPosition;
         x2 = localBufferLength -1;
         y1 = lowerBound[x1];
         y2 = lowerBound[x2];
         if(x1 != x2)
         {
            K = double(y2 - y1)/double(x2 - x1);
            for(int i=1; (x1+i)<x2; i++)
               lowerBound[x1+i] = uint16_t(K*i + lowerBound[x1]);
            
            //debug 
            //printf("%d  %f  %d\r\n", (uint16_t)(upperBound[x1] + (int)(K*(x2-x1))), K, x2-x1);
            //printf("%d  %d\r\n", lastMinPosition, x2-x1);
         }
         
         
         // compute filtered samples as a mean of upper bound and lower bound
         //     -------^--------_---------^
         if((lastMaxPosition > lastMinPosition))
         {
            for(int i=0; i<(lastMaxPosition -lastMinPosition+1); i++)
            {
               int j = lastMinPosition + i;
               buffer[j] = (upperBound[j] + lowerBound[j])/2;
            }
         }
         
         
         
         // point index of buffer to the last found max
         lastMinPosition = localBufferLength -1;
         
      }// end if(maxFound)...else if(minFound)
      
      
      int k = localBufferLength -1;
      buffer[k] = (upperBound[k] + lowerBound[k])/2;
      
      //debug
      //buffer[k] = previousSample;
         
      //filteredSample = buffer[0];
      
      //debug
      filteredSample = previousSample;
      
   }// end filtration
   
   // function is looking for a local maximum
   void findMaxMinInflection(void)
   {
      // drop down flags of max and min finding
      maxFound = 0;
      minFound = 0;
      
      if(less)
      {
         if(rawSample < previousSample)
         {
            // nothing to do
         }
         else if(rawSample > previousSample)
         {
            // we have found a local minimum
            minFound = 1;
            lastMin = previousSample;
            less = 0;
            greater = 1;
            equal = 0;
            
            //debug
            //printf("MIN %d\r\n", lastMinPosition);
         }
         else // previousSample == rawSample
         {
            // we have found an inflection point
            // interprete it as a local minimum in this situation
            minFound = 1;
            lastMin = previousSample;
            less = 0;
            greater = 0;
            equal = 1;
            
            //debug
            //printf("MIN %d\r\n", lastMinPosition);
         }
      }
      else if(greater)
      {
         if(rawSample < previousSample)
         {
            // we have found a local maximum
            maxFound = 1;
            lastMax = previousSample;
            less = 1;
            greater = 0;
            equal = 0;
            
            //debug
            //printf("MAX %d\r\n", lastMaxPosition);
         }
         else if(rawSample > previousSample)
         {
            // nothing to do
         }
         else // previousSample == rawSample
         {
            // we have found an inflection point
            // interprete it as a local maximum in this situation
            maxFound = 1;
            lastMax = previousSample;
            less = 0;
            greater = 0;
            equal = 1;
            
            //debug
            //printf("MAX %d\r\n", lastMaxPosition);
         }
      }
      else if(equal)
      {
         if(rawSample < previousSample)
         {
            // we interprete it as a local maximum in this situation
            maxFound = 1;
            lastMax = previousSample;
            less = 1;
            greater = 0;
            equal = 0;
            
            //debug
            //printf("MAX %d\r\n", lastMaxPosition);
         }
         else if(rawSample > previousSample)
         {
            // we interprete it as a local minimum in this situation
            minFound = 1;
            lastMin = previousSample;
            less = 0;
            greater = 1;
            equal = 0;
            
            //debug
            //printf("MIN %d\r\n", lastMinPosition);
         }
         else // previousSample == rawSample
         {
            // nothing to do
         }
      }
      
            
   }// end findMaxMinInflection
   
   
   // function saves a fresh sample (which it gets as a parameter) 
   //as a rawSample variable value
   void updateSample(uint16_t newSample)
   {
      previousSample = rawSample;
      rawSample = newSample;
      shiftBuffers();
   }// end updateSample
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   uint16_t getFilteredSample()
   {
      return filteredSample;
   }// end getFilteredSample
   
   uint16_t getNoiceSpan()
   {
      return noiceSpan;
   }// end getNoiceSpan 
   
   portTickType getTimeSpan()
   {
      return timeSpan;
   }// end getTimeSpan 
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Method ...
   void someMethod(void)
   {
      
   }//end someMethod
   //--------------------------end----------------------------------------------
   
      
};