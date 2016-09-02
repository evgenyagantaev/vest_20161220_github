/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "system_stm32f2xx.h"
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
    
#include "semphr.h"
#include "queue.h"
   
/* ADC1 Global defenition-----------------------------------------------------*/ 
// length of cyclic buffer for ecg samples  
#define ADC1BUFFERLEN 512
// length of cyclic buffer for accelerometer samples  
#define ACCBUFFERLEN 512
    
    
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

