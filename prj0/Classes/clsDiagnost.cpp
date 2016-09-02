// This class provides functionality for ecg analysis

class clsDiagnost
{
private://**********************************************************************

   //************************* constants ***************************************
   static const uint16_t HARDBRADYTHRESHOLD = 40;
   static const uint16_t BRADYTHRESHOLD = 50;
   static const uint16_t HARDTACHYTHRESHOLD = 185;
   static const uint16_t TACHYTHRESHOLD = 125;
   static const uint16_t numberOfRepeatingStatesToSwitch = 15;
   static const uint16_t RECREATIONPERIODLENGTH = 120; // in seconds
   
  
   static const uint16_t timeToGetStartPulse = 20;      // time (seconds) to set start pulse after walking or running
   static const uint16_t startPulseAdd = 5;             // add to start pullse
   static const uint16_t tuningInterval = 10;           // floating tachicardia tuning interval
      
   //*********************** end constants *************************************   


   //************************* variables ***************************************

   // value reprezents operator condition
   int status;
   // local copy of current heart rate value
   //int pulse;
   
   // floating tachicardia threshold
   int tachiThreshold;
   
   int yellowCounter;
   int redCounter;
   
   int state, previousState;
   
   // interval of diagnostics information output (in seconds)
   long DIAGNOSTICSINTERVAL;
   
   // flag that indicates that recreation period after walking or running
   // not finished yet
   int recreationPeriodPending;
   
   // timer (counter) that counts recreation period
   int recreationPeriodTimer;
   
   int lastDisplayedPulse; // last normal value of pulse which was displayed
   
   double recreationRatio;      // текущий темп восстановления после нагрузки
   double recreationRatio1;     // темп восстановления после нагрузки ([удары в минуту]/секунды) на первом участке (1 минута)
   double recreationRatio2;	// темп восстановления после нагрузки ([удары в минуту]/секунды) на втором участке (2 минута)
   double recreationRatio3;	// темп восстановления после нагрузки ([удары в минуту]/секунды) на третьем участке
   
   // time marker which fixes the moment when running or walking stops
   long walkingRunningStopMarker;
   
   // start pulse for recreation
   int startPulse;

   // time marker of last tachicardia tuning moment
   long lastTachycardiaTuningMarker;
   // flag; indicates that floating tachicardia threshold is tuned yet
   int tachycardiaThresholdTuned;
   
   int badAdcRangeFlag;
   long badAdcRangeMarker;
   
   //*********************** end variables ************************************


   //********************* private functions **********************************



   //****************** end private functions **********************************
   
public://***********************************************************************

   static const uint16_t LOWBOUNDADCDIAPAZONE = 500; 
   static const uint16_t UPPERBOUNDADCDIAPAZONE = 65500; 
   static const uint16_t ADCRANGEPERIOD = 1; // in seconds
   
   // flag; when set, signals that input of adc is out of range
   int badAdcRange;
   
   int adcRangeTimer;
   
   int badIsoline;
   
   //---------------------------------------------------------------------------
   // public constructor
   clsDiagnost()
   {
      status = 0;
      DIAGNOSTICSINTERVAL = 2;
      tachiThreshold = TACHYTHRESHOLD;
      
      yellowCounter = redCounter = 0;
      state = previousState = 0;
      
      recreationPeriodPending = 0;
      
      recreationPeriodTimer = 0;
      
      badAdcRange = 1;
      adcRangeTimer = 0;
      
      lastDisplayedPulse = 444;
      
      recreationRatio1 = 0.1;  
      recreationRatio2 = 0.2;	
      recreationRatio3 = 0.3;
      
      walkingRunningStopMarker = 0;
      
      startPulse = 0;
      
      lastTachycardiaTuningMarker = 0;
      tachycardiaThresholdTuned = 0;
      
      badAdcRangeFlag = 0;
      badAdcRangeMarker = 0;

   }//end clsDiagnost
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   
   int getStatus()
   {return status;}
   
   long getInterval()
   {return DIAGNOSTICSINTERVAL;}
   
   void setInterval(long interval)
   {DIAGNOSTICSINTERVAL = interval;}
   
   void setLastDisplayedPulse(int pulse)
   {
      lastDisplayedPulse = pulse;
   }
   
   int getLastDisplayedPulse()
   {return lastDisplayedPulse;}
   
   int getTachiThreshold()
   {return tachiThreshold;}
   
   int getBadAdcRangeFlag()
   { return badAdcRangeFlag; }
   
   void setBadAdcRangeFlag()
   { badAdcRangeFlag = 1; }
   
   void resetBadAdcRangeFlag()
   { badAdcRangeFlag = 0; }
   
   long getBadAdcRangeMarker()
   { return badAdcRangeMarker; }
   
   void setBadAdcRangeMarker(long timer)
   { badAdcRangeMarker = common.secondsTimer; }
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   
   //---------------------------------------------------------------------------
   // Method performs diagnostics
   void makeDiagnosis(int pulse, int walkingDetected, int runningDetected, 
                      int position)
   {
    
      if(walkingDetected || runningDetected || (position == 8))
      {
         walkingRunningStopMarker = common.secondsTimer;
      }
      else // no walking, no running
      {
         long noLocomotionPeriod = common.secondsTimer - walkingRunningStopMarker;
         //if(noLocomotionPeriod < RECREATIONPERIODLENGTH)
         if(1)
         {
           
            // during start pulse set period, set start pulse
            //if((noLocomotionPeriod < timeToGetStartPulse) && (startPulse < lastDisplayedPulse))
            if((noLocomotionPeriod < timeToGetStartPulse) && (tachiThreshold < lastDisplayedPulse))
            {
               tachiThreshold = lastDisplayedPulse + startPulseAdd;
               startPulse = lastDisplayedPulse;

               if(recreationRatio != recreationRatio1)
                  recreationRatio = recreationRatio1;

            }
            else if((noLocomotionPeriod >= timeToGetStartPulse) && (startPulse < tachiThreshold))
               startPulse += startPulseAdd;

            if((tachiThreshold > TACHYTHRESHOLD) && 
               ((common.secondsTimer - lastTachycardiaTuningMarker) >= tuningInterval)) // tune only if floating threshold exceeds fixed one
            {
               // подстраиваем в моменты времени кратные 10 секундам
               if(noLocomotionPeriod > timeToGetStartPulse)
               {
                  // choose a recreation curve slope
                  if((noLocomotionPeriod > 60) && (noLocomotionPeriod < 120) && (recreationRatio != recreationRatio2))
                     recreationRatio = recreationRatio2;
                  else if((noLocomotionPeriod > 120) && (recreationRatio != recreationRatio3))
                     recreationRatio = recreationRatio3;

                  if(!tachycardiaThresholdTuned)
                  {
                     tachiThreshold -= (int)(recreationRatio * tuningInterval);
                     lastTachycardiaTuningMarker = common.secondsTimer;
                     tachycardiaThresholdTuned = 1;
                  }
               }
                   
            }
            
            if(tachycardiaThresholdTuned && (common.secondsTimer - lastTachycardiaTuningMarker) >= tuningInterval)
               tachycardiaThresholdTuned = 0;


         }
      }


      if(walkingDetected || runningDetected || (position == 8))
      {
         status = 1;
      }
      else if(pulse < HARDBRADYTHRESHOLD) // hard bradycardia
      {
         //printf ("OPERATOR_CONDITION %d\r\n", 3);	// hard wounded
         status = 3;
      }
      else if(pulse < BRADYTHRESHOLD) // light bradycardia
      {
         //printf ("OPERATOR_CONDITION %d\r\n", 2);	// light wounded
         status = 2;
      }
      else if(pulse < 300)
      {
         if(pulse > tachiThreshold) // tachycardia
         {
            if(pulse > HARDTACHYTHRESHOLD) // hard tachycardia
            {
                  //printf ("OPERATOR_CONDITION %d\r\n", 3);	// hard wounded
                  status = 3;
            }
            else     // light tachycardia
            {
                  //printf ("OPERATOR_CONDITION %d\r\n", 2);	// light wounded
                  status = 2;
            }
         }
         else 
         {
                 //printf ("OPERATOR_CONDITION %d\r\n", 1);	// боеспособен
                 status = 1;
         }
      }
      else // pulse >= 300
         status = 0;
      
      if(status == 3)
      {
         redCounter++;
         yellowCounter = 0;
      }
      else if(status == 2)
      {
         redCounter = 0;
         yellowCounter++;
      }
      else 
      {
         redCounter = 0;
         yellowCounter = 0;
      }
      
      
      // меняем цвет маркера только если состояние повторяется заданное количество раз
      if(yellowCounter >= numberOfRepeatingStatesToSwitch)	// хочет пожелтеть
      {
              yellowCounter = numberOfRepeatingStatesToSwitch;
              state = 2;
      }
      else if(redCounter >= numberOfRepeatingStatesToSwitch)	// хочет покраснеть
      {
              redCounter = numberOfRepeatingStatesToSwitch;
              state = 3;
      }
      else if(status == 1)
      {
              state = 1;
      }
      else 
              state = previousState;
      
      previousState = state;
      status = state;
      
      
}
   
   //--------------------------end----------------------------------------------

   //****************** end public functions ***********************************
   
   
   //---------------------------------------------------------------------------
   // Method 
   
   //--------------------------end----------------------------------------------
   
   
      
};
