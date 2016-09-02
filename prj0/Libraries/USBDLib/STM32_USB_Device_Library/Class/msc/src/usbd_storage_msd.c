/**
  ******************************************************************************
  * @file    usbd_storage_template.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include "usbd_msc_mem.h"
#include "main.h"
#include "stm32_sdio_common.h"
#include "stm32_sdio_sd.h"
    #include <stdint.h>
#include "integer.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
    extern uint8_t sdInitOk;
/* Private functions ---------------------------------------------------------*/

#define STORAGE_LUN_NBR                  1                    

int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun, 
                           uint32_t *block_num, 
                           uint32_t *block_size);

int8_t  STORAGE_IsReady (uint8_t lun);

int8_t  STORAGE_IsWriteProtected (uint8_t lun);

int8_t STORAGE_Read (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_Write (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_GetMaxLun (void);

/* USB Mass storage Standard Inquiry Data */
const int8_t  STORAGE_Inquirydata[] = {//36
  
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (USBD_STD_INQUIRY_LENGTH - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'p', 'M', 'T', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'M', 'o', 'n', 'i', 't', 'o', 'r', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '3', '.', '2' ,'2',                     /* Version      : 4 Bytes */
}; 

USBD_STORAGE_cb_TypeDef USBD_MICRO_SDIO_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
  
};



USBD_STORAGE_cb_TypeDef  *USBD_STORAGE_fops = &USBD_MICRO_SDIO_fops;
/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Init (uint8_t lun)
{
  if(!sdInitOk) return (-1);
  return (0);
}

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
  IWDG_ReloadCounter();
  SD_CardInfo SDCardInfo;
  
  SD_GetCardInfo(&SDCardInfo);  
  
  if(SD_GetStatus() != 0 )
  {
    return (-1); 
  }   

  *block_size =  512;  
  *block_num =  SDCardInfo.CardCapacity / 512;  
  
  return (0);
}

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t  STORAGE_IsReady (uint8_t lun)
{
  if(!sdInitOk) return (-1);

  return (0);
}

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
  return  0;
}

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Read (uint8_t lun, 
                 uint8_t *buf, 
                 uint32_t blk_addr,                       
                 uint16_t blk_len)
{
  IWDG_ReloadCounter();
  /*
  if( SD_ReadMultiBlocks (buf, 
                          blk_addr * 512, 
                          512,
                          blk_len) != 0)
  {
    return -1;
  }

  SD_WaitReadOperation();
  while (SD_GetStatus() != SD_TRANSFER_OK);
   
  return (0);*/
  
  

  memset(buf, 0xFF, 512 * blk_len);  

      if(SD_ReadMultiBlocks((BYTE*)buf, blk_addr*512, 512, blk_len) != SD_OK ) 
          return (-1);
      
      if( SD_WaitReadOperation() != SD_OK ) 
          return (-1);

//#ifdef NOSDIOTIMEOUT
   
   while(SD_GetStatus() != SD_TRANSFER_OK);
  /* 
#else
   uint32_t timeout = 0xFFFF;
   
   while( (SD_GetStatus() != SD_TRANSFER_OK) && (timeout>0) )
   {
     timeout--;
   }
   
   if(timeout==0) return (-1);
   
#endif
    */
   return (0); 
  
}
/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_Write (uint8_t lun, 
                  uint8_t *buf, 
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
   IWDG_ReloadCounter();
  /* if( SD_WriteMultiBlocks (buf, 
                           blk_addr * 512, 
                           512,
                           blk_len) != 0)
  {
    return -1;
  }
  
  SD_WaitWriteOperation();
  while (SD_GetStatus() != SD_TRANSFER_OK);  

  return (0);
   */
   
   
   if( SD_WriteMultiBlocks((uint8_t*)buf, blk_addr*512, 512, blk_len) != SD_OK ) 
          return (-1);

    
    if( SD_WaitWriteOperation() != SD_OK ) 
          return (-1); 
    
//#ifdef NOSDIOTIMEOUT
   
   while(SD_GetStatus() != SD_TRANSFER_OK);
 /*  
#else
   uint32_t timeout = 0xFFFF;
   
   while( (SD_GetStatus() != SD_TRANSFER_OK) && (timeout>0) )
   {
     timeout--;
   }
   
   if(timeout==0) return (-1);
   
#endif
   */ 
   return (0);  
}
/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the STORAGE card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int8_t STORAGE_GetMaxLun (void)
{
  return (STORAGE_LUN_NBR - 1);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
