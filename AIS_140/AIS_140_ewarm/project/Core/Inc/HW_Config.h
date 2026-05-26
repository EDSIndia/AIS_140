/******************** (C) COPYRIGHT 2018 EDS-INDIA ***********************
* File Name     : HW_Config.h					       	       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file provides all Application hardware Details          *
* Revision	: Rev0                                  		       *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __Configure_HW_H
#define __Configure_HW_H

#define BAUD_RATE_COM           115200                                          // RS-232 BAUD RATE 
#define BAUD_RATE_COM2          9600                                            // RS-232 BAUD RATE 

// points to check before making file
// 1. check for SW_Date in this page
// 2. check for SW_Ver  in GPRS.c page
// 3. check for watchdog in configure page
// 4. check for SIM DEFINITION type airtel/voda
// 5. comment IAR DEBUG 

#define HW_Ver          "2.0"
#define HW_DATE_TIME    "2026-01-05 16:00:00"

#define FW_VER          "V1.0.3"                                                // also change version number in gprs.c  
#define SW_Date         "2026-02-03 112'908 Byte" 

/*-------SERVER --------------*/
#define BSNL_PC            0
#define DIMTS_PC           1
#define UK_PC              2
#define HP_PC              3
#define NIC_PC             4
#define BSNL_MH            5
#define NIC_OD             6

/*------- IAR DEBUG ------------*/
//#define CAN_ENABLE
//#define RS485_ENABLE

////#define DEBUG_ENB
#define CLIENT_ID        "1"
#define TAISYS                                                                  // SENSORISE / IOTVT / TAISYS
#define TAISYS_AB                                                               // VODA_BSNL / AIRTEL_BSNL / TAISYS_AB / TAISYS_VB
#define VENDOR_ID        "RDME"
#define HEADER_ID        "ID01"
                                                                            
/***************************** HARDWARE MAPING ******************************/
/**************************  USART PIN Definations ****************************/
// USART 3 used for RS-232
#define COM_USART               USART3
#define COM_PORT                GPIOB
#define COM_Tx                  GPIO_Pin_10                                     // USART3(RS-232) TRANSMITTER PIN
#define COM_Rx                  GPIO_Pin_11                                     // USART3(RS-232) RECEIVER PIN

#define COM2_USART              USART6
#define COM2_PORT               GPIOC
#define COM2_Tx                 GPIO_Pin_0                                      // USART3(RS-232) TRANSMITTER PIN
#define COM2_Rx                 GPIO_Pin_1                                      // USART3(RS-232) RECEIVER PIN

/******************************  ON/OFF CONTROL  ******************************/
#define PORT_CHRG               GPIOC
#define PIN_CHRG                GPIO_Pin_1                                      // TO CONTROL CHARGING OF INTERNAL BATTERY 
#define PORT_DEVICE_ON          GPIOC
#define PIN_DEVICE_ON           GPIO_Pin_12                                      // TO ENABLE/DISABLE VEHICLE POWER SUPPLY
#define PORT_PL                 GPIOC                          
#define PIN_PL                  GPIO_Pin_11                                      // PANIC LED

#define PORT_STTS               GPIOB
#define PIN_STTS                GPIO_Pin_7                                      // Ignition LED

#define PORT_PPS                GPIOB
#define PIN_PPS                 GPIO_Pin_6                                      // Ignition LED

#define PORT_NL                 GPIOC
#define PIN_NL                  GPIO_Pin_6                                      // Ignition LED



/******************************  DIGITAL INPUTS  ******************************/
#define PORT_IGN                GPIOA
#define PIN_IGN                 GPIO_Pin_0                                      // TO SENSE VEHICLE IGINITION STATUS 
#define PORT_DIN1               GPIOA                                      
#define PIN_DIN1                GPIO_Pin_7                                     // TO SENSE DIGITAL INPUT 1
#define PORT_DIN2               GPIOC
#define PIN_DIN2                GPIO_Pin_0                                      // TO SENSE DIGITAL INPUT 2
#define PORT_DIN3               GPIOB
#define PIN_DIN3                GPIO_Pin_9                                      // TO SENSE DIGITAL INPUT 4
#define PORT_BATT               GPIOC
#define PIN_BATT                GPIO_Pin_13                                     // TO SENSE EXTERNAL POWER SUPPLY 

/******************************  ANALOG INPUTS  ******************************/
#define PIN_AIN2                GPIO_Pin_1                                      // TO SENSE ANALOG INPUT 3
#define PIN_TEMP                GPIO_Pin_4                                      // TO SENSE LDR
#define PIN_AIN1                GPIO_Pin_5                                      // TO SENSE ANALOG INPUT 1
#define PIN_PB                  GPIO_Pin_6                                      // TO SENSE PANIC BUTTON
#define PORT_ANALOG             GPIOA                                           // ANALOG INPUT PORT

#define PIN_EBV                 GPIO_Pin_0                                      // TO SENSE EXTERNAL BATTERY VOLTAGE
#define PIN_IBV                 GPIO_Pin_1                                      // TO SENSE INTERNAL BATTERY VOLTAGE
#define PORT_ANALOG2            GPIOB

/***************************** DIGITAL OUTPUT *********************************/
#define PORT_DOUT1              GPIOC                                           // DIGITAL OUTPUT PORT
#define PIN_DOUT1               GPIO_Pin_8                                      // TO CONTROL DIGITAL OUTPUT 1 HIGH/LOW

#define PORT_DOUT2              GPIOB                                           // DIGITAL OUTPUT PORT
#define PIN_DOUT2               GPIO_Pin_12                                      // TO CONTROL DIGITAL OUTPUT 2 HIGH/LOW


/*------------- RESERVED PINS FOR FUTURE -----------------------------------*/
#define PORT_RI                 GPIOC
#define PIN_RI                  GPIO_Pin_3
 

/******************************************************************************/

#define IS_BIT_SET(REG, BIT)         (((REG) & (BIT)) != RESET)
#define IS_BIT_CLR(REG, BIT)         (((REG) & (BIT)) == RESET)

#endif