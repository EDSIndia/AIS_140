/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : SPI_MEMS.h					       	       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This File Defines functions of SPI MEMS                      *
* Revision	: Rev0 						      	       *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_MEMS_H
#define __SPI_MEMS_H



/************************** MEMS PINOUT ***************************************/
#define PORT_MEMS_CS            GPIOD                                          
#define PIN_MEMS_CS             GPIO_Pin_2                                     // TO ENABLE MEMES AS ACTIVE TO SPI BUS 

#define PORT_MEMS_DATA          GPIOB  
#define PIN_MEMS_MOSI           GPIO_Pin_15        
#define PIN_MEMS_MISO           GPIO_Pin_14
#define PIN_MEMS_SCK            GPIO_Pin_13

#define PORT_MEMS_INT1          GPIOC
#define PIN_MEMS_INT1           GPIO_Pin_10                                     // TO GIVE AN INTERRUPT TO MEMS 

#define PORT_MEMS_INT2          GPIOB
#define PIN_MEMS_INT2           GPIO_Pin_2                                     // TO GIVE AN INTERRUPT TO MEMS 


#define MEMS_ON                 PORT_MEMS_CS->BRR = PIN_MEMS_CS
#define MEMS_OFF                PORT_MEMS_CS->BSRR = PIN_MEMS_CS


void SPI_MEMS_Init(void);
void MEMS_INIT(void);
void GET_MEMSData(void);


#endif 