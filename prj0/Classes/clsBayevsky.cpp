// This class provides functionality for Bayevsky tension index calculation

class clsBayevsky
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   static const int RRVECTORLENGTH = 600;
   //*********************** end constants *************************************   


   //************************* variables ***************************************
   // array of RR intervals
   int rrVector[RRVECTORLENGTH];

   //*********************** end variables *************************************


   //********************* private functions ***********************************


   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsBayevsky()
   {
      
   }//end clsBayevsky
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
   int getRRbyIndex(int index)
   { return rrVector[index]; }
   
   int getRRVECTORLENGTH(void)
   { return RRVECTORLENGTH; }
   
   int *getRRvectorPointer(void)
   { return rrVector; }
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   // Function shifts a vector of rr intervales and saves new value of rr interval
   void saveNewRR(int rrInterval)
   {
      for(int i= 1; i<RRVECTORLENGTH; i++)
         rrVector[i-1] = rrVector[i];
      rrVector[RRVECTORLENGTH - 1] = rrInterval;
   }//end saveNewRR
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};