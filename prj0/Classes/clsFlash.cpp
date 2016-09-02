//This class provides flash operations for power-off and iap processing
#include "flash_if.h"
#include "flash_if.c"

class clsFlash
{
public:
  clsFlash()
  {
    FLASH_If_Init();
  }
  /*
          0 - power off ok
          1 - power off err
          2 - iap
  */
  void writePowerOffOk()
  {
        uint32_t rd = 0;
        FLASH_Status flashStatus;
        uint32_t addr = ADDR_FLASH_SECTOR_1; //address to write
    
        // debug flash/////////////////////////////////////////////////////////////

        // Disable all interrupts 
        RCC->CIR = 0x00000000;

        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        flashStatus = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);

        FLASH_Lock();


        uint32_t wrval = 0; //value to write
        uint16_t cnt = 1; //count of units to write


        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        //operation_result = FLASH_If_Write(&addr, &wrval ,(uint16_t)cnt); //write to flash

        flashStatus = FLASH_ProgramWord(ADDR_FLASH_SECTOR_1, wrval);

        FLASH_Lock();
        // sanity check
        rd = 777;
        rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1); //read last status from flash
        // debug flash/////////////////////////////////////////////////////////////
        
  }
  void writePowerOffErr()
  {
            uint32_t rd = 0;
        FLASH_Status flashStatus;
        uint32_t addr = ADDR_FLASH_SECTOR_1; //address to write
    
        // debug flash/////////////////////////////////////////////////////////////


        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        flashStatus = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);

        FLASH_Lock();


        uint32_t wrval = 1; //value to write
        uint16_t cnt = 1; //count of units to write


        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        //operation_result = FLASH_If_Write(&addr, &wrval ,(uint16_t)cnt); //write to flash

        flashStatus = FLASH_ProgramWord(ADDR_FLASH_SECTOR_1, wrval);

        FLASH_Lock();
        // sanity check
        rd = 777;
        rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1); //read last status from flash
        // debug flash/////////////////////////////////////////////////////////////
        
  }
  void writeIap()
  {
        uint32_t rd = 0;
        FLASH_Status flashStatus;
        uint32_t addr = ADDR_FLASH_SECTOR_1; //address to write
    
        // debug flash/////////////////////////////////////////////////////////////

        // Disable all interrupts 
        //RCC->CIR = 0x00000000;
        NVIC_DisableIRQ(TIM4_IRQn);

        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        flashStatus = FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3);

        FLASH_Lock();


        uint32_t wrval = 2; //value to write


        FLASH_Unlock();
        // Clear pending flags incase they were triggered
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                        FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
        //operation_result = FLASH_If_Write(&addr, &wrval ,(uint16_t)cnt); //write to flash

        flashStatus = FLASH_ProgramWord(ADDR_FLASH_SECTOR_1, wrval);

        FLASH_Lock();
        // sanity check
        rd = 777;
        rd = (*(__IO uint32_t*) ADDR_FLASH_SECTOR_1); //read last status from flash
        // debug flash/////////////////////////////////////////////////////////////
        
        NVIC_EnableIRQ(TIM4_IRQn);
  }
  
};