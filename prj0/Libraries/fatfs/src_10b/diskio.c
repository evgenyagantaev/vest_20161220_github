/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include <stdint.h>
#include "stm32_sdio_common.h"
#include "stm32_sdio_sd.h"

#define SDIOTIMEOUT 0xFFFF;
DWORD systemTime = ((2013UL-1980) << 25)       // Year = 2013
      | (2UL << 21)       // Month = Feb
      | (12UL << 16)       // Day = 12
      | (10U << 11)       // Hour = 10
      | (30U << 5)       // Min = 30
      | (0U >> 1)       // Sec = 0
      ;


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  if (pdrv) return STA_NOINIT;		/* Supports only single drive */
  return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
  // Read the data from the Card  
  //printf("---rd sect %d count %d\r\n",sector,count);
  
  //watchdog.reload();
  memset(buff, 0xFF, 512 * count);  
  /*
    if(count>1)
    {
    */  
      if( SD_ReadMultiBlocks((BYTE*)buff, sector*512, 512, count) != SD_OK ) 
          return RES_ERROR;
      
      if( SD_WaitReadOperation() != SD_OK ) 
          return RES_ERROR;
/*
    }
    else
    {
      
      if( SD_ReadBlock((BYTE*)buff, sector*512, 512) != SD_OK ) 
          return RES_ERROR;
      
      if( SD_WaitReadOperation() != SD_OK ) 
          return RES_ERROR;

    }
  */  
#ifdef NOSDIOTIMEOUT
   
   while(SD_GetStatus() != SD_TRANSFER_OK);
   
#else
   uint32_t timeout = SDIOTIMEOUT;
   
   while( (SD_GetStatus() != SD_TRANSFER_OK) && (timeout>0) )
   {
     timeout--;
   }
   
   if(timeout==0) return RES_ERROR;
   
#endif
    
   return RES_OK; 
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	   // Write the data from the Card 
  
//printf("---wr sect %d count %d\r\n",sector,count);
/*
for(int i=0;i<10;i++)
{
printf("%c",buff[i]);
}
printf("\r\n");
*/
    //watchdog.reload();
 /*  if(count>1)
    { 
   */   
      
    if( SD_WriteMultiBlocks((uint8_t*)buff, sector*512, 512, count) != SD_OK ) 
          return RES_ERROR;

    
    if( SD_WaitWriteOperation() != SD_OK ) 
          return RES_ERROR; 

      
    /*  for(int i=0;i<count;i++)
      {
        if( SD_WriteBlock((uint8_t*)buff, sector*512*(i+1), 512) != SD_OK ) 
          return RES_ERROR;

    
        if( SD_WaitWriteOperation() != SD_OK ) 
          return RES_ERROR;
        
      }
      */
      /*
    }
   else
   {
     
    if( SD_WriteBlock((uint8_t*)buff, sector*512, 512) != SD_OK ) 
          return RES_ERROR;

    
    if( SD_WaitWriteOperation() != SD_OK ) 
          return RES_ERROR;

   }
   */
#ifdef NOSDIOTIMEOUT
   
   while(SD_GetStatus() != SD_TRANSFER_OK);
   
#else
   uint32_t timeout = SDIOTIMEOUT;
   
   while( (SD_GetStatus() != SD_TRANSFER_OK) && (timeout>0) )
   {
     timeout--;
   }
   
   if(timeout==0) return RES_ERROR;
   
#endif
    
   return RES_OK;  

}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
  DRESULT res = RES_OK;
  switch (cmd) {
    
    case CTRL_SYNC:   // make sure we have availability
    
        res = RES_OK;
     
    break;

    case CTRL_TRIM:   // make sure we have availability
    
        res = RES_OK;
     
    break;
    
  case GET_SECTOR_COUNT :	  // Get number of sectors on the disk (DWORD)
    { IWDG_ReloadCounter();
      SD_CardInfo SDCardInfo;
  
  SD_GetCardInfo(&SDCardInfo);  
  
    *(DWORD*)buff = SDCardInfo.CardCapacity / 512; 
    res = RES_OK;
    break;
    }
  case GET_SECTOR_SIZE :	  // Get R/W sector size (WORD) 
    *(WORD*)buff = 512;
    res = RES_OK;
    break;
    
  case GET_BLOCK_SIZE :	    // Get erase block size in unit of sector (DWORD)
    *(DWORD*)buff = 1;
    res = RES_OK;
  }
  
  return res;
}
#endif

/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/

DWORD get_fattime ()
{
  return systemTime;
}

void set_fattime (DWORD t)
{
  systemTime = t;
}