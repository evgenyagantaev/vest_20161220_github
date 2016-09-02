//This class provides independed watchdog (IWDG) /w 1s counter
class clsWatchdog
{
  public:
  clsWatchdog()
  {
        /* IWDG timeout equal to 250ms.
        The timeout may varies due to LSI frequency dispersion, the 
        LSE value is centred around 32 KHz */
        /* Enable write access to IWDG_PR and IWDG_RLR registers */
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
        /* IWDG counter clock: LSI/32 */
        IWDG_SetPrescaler(IWDG_Prescaler_32);
        /* Set counter reload value to obtain 250ms IWDG TimeOut.
        Counter Reload Value = 250ms/IWDG counter clock period
        = 250ms / (LSI/32)
        = 0.25s / (32 KHz /32)
        = 250
        */
        IWDG_SetReload(WATCHDOGTIMEOUT);
        /* Reload IWDG counter */
        IWDG_ReloadCounter();

        //start();
  }
  
    void start()
    {
        IWDG_Enable();
        
        // configure gpio of hardware watchdog
        GPIO_InitTypeDef GPIO_InitStructure; 
        // hardware watchdog pin PB.8
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;   
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        //GPIO_Init(GPIOB, &GPIO_InitStructure);
    }

    void reload()
    {
        // reload soft watchdog
        IWDG_ReloadCounter();
        // reset hardware watchdog
        GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
    }
  

};