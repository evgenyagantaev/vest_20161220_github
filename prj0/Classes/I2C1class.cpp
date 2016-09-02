#include "main.h"

class clsI2C1 //I2C1 master class @ SCL=PB.6 and SDA=PB.7 pins /w SMB380 accelerometer
{  

public:
   uint8_t TempBuffer;
   uint8_t WAddr, RAddr;
   uint8_t GetHeading;
   uint8_t RxBuffer[6];
   
   clsI2C1() //Construct
   {
     TempBuffer=0;
     WAddr = 0x70, RAddr = 0x71;
     GetHeading = 0x8;
     
     /* Put in global buffer same values */
    for (int index = 0; index < 6; index++ )
    {
      RxBuffer[index] = 0x00;
    }
    
    /*I2C Peripheral clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    /*SDA and SCL GPIO clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
      
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
    
    
    I2C_DeInit(I2C1);
    
    I2C_InitTypeDef  I2C_InitStructure;
 
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &I2C_InitStructure);
    
    //I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

    
    /* DMA Configuration */
    /* Clear any pending flag on Tx Stream  */
  //  DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);                                 
    /* Clear any pending flag on Rx Stream  */
    DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0 | DMA_FLAG_FEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_HTIF0);
  
    /* Disable the I2C Tx DMA stream */
   // DMA_Cmd(DMA1_Stream6, DISABLE);
    /* Configure the DMA stream for the I2C peripheral TX direction */
  //  DMA_DeInit(DMA1_Stream6);
  
    /* Disable the I2C Rx DMA stream */
    DMA_Cmd(DMA1_Stream0, DISABLE);
    /* Configure the DMA stream for the I2C peripheral RX direction */
    DMA_DeInit(DMA1_Stream0);
    
    
    DMA_InitTypeDef  DMA_InitStructure;
  
    DMA_DeInit(DMA1_Stream0);
    DMA_InitStructure.DMA_Channel = DMA_Channel_1;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2C1->DR;       
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
    /* Init DMA for Reception */
    /* Initialize the DMA_DIR member */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    /* Initialize the DMA_Memory0BaseAddr member */
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)RxBuffer;
   /* Initialize the DMA_BufferSize member */
    DMA_InitStructure.DMA_BufferSize = 6;
    DMA_DeInit(DMA1_Stream0);
    DMA_Init(DMA1_Stream0, &DMA_InitStructure);
  
    /* Init DMA for Transmission */
    /* Initialize the DMA_DIR member */
 //   DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    /* Initialize the DMA_Memory0BaseAddr member */
 //   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TxBuffer;
    /* Initialize the DMA_BufferSize member */
  //  DMA_InitStructure.DMA_BufferSize = 3;
 //   DMA_DeInit(DMA1_Stream6);
   // DMA_Init(DMA1_Stream6, &DMA_InitStructure);
   }

   float GetAccTemperature(void)
   {
    GetHeading = 0x8; //Temperature register address
     
    I2C_AcknowledgeConfig(I2C1, ENABLE);
  /*----------------------------- Transmission Phase -------------------------*/

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  /*!< EV5 */

    I2C_Send7bitAddress(I2C1, WAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); /*!< EV6 */
  
    I2C_SendData(I2C1, GetHeading);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); /*!< EV8 */

    I2C_GenerateSTOP(I2C1, ENABLE);
  
  /*------------------------------- Reception Phase --------------------------*/
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  /*!< EV5 */

    I2C_Send7bitAddress(I2C1, RAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));  /*!< EV6 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));  /*!< EV7 */
 
    TempBuffer = I2C_ReceiveData(I2C1);
  
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
  
     return (float)TempBuffer/2.0-30.0;
   }
   void GetAccXYZ(void)
   {
     GetHeading = 0x2; //Temperature register address
     
    I2C_AcknowledgeConfig(I2C1, ENABLE);
  /*----------------------------- Transmission Phase -------------------------*/

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  /*!< EV5 */

    I2C_Send7bitAddress(I2C1, WAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); /*!< EV6 */
  
    I2C_SendData(I2C1, GetHeading);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); /*!< EV8 */

    I2C_GenerateSTOP(I2C1, ENABLE);
  
  /*------------------------------- Reception Phase --------------------------*/
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  /*!< EV5 */

    I2C_Send7bitAddress(I2C1, RAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));  /*!< EV6 */
    
    
   I2C_DMACmd(I2C1, ENABLE);
  // Enable DMA NACK automatic generation 
  I2C_DMALastTransferCmd(I2C1, ENABLE);
  
  // Enable DMA RX Channel 
  DMA_Cmd(DMA1_Stream0, ENABLE);
  
  // Wait until I2Cx_DMA_STREAM_RX enabled or time out 
  while (DMA_GetCmdStatus(DMA1_Stream0)!= ENABLE);

 
  // Transfer complete or time out 
  while (DMA_GetFlagStatus(DMA1_Stream0,DMA_FLAG_TCIF0)==RESET);


  // Send I2Cx STOP Condition 
  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);

  // Disable DMA RX Channel 
  DMA_Cmd(DMA1_Stream0, DISABLE);
  
  // Wait until I2Cx_DMA_STREAM_RX disabled or time out 
  while (DMA_GetCmdStatus(DMA1_Stream0)!= DISABLE);

  // Disable I2C DMA request   
  I2C_DMACmd(I2C1,DISABLE);
  
  // Clear any pending flag on Rx Stream  
  DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0 | DMA_FLAG_FEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_HTIF0);
    
  //  while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
   }
};