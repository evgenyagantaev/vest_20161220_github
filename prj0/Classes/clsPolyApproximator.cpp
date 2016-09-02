// This class provides 3 degree polynomial approximation of raw samples 
// sequence (4 points)

class clsPolyApproximator
{
//******************************************************************************   
private://                   PRIVATE ZONE
//******************************************************************************   

   //************************* constants ***************************************
   //static const uint16_t 
   //*********************** end constants *************************************   


   //************************* variables ***************************************
   // matrix of coefficients of a linear system
   double a[4][5];

   //*********************** end variables *************************************


   //********************* private functions ***********************************


   //******************** end private functions ********************************
   
      
//******************************************************************************   
public://                    PUBLIC ZONE
//******************************************************************************      

   //---------------------------------------------------------------------------
   // public constructor
   clsPolyApproximator()
   {
      // initialization if matrix of coefficients
      a[0][0] = 4;
      a[0][1] = 1+2+3;
      a[0][2] = 1+2*2+3*3;
      a[0][3] = 1+2*2*2+3*3*3;
      a[1][0] = 1+2+3;
      a[1][1] = 1+2*2+3*3;
      a[1][2] = 1+2*2*2+3*3*3;
      a[1][3] = 1+2*2*2*2+3*3*3*3;
      a[2][0] = 1+2*2+3*3;
      a[2][1] = 1+2*2*2+3*3*3;
      a[2][2] = 1+2*2*2*2+3*3*3*3;
      a[2][3] = 1+2*2*2*2*2+3*3*3*3*3;
      a[3][0] = 1+2*2*2+3*3*3;
      a[3][1] = 1+2*2*2*2+3*3*3*3;
      a[3][2] = 1+2*2*2*2*2+3*3*3*3*3;
      a[3][3] = 1+2*2*2*2*2*2+3*3*3*3*3*3;
   }//end clsPolyApproximator
   //--------------------------end----------------------------------------------
   
   
   //************************ properties ***************************************
        
   //********************** end properties *************************************
   
   //********************* public functions ************************************
   // Function solves a linear system with Gauss method and fill the vector
   // of system solution, which it gets as a third parameter
   void gauss(int orderOfSystem, double *freeVector, double *solution)
   {
      int i,j,k;
      
      // copy column of free coefficients in the extended matrix of system
      for(i=0; i<orderOfSystem; i++)
         a[i][orderOfSystem] = freeVector[i];
      
      //debug
      /*
      for(i=0;i<orderOfSystem;i++)
      {
         for(j=0;j<=orderOfSystem;j++)
            printf("%f; ", a[i][j]);
         printf("\r\n");
      }
      */
      
      // solve a system
      for(k=0; k<(orderOfSystem-1); k++)
      {
         for(i=k+1; i<(orderOfSystem); i++)
         {
            double aux = a[i][k];
            for(j=k;j<(orderOfSystem+1);j++)
            {
               a[i][j] = a[i][j]*a[k][k] - a[k][j]*aux;
            }
         }
         
      }
      
      int systemOk = 1;
      for(i=0; i<orderOfSystem; i++)
      {
         if(a[i][i] == 0)
            systemOk = 0;
      }
      
      if(systemOk)
      {
         //solution[3] = a[3][4]/a[3][3];
         //solution[2] = (a[2][4] - a[2][3]*solution[3])/a[2][2];
         //solution[1] = (a[1][4] - a[1][3]*solution[3] - a[1][2]*solution[2])/a[1][1];
         //solution[0] = (a[0][4] - a[0][3]*solution[3] - a[0][2]*solution[2] - a[0][1]*solution[1])/a[0][0];
         solution[1] = a[1][2]/a[1][1];
         solution[0] = (a[0][2] - a[0][1]*solution[1])/a[0][0];
      }
      else // we've got a zero members on the main diagonal
      {
         printf("approximation fault!\r\n");
         for(i=0; i<orderOfSystem; i++)
            solution[i] = 0;
      }
      
   }//end gauss
   
   // Function approximates vector of points with polynom
   // gets a vector of points which we want to approximate and length of that
   // vector; returns the rightest point of approximation vector (value of polynom
   // at point 3)
   uint16_t approximation(uint16_t *y, int length)
   {
      // vector of constatnt terms of linear system
      double v[2];
      v[0] = y[0]+y[1]+y[2]+y[3];
      v[1] = y[1]+2*y[2]+3*y[3];
      
      // vector of linear system solution (coefficients of approximating line);
      // is filled by the gauss function
      double a[2];
      
      gauss(2, v, a);
      
      return uint16_t(a[0] + 3*a[1]);
   }//end approximation
   //******************* end public functions **********************************
   
   
   //---------------------------------------------------------------------------
   // Function ...
   void someFunction(void)
   {
      
   }//end someFunction
   //--------------------------end----------------------------------------------
   
      
};