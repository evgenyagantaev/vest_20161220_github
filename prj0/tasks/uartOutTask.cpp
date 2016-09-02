// This task transmits via uart (using DMA) all messages from all tasks
// in one single stream
  
void uartOutTask(void *parameters)
{
    static int local_call_stack_pointer;
   char task_id = 'd';
    while(1)
    {
        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKIN(task_id)
        xSemaphoreGive(callStackMutex);
        
        if(!uartOutputInProcess)
        {
            uart.outStringViaDma();
        }
        taskYIELD();

        xSemaphoreTake(callStackMutex, portMAX_DELAY);
        CHECKOUT(task_id)
        xSemaphoreGive(callStackMutex);
    }
}