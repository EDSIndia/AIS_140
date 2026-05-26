/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : SPI_Flash.h	                         	               *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This File Defines functions of SPI Flash                     *
* Revision	: Rev0						               *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#define SPI_FLASH_ID            0xEF4018 //0x852018 //0xEF4018 //0x2C25


/* Exported macro ------------------------------------------------------------*/
#define PORT_FLASH_CS           GPIOA
#define PIN_FLASH_CS            GPIO_Pin_15                                     // SPI1 NSS CHIP SELECT PIN
 

#define PORT_FLASH              GPIOB
#define PIN_FLASH_SCK           GPIO_Pin_3                                     // SPI1 NSS CHIP SELECT PIN
#define PIN_FLASH_MISO          GPIO_Pin_4                                     // SPI1 NSS CHIP SELECT PIN
#define PIN_FLASH_MOSI          GPIO_Pin_5                                     // SPI1 NSS CHIP SELECT PIN
  


#define Flash_ON                PORT_FLASH_CS->BRR  = PIN_FLASH_CS
#define Flash_OFF               PORT_FLASH_CS->BSRR = PIN_FLASH_CS

// Just for Verification
#define  FLASH_WriteAddress     0xF00000                                        // Address above 15 MB 
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress
#define  FLASH_ID             





/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(unsigned long SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(char* pBuffer, unsigned long WriteAddr, unsigned short NumByteToWrite);
void SPI_FLASH_BufferWrite(char* pBuffer, unsigned long WriteAddr, unsigned short NumByteToWrite);
void SPI_FLASH_BufferRead(char* pBuffer, unsigned long ReadAddr, unsigned short NumByteToRead);
unsigned long SPI_FLASH_ReadID(void);
void SPI_FLASH_PD(void);
void SPI_FLASH_StartReadSequence(unsigned long ReadAddr);

///*----- Low layer function -----*/
//unsigned char SPI_FLASH_ReadByte(void);
//unsigned char SPI_FLASH_SendByte(unsigned char byte);
//unsigned short SPI_FLASH_SendHalfWord(unsigned short HalfWord);
//void SPI_FLASH_WriteEnable(void);
//void SPI_FLASH_WaitForWriteEnd(void);

#endif /* __SPI_FLASH_H */