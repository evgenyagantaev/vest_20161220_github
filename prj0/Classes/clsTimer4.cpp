// This class provides functionality of timer TIM4 
// zapuskaet preryvanie kazhdye 4 mS (250 Hz)
// po preryvaniyu schityvaem sample ECG

class clsTimer4
{
public:
    clsTimer4()
    {

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
        TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
        TIM_TimeBaseStructure.TIM_Prescaler = 600;
        TIM_TimeBaseStructure.TIM_Period = 400; //60Mhz/(600*400)=250Hz
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

        TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

        //TIM_Cmd(TIM4, ENABLE);
        //NVIC_EnableIRQ(TIM4_IRQn);

    }
    
    void startTimer4(void)
    {
        TIM_Cmd(TIM4, ENABLE);
        NVIC_EnableIRQ(TIM4_IRQn);

        //DEBUG************************************

    }
  
};

extern "C" void TIM4_IRQHandler(void) //timer 4 interrupt handler
{
    static int counter = 0;
    
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE; 
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    
    // iniciiruem poluchenie dannyh ot ads1220 *****
    // give semaphore to the ISR handler task
    xSemaphoreGiveFromISR(newEcgSampleTimerSemaphore, &xHigherPriorityTaskWoken);
    
    //DEBUG
    /*
    if(counter == 0)
    {
        counter = 1;
        GPIOC->BSRRH = GPIO_Pin_2;
    }
    else
    {
        counter = 0;
        GPIOC->BSRRL = GPIO_Pin_2;
    }
    */
    //DEBUG

    // do the context switch, if necessery
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
   
}