/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : VTS_Functions.c					       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file contains declaration of all misc functions of VTS  *
* Revision	: Rev0    						       *
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#ifndef __Functions_H
#define __Functions_H

void mSec_Delay(unsigned int);                                                  // MILI SECOND DELAY FUNCTION 
void Test_Mode(void);                                                           // SELF TEST FUNCTION TO TEST DEVICE PERIPHERALS
void Key_InputScan(void);                                                       // DIGITAL INPUT SCANNING 
void Get_HitTime(unsigned char);                                                         // GET SERVER HIT TIME 
void Run_IAP(void);                                                             // JUMP FOR OTA TO UPGRADE FIRMWARE 
void Get_Config(void);                                                          // READ CONFIGURATION FROM MICRO FLASH 
void Write_Config(void);                                                        // WRITE CONFIGURATION FILE INTO MICRO FLASH                                                                                                                              
void Serial_SendData_232(char SerialData);                                      // SEND DATA TO SERIAL PORT 
void Serial_PutString_232(char *s,uint8_t);                                             // SEND A STRING TO SERIAL PORT
void ftoa(float num, char *buffer, int befor_point, int after_point);           // FLOAT TO CHARACTER CONVERSION  
void Send_IMEI(void); 
void SET_FACTORY(void);
void REG_LM2576(uint8_t state);
void Second_Timer(void);                                                        // Every Second Activity

#if defined (RS485_ENABLE)
void Serial_PutString_485(char *s) 
#endif


#endif