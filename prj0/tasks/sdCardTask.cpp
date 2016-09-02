//This task provides sdio write operation
void sdCardTask(void *pvParameters)
{
  
    static int local_call_stack_pointer;
    char task_id = 'n';
    while(1)
    {
        CHECKIN(task_id)
        
        xSemaphoreTake(sdioOutputSemaphore, portMAX_DELAY);
        if(!common.commandMode) sdio.writeToFile();
        taskYIELD(); 
        
        CHECKOUT(task_id)
    }
  
}