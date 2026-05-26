/******************** (C) COPYRIGHT 2018 EDS INDIA******************************
* File Name     : GNSS.h   	              		       	               *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file has all subroutibnes for GPS functions             *
* Revision	: Rev0       		                                       *
*******************************************************************************/
/* DEFINE TO PREVENT RECURSIVE INCLUSION -------------------------------------*/

#ifndef __GNSS_H
#define __GNSS_H

/*----- DEFINE VALUES --------------------------------------------------------*/
#define GNSS_Fixed               18                                             // LOCATION FOR GPS DATA VALID SIGNAL
#define GNSS_INTERVAL            3                                              // EVERY 3 SECONDS CHECKS GPS DATA

#define BAUD_RATE_GNSS           9600      					// GPS BAUD RATE  // FOR IRNSS TESTING


// USART 2 USED FOR GNSS
#define GNSS_USART               USART2
#define GNSS_PORT                GPIOA
#define GNSS_Tx                  GPIO_Pin_2                                     // USART2(GPS) TRANSMITTER PIN
#define GNSS_Rx                  GPIO_Pin_3                                    // USART2(GPS) RECEIVER PIN             


#define PORT_GNSS                GPIOB
#define PIN_GNSS                 GPIO_Pin_8                                     // TO ENABLE/DISABLE GPS POWER SUPPLY

#define GNSS_ENB                 PORT_GNSS->BRR    = PIN_GNSS                    // INVERSE LOGIC TRANSISTOR
#define GNSS_DSB                 PORT_GNSS->BSRR   = PIN_GNSS                    // INVERSE LOGIC TRANSISTOR

#define PORT_L89_WKP             GPIOC
#define PIN_L89_WKP              GPIO_Pin_14                                      // L89 Wake Up

#define RST_GNSS                 PORT_L89_WKP->BRR = PIN_L89_WKP
#define RUN_GNSS                 PORT_L89_WKP->BSRR = PIN_L89_WKP



#define OUT     0
#define IN      1
/*----- FUNCTIONS ------------------------------------------------------------*/

void INIT_USART_GNSS(void);
void Get_Location(void);   
void Get_Location_IRNSS(void);
void Get_Location_GNSS(void);

void GNSS_PutString(char *s);
void MODULE_L89(uint8_t state);
//void MODULE_L89_STNDBY(uint16_t time);
void MODULE_L89_FLR(void);
int Calc_Distance(float, float, float, float);
void Set_APN(void);

void IRNSS_ENB(void);

/*----- DATA TYPES & VARIABLES -----------------------------------------------*/
typedef struct									// DATA STRUCTURE FOR GNSS 
  {
   char DATE[10];                                                               // CURRENT DATE                /*  3-GPS & GNSS BOTH FIX       */
   char TIME[10];                                                               // CURRENT TIME                /*  4-3D GPS FIX                */
   char LAT[15];                                                                // LATITUDE RAW VALUE          /******* GPS VALIDITY INDEX *****/
   char LON[15];                                                                // LONGITUDE RAW VALUE         /*  0- GPS & GNSS NOT FIX       */                
   char SPEED[10];                                                               // CURRENT SPEED IN KNOT       /*  1-GPS FIX                   */
   char DIRECTION[10];                                                          // CURRENT DIRECTION           /*  2-GNSS FIX                  */ 
   char Altitude[10];                                                            // ALTITUDE
   char PDOP[10];                                                                // DILLUTION OF PRECISION      /*  5-3D GNSS FIX               */
   char HDOP[10];
   unsigned char Used_Satellite;                                                         // NUMBER OF USED SATELLITE    /*  6-3D GPS & 3D GNSS BOTH FIX */
  }GNSS_InitTypeDef;
extern GNSS_InitTypeDef    IRNSSData;


typedef struct									// DATA STRUCTURE FOR VEHICLE 
  {
    uint8_t ID;
    float LAT[10];
    float LON[10];
    uint8_t Last_FS;
  }GF_InitTypeDef;
extern GF_InitTypeDef    GF[10];

#endif