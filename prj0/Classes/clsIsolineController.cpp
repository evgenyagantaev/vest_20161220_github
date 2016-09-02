// This class computes isoline and adjusts the isoline level using DAC

class clsIsolineController
{
   
private:
   
   // elements of lists that are containing samples of cyclic and sorted buffers
   // (for isoline calculating)
   struct strSortedBufferList
   {
       uint16_t sample;
       strSortedBufferList *next;
       strSortedBufferList *previous;
   };
   struct strBufferList
   {
       uint16_t sample;
       strBufferList *next;
       strSortedBufferList *sorted;
   };
   
   

   // pointers to heads and tails of buffers
   strSortedBufferList *sortedBuffer;  
   strBufferList *samplesBuffer;
   strBufferList *samplesBufferTail;

   strBufferList *bufferPointer;
   strSortedBufferList *sortedBufferPointer;
   
   
   //********************************************************************
   // function adds samples in cyclic and sorted buffers
    void addSampleInBuffer(uint16_t sample)
   {
           
        int jobIsDone = 0;
        int endOfListReached = 0;
        strSortedBufferList *pointer;

       // move head element to the tail
       samplesBufferTail->next = samplesBuffer;
       samplesBuffer = samplesBuffer->next;
       samplesBufferTail = samplesBufferTail->next;
       samplesBufferTail->next = NULL;
       // save new sample value
       samplesBufferTail->sample = sample;
       // remove element from sorted buffer
       if (samplesBufferTail->sorted->previous != NULL)
           samplesBufferTail->sorted->previous->next = samplesBufferTail->sorted->next;
       else
           sortedBuffer = samplesBufferTail->sorted->next;
       if (samplesBufferTail->sorted->next != NULL)
           samplesBufferTail->sorted->next->previous = samplesBufferTail->sorted->previous;
       samplesBufferTail->sorted->previous = NULL;
       samplesBufferTail->sorted->next = NULL;
       // add element in sorted buffer
       samplesBufferTail->sorted->sample = sample;
           
           
           pointer = sortedBuffer;

       while (!jobIsDone && !endOfListReached)
       {
           if (sample < pointer->sample)
           {
               if (pointer != sortedBuffer) // less then minimal
               {
                   pointer->previous->next = samplesBufferTail->sorted;
                   samplesBufferTail->sorted->previous = pointer->previous;
               }
               else
               {
                   sortedBuffer = samplesBufferTail->sorted;
               }
               samplesBufferTail->sorted->next = pointer;
               pointer->previous = samplesBufferTail->sorted;

               jobIsDone = 1;
           }
           else
           {
               if (pointer->next != NULL)
               {
                   pointer = pointer->next;
               }
               else
               {
                   endOfListReached = 1;
               }

           }
       }

           

       if (endOfListReached) // greater then maximal
       {
           // add to the tail
           pointer->next = samplesBufferTail->sorted;
           samplesBufferTail->sorted->previous = pointer;
       }

   }
   //********************************************************************
   // end addSampleInBuffer
   
   
   // function computes isoline value
   uint16_t computeIsoline()
   {
       // cut edge values of sorted buffer
       strSortedBufferList *pointer = sortedBuffer;

       for (int i = 0; i < SAMPLESTOCUT; i++)
       {
           pointer = pointer->next;
       }
       // compute average
       int average = 0;
       for (int i = 0; i < (BUFFERLENGTH - 2 * SAMPLESTOCUT); i++)
       {
           average += pointer->sample;
                   pointer = pointer->next;
       }

           
       return (uint16_t)(average / (BUFFERLENGTH - 2 * SAMPLESTOCUT));
   }// end computeIsoline()

   
public:
   
   //************************* constants ***************************************
   static const uint16_t AVERAGINGBUFFERLENGTH = 15;
   static const uint16_t TARGETISOLINELEVEL = 2048;
   static const uint16_t MAXISOLINEDEVIATION = 20;
   // isoline deviation which turns on active adjustment mode
   static const uint16_t ACTIVEMODEISOLINEDEVIATION = 150;
   static const uint16_t DACSTEP = 3;
   // max number which we can set to DAC (matches supply voltage 3.3V)
   static const uint16_t MAXDACINPUT = 4095;

   // length of cyclic and sorted buffers, which we are using for isoline
   // calculation
   static const uint16_t BUFFERLENGTH = 13;
   // number of samples we cut on the edges of sorted buffer
   static const uint16_t SAMPLESTOCUT =  5;
   //*********************** end constants *************************************
   
   // flag; if set, says that mode of active adjustment of isoline level is ON
   int activeMode;
   
   // currentvalue of isoline level
   uint16_t currentIsoline;
   
   // public constructor
   clsIsolineController()
   {
      // turn off active mode
      activeMode = 0;
      
      //***************************************************************
      //create and init buffers:
      
      // create first element
      samplesBuffer = (strBufferList *)malloc(sizeof(strBufferList));	
      // create first element   
      sortedBuffer = (strSortedBufferList *)malloc(sizeof(strSortedBufferList));			
      sortedBuffer->previous = NULL;
      bufferPointer = samplesBuffer;
      sortedBufferPointer = sortedBuffer;

      for (int i = 0; i < BUFFERLENGTH; i++)
      {
          bufferPointer->sample = i;
          sortedBufferPointer->sample = i;
          bufferPointer->sorted = sortedBufferPointer;
                          
          if (i != (BUFFERLENGTH - 1))
          {
              bufferPointer->next = (strBufferList *)malloc(sizeof(strBufferList));
              sortedBufferPointer->next = (strSortedBufferList *)malloc(sizeof(strSortedBufferList));
              if (i != 0)
              {
                  sortedBufferPointer->next->previous = sortedBufferPointer;
              }
              bufferPointer = bufferPointer->next;
              sortedBufferPointer = sortedBufferPointer->next;
          }
      }
      // set pointer to the tail of buffer
      samplesBufferTail = bufferPointer;
      // write NULL as a value of "next" field of last element
      bufferPointer->next = NULL;
      sortedBufferPointer->next = NULL;
      
      //end create and init buffers
      //***************************************************************

   }// end clsIsolineController() (public constructor)
   
   // function updates isoline level and returns current fresh value of isoline level
   uint16_t getIsoline(uint16_t newSample)
   {
      addSampleInBuffer(newSample);
      currentIsoline = computeIsoline();
      return currentIsoline;
   }
   
   
};