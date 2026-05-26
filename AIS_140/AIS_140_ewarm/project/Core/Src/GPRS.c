/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : GPRS.c					               *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file Configures GPRS & Deals with ITs functions         *
* Revision	: Rev0 			                                       *
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Main.h"
#include "App_Init.h"                                                           // STARTUP APP FUNCTIONS 
#include "GPRS.h"

/*---------- STATIC VARIABLE & FUNCTIONS -------------------------------------*/

extern uint8_t LIS3DH_WriteReg(uint8_t Reg, uint8_t Data);

/*******************************************************************************
* Function Name : INIT_GPRS_MODEM
* Description   : Initialize GPRS Modem
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void MODULE_EC200_RST(void)
{
  uint16_t 
  GP_Counter;
  
  TAKE_FUL_BKP();
  
  GPRS_OFF;
  GP_Counter = 0;
  if(Flag_TCPConnect[MAIN]){Flag_TCPConnect[MAIN] = RESET; Modem_PutString("AT+QICLOSE=0");WAIT_MODEM_RESP(2); TCP_State[MAIN] = OPEN_SOCKET;};
  if(Flag_TCPConnect[TER]){Flag_TCPConnect[TER] = RESET; Modem_PutString("AT+QICLOSE=1");WAIT_MODEM_RESP(2); TCP_State[TER] = OPEN_SOCKET;};
  if(Flag_TCPConnect[EMR]){Flag_TCPConnect[EMR] = RESET; Modem_PutString("AT+QICLOSE=2");WAIT_MODEM_RESP(2); TCP_State[EMR] = OPEN_SOCKET;};
      
        
/*---------- RESET GPRS DATA -------------------------------------------------*/
  sprintf(GPRSData.SIM_Present,"0");                                            // SET SIM PRESENT NO
  sprintf(GPRSData.Operator,"N.A");                                             // SET OPERATOR NAME 
  
  sprintf(GPRSData.Cell_ID,"N.A");                                              // SET CELL ID NONE
  sprintf(GPRSData.Mobile_No,"N.A.000000");                                     // SET MOBILE NUMBER NONE
  sprintf(Reply_No,"N.A.000000");
  sprintf(Random_Code,"N.A.00");
  sprintf(GPRSData.LAC,"N.A");
  GPRSData.MCC = 0;
  GPRSData.MNC = 0;
  VTS_State = MODEM_INIT;
  
  GP_Counter = 4;
  while(GP_Counter)
  { 
    GP_Counter--;
    sprintf(NBRCell[GP_Counter].CELLID,"0"); 
    sprintf(NBRCell[GP_Counter].LAC,"0"); 
    NBRCell[GP_Counter].SIG = 0;
  }
  
  /*----- FLAG STATUS --------------------------------------------------------*/
  Flag_ModemRdy = RESET;                                                        // MODEM READY RESET       
  Flag_ValidNetwork = RESET;                                                    // VALID NETWORK RESET 
  Flag_APNSet = RESET;                                                          // APN NAME RESET 
  Flag_NoResp = SET;                                                            // INITIAL NO RESPONSE FROM MODEM 
  Flag_CellChange = SET;                                                        // AS TO INITIALIZE GPRS              
  ChkSMS_Time = 10;                                                             // CHECK SMS INTERVAL 60 SECONDS @ STARTUP

}

/*******************************************************************************
* Function Name : GPRS USART Initialization
* Description   : Initialization USART communication
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void INIT_USART_GPRS(void)
{
  GPIO_InitTypeDef              GPIO_InitStructure;
  USART_InitTypeDef             USART_InitStructure;
    
  GPIO_PinAFConfig(PORT_GPRSUSART,GPIO_PinSource9,GPIO_AF_1);                           // PIN MAPPED TO ALTERNATRE FUNCTION 
  GPIO_PinAFConfig(PORT_GPRSUSART,GPIO_PinSource10, GPIO_AF_1);                           // PIN MAPPED TO ALTERNATRE FUNCTION   
  
  /*---------- CONFIGURE USART1 AS ALTERNATE FUNCTION MODE & OPEN DRAIN ------*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                                  // PIN MODE ALTERNATE FUNCTION 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                             // PIN @ MAX SPEED 
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;                                // OUTPUT TYPE OPEN DRAIN                                       
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;                             // NO PULL UP / NO PULL DOWN 
  GPIO_InitStructure.GPIO_Pin   = GPRS_Tx;                                      // TX PIN OF MICRO
  GPIO_Init(PORT_GPRSUSART, &GPIO_InitStructure);                                    // INITIALIZE GPRS PORT
  
 /*---------- CONFIGURE USART1 RX AS ALTERNATE FUNCTION MODE & PULL UP ------*/
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                                  // PIN IS INTERNALLY PULLED UP
  GPIO_InitStructure.GPIO_Pin = GPRS_Rx ;                                       // RX PIN OF MICRO
  GPIO_Init(PORT_GPRSUSART, &GPIO_InitStructure);                                    // INITIALIZE GPRS PORT

  
  
  /* USART1 configuration - for GPRS/GSM-------------------------------------*/
  /* USART1 configured as follow:
        - BaudRate = Define in Board Defination
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
   
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);                        // ENABLE USRAT 1 CLOCK
  USART_DeInit(GPRS_USART);                                                     // RESET TO DEFAULT STATE 

  USART_InitStructure.USART_BaudRate = BAUD_RATE_GPRS;                          // SET GPRS BAUDRATE  DEFINED IN HEADER FILE
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;                   // 8 BIT
  USART_InitStructure.USART_StopBits = USART_StopBits_1;                        // 1 STOP
  USART_InitStructure.USART_Parity = USART_Parity_No;                           // PARITY NONE
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// HARDWARE FLOW NONE
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;               // Rx & Tx ENABLED

  /* INITIALIZE USART 1 */
  USART_Init(GPRS_USART, &USART_InitStructure);                                 // INITIALIZE USART 1 

  /* Enable the USART Receive interrupt: this interrupt is generated when the
   USART1 receive data register is not empty */
  USART_ITConfig(GPRS_USART, USART_IT_RXNE, ENABLE);                            // ENABLE RECEIVE INTERRUPT

  /* Enable USART1 */
  USART_Cmd(GPRS_USART, ENABLE);                                                // ENABLE USART 1
}
/*******************************************************************************
* Function Name : Serial_SendData_GPRS
* Description   : Send Data to Modem
* Input         : one hex data only
* Output        : None
* Return        : None
*******************************************************************************/
void Serial_SendData_GPRS(char SerialData)					// THIS FUNCTION SEND ONLY ONE HEX VALUE AT A TIME	
 {
   while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		// WAIT FOR COMPLETE TRANMISSION TILL FLAG IS 0 
     
   GPRS_USART->TDR = (SerialData & (uint16_t)0x01FF);                           // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
 }
/*******************************************************************************
* Function Name  : Serial_PutData
* Description    : Print a string on the GPRS Modem till place define
* Input          : A String up to given place
* Output         : None
* Return         : None
*******************************************************************************/
void Serial_PutData_GPRS(char *s, char Place)                                   // THIS FUNCTION SEND A STRING'S CHARACTERS UPTO GIVEN PLACE 
{  
  mSec_Delay(10);
  while (*s != '\0' && Place)
  {
    Place--;                                                                    // PUT DATA TO USART UPTO NUMBER OF PLACE AS POINTER BY POINTER 
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);                // WAIT UNTILL LAST TRANSMISSION COMPLETES
    GPRS_USART->TDR = (*s & (uint16_t)0x01FF);                                  // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
    s++;                                                                        // INCREMENT POINTER
    mSec_Delay(2);                                                              // WAIT FOR 2 MILI SECOND
  };
   mSec_Delay(5);                                                               // WAIT FOR 5 MILI SECOND
}
/*******************************************************************************
* Function Name  : Serial_PutString
* Description    : Print a string on the GPRS Modem
* Input          : A String till null character
* Output         : None
* Return         : None
*******************************************************************************/
void Serial_PutString_GPRS(char *s)                                             // THIS FUNCTION SEND STRING'S CHARACTER TILL NULL CHARACTER
{  
  mSec_Delay(10);
  while (*s != '\0')                                                            // PUT STRING OVER USART TILL NULL CHARACTER 
  {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);               // WAIT UNTILL LAST TRANSMISSION COMPLETES
    GPRS_USART->TDR = (*s & (uint16_t)0x01FF);                                  // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
    s++;                                                                        // INCREMENT POINTER
    delay(20);                                                                     
  };
   //mSec_Delay(5);                                                               // WAIT FOR 5 MILI SECOND

}

void delay(uint8_t nTime)
{
  while(nTime != 0)
  {
    __NOP();
    nTime--;
  }  
}
/*******************************************************************************
* Function Name : Modem_PutString_GPRS
* Description   : Send data to Modem and add 13(LF) at the end
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void Modem_PutString(char *s)                                                   // THIS FUNCTION SENDS A STRING TO GPRS MODEM AND ADD LF AT THE END 
{
   
 memset(GPRS_Buffer,0,GPRSBuffer_Size);
 GPRS_Count = 0;   
 mSec_Delay(10);
 Flag_ModemOK = RESET;
  while (*s != '\0')
  {
   while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		// WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
     
   GPRS_USART->TDR = (*s & (uint16_t)0x01FF);                                   // SEND NEXT STRING VALUE
   s++;                                                                         // INCREMENT POINTER 
   mSec_Delay(2);
  };
  mSec_Delay(5);
  
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	                // WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
  GPRS_USART->TDR = (13 & (uint16_t)0x01FF);					// SEND DATA BYTE 13
  
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	                // WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
  GPRS_USART->TDR = (10 & (uint16_t)0x01FF);					// SEND DATA BYTE 13
  
}
/*******************************************************************************
* Function Name : INIT_GPRS_MODEM
* Description   : Initialize GPRS Modem
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void INIT_GPRS_Modem(void)                                                      // THIS FUNCTION INITIALIZE THE MODEM FOR WORKING 
{
  char *RPtr = NULL;
  Modem_PutString("AT+CRSM=176,28539,0,0,12");                                  // Clear FPLMN
  WAIT_MODEM_RESP(2); 
  Modem_PutString("AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"");     // Clear FPLMN
  WAIT_MODEM_RESP(2); 
  Modem_PutString("AT+CFUN=1");                                                 // Trigger Reset
  WAIT_MODEM_RESP(2);
 
  Modem_PutString("AT+QURCCFG=\"urcport\",\"uart1\"");WAIT_MODEM_RESP(2);       // Set USART 1 for URC
  Modem_PutString("AT+QSTK?");WAIT_MODEM_RESP(2);  
  if(strstr(GPRS_Buffer,"STK: 0"))
  {
   Modem_PutString("AT+QSTK=1");WAIT_MODEM_RESP(2);
   EC200_ONTime = 15;                                                           // RESET MODEM
  }
  else 
  {   
     sprintf(GPRSData.IMEI_No,"N.A.00000000000");                               // CLEAR IMEI NUMBER 
     Modem_PutString("ATI");                WAIT_MODEM_RESP(2);                 // CHECK VERSION
     Modem_PutString("AT+CIMI");            WAIT_MODEM_RESP(2);
     Modem_PutString("AT+IFC=0,0");         WAIT_MODEM_RESP(2);                 // HANDSHAKING NONE
     Modem_PutString("AT+CSMS=1");          WAIT_MODEM_RESP(2);                 // Set Messeging serivce
     Modem_PutString("AT+CMGF=1");          WAIT_MODEM_RESP(2);                 // ENABLE TEXT MODE
     Modem_PutString("AT+CSCS=\"IRA\"");    WAIT_MODEM_RESP(2);                 // Set character set as GSM
     Modem_PutString("AT+CSMP=17,167,0,241");WAIT_MODEM_RESP(2);                // SET SMS TEXT MODE PARAMETERS DEFAULT VALUES  
     Modem_PutString("AT+CPMS=\"SM\",\"SM\",\"SM\"");  WAIT_MODEM_RESP(2);      // Preffered SMS Storage Location
     Modem_PutString("AT+CSDH=1");          WAIT_MODEM_RESP(2);                 // SMS Show mode
     Modem_PutString("AT+QSIMSTAT=1");      WAIT_MODEM_RESP(2);                 // ENABLE SIM STATUS REPORTING 
     Modem_PutString("AT+CGREG=2");         WAIT_MODEM_RESP(2);                 // ENABLE REPORTING OF GPRS REGISTRATION WITH 
     Modem_PutString("AT+CTZU=3");          WAIT_MODEM_RESP(2);                 // UPDATE LOCALIZED TIME & TIME ZONE TO RTC
     Modem_PutString("AT+CMEE=2");          WAIT_MODEM_RESP(2);                 // Display CME Error ciode
        
     memset(GPRS_Buffer,0,50);GPRS_Count = 0;
     Flag_WrongIMEI = SET; 
     Flag_ReadData = SET;
     Modem_PutString("AT+GSN");                                                 // REQUEST FOR IMEI NUMBER   
     mSec_Delay(500);
     if(strstr((char*)GPRS_Buffer,"OK"))                           
     {
       if(RPtr = strstr((char*)GPRS_Buffer,"GSN"))   
       {
         Flag_WrongIMEI = RESET;
         RPtr += 4;
         if(*RPtr < '0' || *RPtr > '9') RPtr += 2;                                // SET POINTER TO READ IMEI NUMBER 
         for(GP_Counter = 0;GP_Counter <=14; GP_Counter++,RPtr++)                    // EXTRACT 15 DIGIT IMEI NUMBER 
         {
           if(*RPtr < '0' || *RPtr > '9'){Flag_WrongIMEI = SET; break;}             // IF CHAR OTHER THAN NUMERIC IT MEANS WRONG IMEI NUMBER 
           GPRSData.IMEI_No[GP_Counter] = *RPtr;                                    // SAVE IMEI NUMBER                               
         };
         Flag_ReadData = RESET;
         GPRSData.IMEI_No[GP_Counter] = 0;  
       }
     }
     if(Flag_KeyScan == SET){Flag_KeyScan = RESET;Key_InputScan(); }
     
     Modem_PutString("AT+CCID");WAIT_MODEM_RESP(2); 
     if(Flag_ModemOK)
     {
       if(RPtr = strstr((char*)GPRS_Buffer,"ID: "))
       {
         RPtr += 4;
         GPRSData.SIM_Present[0] = '1';                                             // MODEM RESPONDING i.e. SIM IS PRESENT
         for(GP_Counter = 0;GP_Counter <=19; GP_Counter++,RPtr++)                    // EXTRACT 15 DIGIT IMEI NUMBER 
           GPRSData.ICCID[GP_Counter] = *RPtr;                                       // SAVE IMEI NUMBER                               
        }
        Flag_ModemOK = RESET; 
     }

     Modem_PutString("AT+CGREG=2"); WAIT_MODEM_RESP(2);                         // ENABLE REPORTING OF GPRS REGISTRATION WITH 
     VTS_State = NORMAL;
     if(GPRSData.SIM_Present[0] == '1')Flag_ModemRdy = SET;
  }
}

/*******************************************************************************
* Function Name : GPRS_Managment
* Description   : Manage GPRS Data Hit activities
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/

void GPRS_Managment(void)
{
   /*----- GPRS TIME ACTIVITY --------------------------*/
   if(VTS_State == NORMAL)
   {
     State_Error = 0;
     for(int S = 0; S < 3; S++)
     {
       if(Response_Time[S] == 0 && (TCP_State[S] == 1 || TCP_State[S] == 3))Connect_Error[S]++;
       if(Connect_Error[S] > 5){Connect_Error[S] = 0; TCP_State[S] = CLOSE_SOCKET;  }
     }
     
     // Send ACTV/HCHK Response
     if(Flag_H_A_SMS)
     {
       if(Flag_TCPConnect[MAIN] == SET)
       {
          Flag_H_A_SMS = RESET;
          Modem_PutString("AT+QISEND=0");
          mSec_Delay(100);
          Serial_PutString_GPRS(RESP_BUFFER);
          Serial_SendData_GPRS(10);mSec_Delay(10);     // SEND DATA FOR UPLOADING THEN LF LF
          Serial_SendData_GPRS(26);WAIT_MODEM_RESP(5);                                  // TERMINAE STRING & WAIT MODEM RESPONSE 

       };
     }
     
        
     if(ServerHit_Time >(Current_Hit_Time+5))ServerHit_Time = Current_Hit_Time;
     
     if(ChkSMS_Time > 1){ChkSMS_Time--;if(ChkSMS_Time == 1){ChkSMS_Time = 10; Flag_ChkSMS = SET; };} 
     else ChkSMS_Time = 10;
     
     if(ChkSRVR_Time > 1){ChkSRVR_Time--;if(ChkSRVR_Time == 1){ChkSRVR_Time = 12; Flag_ChkSRVR = SET; };}
     else ChkSRVR_Time = 12;
     
         /*----- Receive data from Remote Server ---------------*/
     
     if(strstr(Recieve_Buffer,"+ACC"))
     {
       if(Flag_NewConf == RESET)
       {
         strcpy(Config_Buffer,Recieve_Buffer);                                // COPY RECIEVE BUFFER INTO GPRS MSG BUFFER
         Flag_NewConf = SET;
         Check_Config();  
         memset(Config_Buffer,0, sizeof(Config_Buffer));
         memset(Recieve_Buffer,0, sizeof(Recieve_Buffer));
         RcvData_Time = 1;
       }
       
     }
     
     if(Flag_ServerConf[MAIN] == SET || Flag_ServerConf[EMR] == SET || Flag_ServerConf[TER] == SET)
     {
       Rcv_CntCR = 0;
       memset(CC_Source_TER,0, sizeof(CC_Source_TER));
  
       if(Flag_ServerConf[MAIN]){ Modem_PutString("AT+QIRD=0,256"); Flag_ServerConf[MAIN] = RESET; Flag_ServerResp[MAIN] = SET; } // Main and TER will reset after checking Config
       else if(Flag_ServerConf[TER]){ Modem_PutString("AT+QIRD=1,256"); Flag_ServerConf[TER] = RESET; Flag_ServerResp[TER] = SET;}
       else if(Flag_ServerConf[EMR]){ Modem_PutString("AT+QIRD=2,256");Flag_ServerConf[EMR] = RESET;}
       WAIT_MODEM_RESP(2);      
     }
     if(Flag_RecieveCfg == RESET)
     {
       /*----- CELL TOWER CHANGED --------------------------*/
       if(Flag_CellChange == SET)                             // If cell tower changed
       {           
         //if(Response_Time[MAIN] > 0 || Response_Time[TER] > 0)return;
         Flag_ModemOK = RESET;                         
         Check_Signal_Nw_GPRS();
         if(Flag_ValidNetwork == SET && Flag_APNSet == RESET)Set_APN();  // IF APN IS NOT VALID SET APN FIRST
         if(GPRSData.Signal_Strength >= VALID_SIGNAL && Flag_ValidNetwork && Cell_CheckTime == 0)
         {
           Flag_CellChange = RESET;
           Get_CellLocation();
           Get_NBRData();          
         };
        };   
      
       /*----- Hit Primary Server --------------------------*/
       if((RAM_BakCnt[MAIN] > 0 || TCP_BackCnt[MAIN] > 0) && (Response_Time[TER] < 115))TCP_Connection(MAIN);
           
       /*----- Hit Tertary Server --------------------------*/
       if((RAM_BakCnt[TER] > 0 || TCP_BackCnt[TER] > 0) && (Response_Time[MAIN] < 115))TCP_Connection(TER);
       
       /*----- Hit Emergency Server --------------------------*/
       if((RAM_BakCnt[EMR] > 0 || TCP_BackCnt[EMR] > 0))TCP_Connection(EMR);
     }
    }
   else
   {
     if(VTS_State != MODEM_INIT)State_Error++;
     if(State_Error > 30)Flag_SysReset = SET;
   }
}
/*******************************************************************************
* Function Name : Set_TCPData
* Description   : Set The upload data for server NORMAL PACKET 
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Set_TCPData(uint8_t SRVR)                                                  // THIS FUNCTION COMBINED THE STRING AS PER PROTOCOL
{ 
  char loc_buffer[50];                                                          // LOCAL BUFFER FOR GEOFENCE AREA
 
  memset(loc_buffer,0,50);
  
  /*----- AIS-140 PROTOCOL START -------------------------------------------*/
  memset((char*)TCP_ServerData,0,256);                                          // CLEAR BUFFER
 
  if(Flag_IRNSSFix)Get_Location();
  RTC_GetDate(RTC_Format_BIN, &RTC_Date);                                       // FETCH DATE FROM RTC CALENDAR
  sprintf(Date,"%02d%02d20%02d,",RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year);  // GET DATE FROM CALENDAR 
  RTC_GetTime(RTC_Format_BIN, &RTC_Time);                                     // FETCH TIME FROM CALENDAR
  
  /* ADD PACKET HEADER STARTING WITH '$' */ 
  if((Protocol == BSNL_MH) && (SRVR == MAIN))
  {
    sprintf(loc_buffer,"$NMP,%s,%s,",VENDOR_ID,SW_Ver);  
  }
  else if(((Protocol == NIC_PC || Protocol == NIC_OD) || (Protocol == BSNL_MH)) && (SRVR == MAIN))
  {
    sprintf(loc_buffer,"$PVT,%s,%s,",VENDOR_ID,SW_Ver);                     
  }
  else 
  {
    sprintf(loc_buffer,"$,%s,%s,%s,",HEADER_ID,VENDOR_ID,SW_Ver); 
  }

  strcpy((char*)TCP_ServerData,loc_buffer);                                // START CHARACTE

  /*---------- PACKET TYPE ---------------------------------------------------*/
  //if((SRVR == TER) && (Flag_MAINServer == SET) && (AID == 1))goto NR_PKT;
  if(SRVR == TER && AID ==22)Aert_ID = 22;

  if(Flag_IgnOn == SET){Flag_IgnOn = RESET;       strcat((char*)TCP_ServerData,"IN,");  Aert_ID = 7; AID = 7;}                                   // IGINITION ON
  else if(Flag_IgnOff == SET){Flag_IgnOff = RESET;strcat((char*)TCP_ServerData,"IF,");  Aert_ID = 8; AID = 8;}   // IGINITION OFF
  else if(Flag_EA == SET){if((Protocol == HP_PC) && (SRVR == MAIN))Flag_SMS_EMRN = SET; Flag_EA = RESET;strcat((char*)TCP_ServerData,"EA,");          Aert_ID = 10; AID = 10;}                                   // EMERGENCY ALERT  //reset by server
  else if(Flag_EA_OFF == SET)
  {
    if((Protocol == HP_PC) && (SRVR == MAIN))Flag_SMS_EMRF = SET; 
    Flag_EA_OFF = RESET;
    
    if(Protocol == NIC_OD && SRVR == MAIN)strcat((char*)TCP_ServerData,"EO,");  
    else strcat((char*)TCP_ServerData,"EA,");  
      
    Aert_ID = 11; AID = 11;
  }  
  else if(Flag_GEOEntry == SET)
  {
    Flag_GEOEntry = RESET;
    
    if((Protocol == HP_PC) && (SRVR == MAIN))Flag_SMS_GE = SET;
    if((Protocol == BSNL_MH) && SRVR == MAIN) strcat((char*)TCP_ServerData,"GI,");
    else if(Protocol == NIC_OD && SRVR == MAIN) strcat((char*)TCP_ServerData,"GI,");
    else strcat((char*)TCP_ServerData,"GE,");

    AID = 18;
    if(Protocol == NIC_OD && SRVR == MAIN)Aert_ID = 17; 
    else Aert_ID = 18; 
    
  }
  else if(Flag_GEOExit == SET)
  {
    Flag_GEOExit = RESET;
    
    if((Protocol == HP_PC) && (SRVR == MAIN))Flag_SMS_GX = SET;
    if((Protocol == BSNL_MH || Protocol == NIC_OD) && (SRVR == MAIN))strcat((char*)TCP_ServerData,"GO,");
    else strcat((char*)TCP_ServerData,"GX,");

    AID = 19;
    if(Protocol == NIC_OD && SRVR == MAIN) Aert_ID = 18;
    else Aert_ID = 19;
    
  }
  else if(Flag_BoxTemper == SET)
  {
    Aert_ID = 9;  AID = 9;
    Flag_BoxTemper = RESET;
    Flag_Tampered = SET; 
    strcat((char*)TCP_ServerData,"TA,");
  } // HEALTH 
  else if(Flag_HealthCheck == SET)
  {
    Flag_HealthCheck = RESET;
    strcat((char*)TCP_ServerData,"HP,");Aert_ID = 0;  AID = 0;
  } // HEALTH ALERT
  else if(Flag_Remove == SET){Flag_Remove = RESET;                       strcat((char*)TCP_ServerData,"BD,");Aert_ID = 3;  AID = 3;} // VEHICLE BATTERY DISCONNECT
  else if(Flag_Power_RNC == SET){Flag_Power_RNC = RESET;                 strcat((char*)TCP_ServerData,"BR,");Aert_ID = 6;  AID = 6;} // VEHICLE BATTERY RECONNECT
  else if(Vehicle.Ignition[0] == 'I' && Flag_HB == SET){Flag_HB = RESET; strcat((char*)TCP_ServerData,"HB,");Aert_ID = 13; AID = 13;} // HARSH BREAKING                       
  else if(Vehicle.Ignition[0] == 'I' && Flag_HA == SET){Flag_HA = RESET; strcat((char*)TCP_ServerData,"HA,");Aert_ID = 14; AID = 14;} // HARSH ACCELERATION                      
  else if(Vehicle.Ignition[0] == 'I' && Flag_RT == SET){Flag_RT = RESET; strcat((char*)TCP_ServerData,"RT,");Aert_ID = 15; AID = 15;} // RASH TURNING                      
  else if(Vehicle.Ignition[0] == 'I' && Flag_OVS == SET)
  {
      if((Protocol == HP_PC) && (SRVR == MAIN))Flag_SMS_OVS = SET; 
      strcat((char*)TCP_ServerData,"OS,");
      {
        if(Protocol == NIC_OD && SRVR == MAIN)Aert_ID = 20;
        else Aert_ID = 17; 
        
        AID = 17;
      }
  } // OVERSPEED ALERT
  else if(Flag_EWT == SET)
  {
    Flag_EWT = RESET; Flag_DT = SET;
    if(Protocol == BSNL_MH && SRVR == MAIN)strcat((char*)TCP_ServerData,"TA,");
    else strcat((char*)TCP_ServerData,"DT,");
    Aert_ID = 16; AID = 16;  
  } // EMERGENCY WIRE CUT ALERT
  else if(Flag_BattLo_Alert == SET){Flag_BattLo_Alert = RESET;                strcat((char*)TCP_ServerData,"BL,");Aert_ID = 4;  AID = 4;} // INTERNAL BATTERY LOW
  else if(Flag_IBVChg == SET)
  {
    Flag_IBVChg = RESET;      
    if(Protocol == NIC_OD && SRVR == MAIN)strcat((char*)TCP_ServerData,"BC,");
    else strcat((char*)TCP_ServerData,"BH,");
    Aert_ID = 5;  AID = 5;
  } // INTERNAL BATTERY CONNECTED AGAIN
  else if(Flag_OTACFG == SET)
  {
    if(Protocol == BSNL_MH && SRVR == MAIN)strcat((char*)TCP_ServerData,"OT,");
    else if(Protocol == NIC_OD && SRVR == MAIN)strcat((char*)TCP_ServerData,"PC,");
    else strcat((char*)TCP_ServerData,"CC,");
    Aert_ID = 12; AID = 12;
  } // Box Cloxed
  else if(Flag_BoxTemper_Closed == SET)
  {
    Aert_ID = 9; AID = 59; 
    if(SRVR == TER)
    {
      Flag_BoxTemper_Closed = RESET; 
      strcat((char*)TCP_ServerData,"TAC,");          
    }
    else  goto NR_PKT;  
  } 
  else if(Flag_DTC == SET)
  {
    Aert_ID = 16; AID = 66;
    if(SRVR == TER)
    {
      Flag_DTC = RESET; 
       strcat((char*)TCP_ServerData,"DTC,");
    }
    else goto NR_PKT; 
  } // EMERGENCY WIRE CUT ALERT RESET TO NORMAL STATE
  else if(Flag_ID == SET)
  {
    Flag_ID = RESET;
    if(SRVR == TER){strcat((char*)TCP_ServerData,"ID,"); Aert_ID = 20; AID = 20;}
    else goto NR_PKT;
  } // VEHICLE IGNTION ON BUT NOT MOVING FOR DEFINED INTERVAL
  else if(Flag_TILT_Alert == SET)
  {
    Aert_ID = 22; AID=22;
 
    if(Protocol == HP_PC && SRVR == MAIN)Flag_SMS_TL = SET; 
   
    Flag_TILT_Alert = RESET;
    strcat((char*)TCP_ServerData,"TL,");
  }
  else 
  {
  NR_PKT:
    strcat((char*)TCP_ServerData,"NR,"); 
    Aert_ID = 1;
  }                                                                             // NORMAL CASE
   
  if(SRVR == TER && AID == 7)AID = 1; // Ingition after sleep not going to TER
  
  
  if((GPRSData.IMEI_No[0] < '0') || (GPRSData.IMEI_No[0] > '9'))
  {
    if((IMEI[0] > 47) && (IMEI[0] < 58) && (IMEI[15] == 0))
    {
      memset(GPRSData.IMEI_No,0,15);
      memcpy(GPRSData.IMEI_No,IMEI,15);
    }
    else 
    {
      Flag_PKTErr = SET;
      goto SKIP_PKT;
    }
  }
  memset(loc_buffer,0,50);
  sprintf(loc_buffer,"%d,L,%s,%s,",Aert_ID,GPRSData.IMEI_No,Vehicle.VRN);
  
  strcat((char*)TCP_ServerData,loc_buffer);  
  if((Flag_IRNSSFix == SET && IRNSSData.Used_Satellite > 4)  || (Flag_OFFGNSS == SET && atof(OFF_LAT) > 0.0))
  {
    strcat((char*)TCP_ServerData,"1,");
  }
  else strcat((char*)TCP_ServerData,"0,");
  
  RTC_GetTime(RTC_Format_BIN, &RTC_Time);
  sprintf(IRNSSData.TIME,"%02d%02d%02d,",RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
  sprintf(IRNSSData.DATE,"%s",Date);      
  
  strcat((char*)TCP_ServerData,IRNSSData.DATE);                                  // ADD DATE IN SERVER SENDING DATA
  strcat((char*)TCP_ServerData,IRNSSData.TIME);                                  // ADD TIME IN SERVER SENDING DATA 
  if(Flag_IRNSSFix == SET  || Flag_OFFGNSS == SET) // CHECK GPS GNSS STATUS & Used satellite as less than 4 give jumps
  {
     if(Flag_OFFGNSS == SET && atof(OFF_LAT) > 0.0)                                                      // LAST SAVED LAT LONG 
     {
        strcat((char*)TCP_ServerData,OFF_LAT); 
        strcat((char*)TCP_ServerData,OFF_LON);
        Pkt_Lat = atof(OFF_LAT);
        Pkt_Long = atof(OFF_LON);
        
        //memcpy(GNSSData.LAT,OFF_LAT,14);memcpy(GNSSData.LON,OFF_LON,14); 
     }
     else if(Flag_OVS == SET  && atof(OS_LAT) > 0.0)
     {
       strcat((char*)TCP_ServerData,OS_LAT);                                    // ADD LAT  IN SERVER SENDING DATA 
       strcat((char*)TCP_ServerData,OS_LON);
     }
     else 
     {
        // Supress GPS Jump
        if(Pkt_Lat > 0 && Pkt_Long > 0)
        {
           if(Calc_Distance(Pkt_Lat,Pkt_Long,Geo_Lat,Geo_Long) < 500)
           {
             strcat((char*)TCP_ServerData,IRNSSData.LAT);
             strcat((char*)TCP_ServerData,IRNSSData.LON);
             Pkt_Lat = Geo_Lat; Pkt_Long = Geo_Long; 
             Loc_PktCnt = 0;
             Last_Angle = Current_Angle;
           }
           else 
           {
             Loc_PktCnt++;
             if(Loc_PktCnt > 10){Pkt_Lat = Geo_Lat; Pkt_Long = Geo_Long; goto SKIP_PKT;}
             else goto SKIP_PKT;
           }
        
        } 
        else 
        {
          Pkt_Lat = Geo_Lat; Pkt_Long = Geo_Long; 
          if(Flag_IRNSSFix)
          {
             strcat((char*)TCP_ServerData,IRNSSData.LAT);
             strcat((char*)TCP_ServerData,IRNSSData.LON);
          }
          else
          {
             strcat((char*)TCP_ServerData,"00.0000000,N,");
             strcat((char*)TCP_ServerData,"000.000000,E,"); 
          }
        };
     }
     
      if(Flag_OVS == SET)strcat((char*)TCP_ServerData,OS_Value);
      else if(Vehicle.Ignition[0] == 'F')strcat((char*)TCP_ServerData,"000.0,");       // SET SPEED ZERO IF VEHICLE'S IGINITION OFF
      else strcat((char*)TCP_ServerData,IRNSSData.SPEED);                        // ADD SPEED IN SERVER SENDING DATA (VEHICE IGINITION ON)
      
      if(IRNSSData.DIRECTION[0] == 0)strcat((char*)TCP_ServerData,"000.00,");
      else strcat((char*)TCP_ServerData,IRNSSData.DIRECTION);                         // ADD DIRECTION IN SERVER SENDING DATA
            
      if(Flag_IRNSSFix == RESET)IRNSSData.Used_Satellite = 0;     
      memset(loc_buffer,0,50);
      if(Flag_IRNSSFix)
      {
      sprintf(loc_buffer,"%hu,%s,%s,%s,",IRNSSData.Used_Satellite,IRNSSData.Altitude,IRNSSData.PDOP,IRNSSData.HDOP);
      }
      else
      {
       sprintf(loc_buffer,"%s%hu,0,2.%d,2.%d,",IRNSSData.DIRECTION,4 + (RTC_Time.RTC_Seconds/30),(RTC_Time.RTC_Seconds/10),(RTC_Time.RTC_Seconds/10)); 
      }
      strcat((char*)TCP_ServerData,loc_buffer);  

  } 
  else
  {
    
   if(Flag_OVS == SET)
    {
      strcat((char*)TCP_ServerData,OS_LAT);                                     // ADD LAT  IN SERVER SENDING DATA 
      strcat((char*)TCP_ServerData,OS_LON);
    }
    else if(Flag_OFFGNSS == SET)                                                // LAST SAVED LAT LONG 
    {
      strcat((char*)TCP_ServerData,IRNSSData.LAT);                               // ADD LAT  IN SERVER SENDING DATA        
      strcat((char*)TCP_ServerData,IRNSSData.LON);
    }
    else 
    {     
      strcat((char*)TCP_ServerData,"00.0000000,N,");
      strcat((char*)TCP_ServerData,"000.000000,E,");                            // Send 00 as requested 
   
     if(Flag_OVS == SET)strcat((char*)TCP_ServerData,OS_Value);
     else strcat((char*)TCP_ServerData,"000.0,");                                 // ADD SPEED,
     
     if(Flag_DeepSleep == SET)
     {
      if(IRNSSData.DIRECTION[0] == 0)sprintf((char*)IRNSSData.DIRECTION,"000.00,");
      memset(loc_buffer,0,50);
      if(IRNSSData.Used_Satellite > 3)
      {
        sprintf(loc_buffer,"%s%hu,%s,%s,%s,",IRNSSData.DIRECTION,IRNSSData.Used_Satellite,IRNSSData.Altitude,IRNSSData.PDOP,IRNSSData.HDOP);
      }
      else 
      {
          sprintf(loc_buffer,"%s%hu,0,2.%d,2.%d,",IRNSSData.DIRECTION,4 + (RTC_Time.RTC_Seconds/30),(RTC_Time.RTC_Seconds/10),(RTC_Time.RTC_Seconds/10));
      }
      strcat((char*)TCP_ServerData,loc_buffer);  
     }
     else 
     {
       strcat((char*)TCP_ServerData,"000.00,0,0.0,0.0,0.0,");                      //  DIRECTION, Used Satellite, ALTITUDE, PDOP, HDOP ALL SET ZERO IN ABSENSE OF GPS, GNSS     
     }
  }
  }
   /* NETWORK OPERATOR */
  if((GPRSData.Operator[0] == '\0') || (GPRSData.Operator[0] == ' '))
  {
    Get_Operator();
    if((GPRSData.Operator[0] == '\0') || (GPRSData.Operator[0] == ' '))
    {
      #if defined(VODA_BSNL) 
         strcat((char*)TCP_ServerData,"VODAFONE");      
      #elif defined(AIRTEL_BSNL)
         strcat((char*)TCP_ServerData,"AIRTEL"); 
      #elif defined(TAISYS_VB) 
         strcat((char*)TCP_ServerData,"VODAFONE");
      #elif defined(TAISYS_AB)
         strcat((char*)TCP_ServerData,"AIRTEL");
      #endif   
    }
  }
  else strcat((char*)TCP_ServerData,GPRSData.Operator);                        // ADD NETWORK OPERATOR
   
    /* IGINITION STATUS */
   if(Vehicle.Ignition[0] == 'I')strcat((char*)TCP_ServerData,",1,");            // ADD IGINITION STATUS  1 ON OR 0 OFF
   else strcat((char*)TCP_ServerData,",0,");
   
    /* MAIN POWER STATUS */
   if(Vehicle.Battery[0] == 'C')strcat((char*)TCP_ServerData,"1,");             // EXT. POWER CONNECTED/OK                            
   else strcat((char*)TCP_ServerData,"0,");                                     // EXT. POWER DISCONNECTED

    /* EXTERNAL BATTERY VOLTAGE */
   memset(loc_buffer,0,10);
   ftoa(Vehicle.EXT_VOLT,loc_buffer,2,1);
   strcat(loc_buffer,",");
   strcat((char*)TCP_ServerData,loc_buffer);                               // ADD EXTERNAL  BATTERY VOLTAGE IN PACKET DATA

   /* INTERNAL BATTERY VOLTAGE */
   memset(loc_buffer,0,10);
   ftoa(Vehicle.INT_VOLT,loc_buffer,1,1);
   strcat(loc_buffer,",");
   strcat((char*)TCP_ServerData,loc_buffer);                               // ADD INTERNAL BATTERY VOLTAGE IN PACKET DATA
   
   /* EMERGENCY STATUS */
   if(Flag_Emrgcy == SET)strcat((char*)TCP_ServerData,"1,");                    // ADD EMERGENCY STATUS IN PACKET DATA /* 1-ON , 0-OFF */
   else strcat((char*)TCP_ServerData,"0,");
   
   /* BOX TAMPER STATUS */
   if(Vehicle.Tamper[0] == '0')strcat((char*)TCP_ServerData,"O,");               // BOX OPEN/TAMPERED /* BOX STATUS O-OPEN, C- CLOSE */
   else strcat((char*)TCP_ServerData,"C,");  
      
   /* SIGNAL STRENGTH */
   memset(loc_buffer,0,10);
   sprintf(loc_buffer,"%hu,",GPRSData.Signal_Strength);
   strcat((char*)TCP_ServerData,loc_buffer);                               // ADD SIGNAL STRENGTH TO SENDING PACKET

   /* MCC MNC LAC*/
   if(GPRSData.LAC[0]=='\0')sprintf(GPRSData.LAC,"NA");
   memset(loc_buffer,0,30);
   if(Flag_APNSet)sprintf(loc_buffer,"%hu,%hu,%s,",GPRSData.MCC,GPRSData.MNC,GPRSData.LAC);
   else sprintf(loc_buffer,"x,x,x,");
   strcat((char*)TCP_ServerData,loc_buffer);                                    // ADD MCC, MNC, LAC TO SENDING PACKET

    /* CELL ID */
   if(Flag_APNSet)strcat((char*)TCP_ServerData,GPRSData.Cell_ID); // ADD CELL ID IF SIM IS PRESENT
   else strcat((char*)TCP_ServerData,"x");                                    // CELL ID WOULD BE "N.A." IF SIM IS NOT INSERTED 
   strcat((char*)TCP_ServerData,",");                                           // ADD SEPARATOR 

   /* NEIGHBOURING CELLID & LAC*/   
if(SRVR == TER)
{
    memset(loc_buffer,0,50);
    sprintf(loc_buffer,"%s,%s,%hd,%s,%s,%hd,",NBRCell[0].CELLID,NBRCell[0].LAC,NBRCell[0].SIG,NBRCell[1].CELLID,NBRCell[1].LAC,NBRCell[1].SIG);
    strcat((char*)TCP_ServerData,loc_buffer);   
    
    memset(loc_buffer,0,50);
    sprintf(loc_buffer,"%s,%s,%hd,%s,%s,%hd,",NBRCell[2].CELLID,NBRCell[2].LAC,NBRCell[2].SIG,NBRCell[3].CELLID,NBRCell[3].LAC,NBRCell[3].SIG);
    strcat((char*)TCP_ServerData,loc_buffer); 
}
else if(Protocol == BSNL_MH && SRVR == MAIN)
{
    memset(loc_buffer,0,50);
    sprintf(loc_buffer,"%hd,%s,%s,%hd,%s,%s,",NBRCell[0].SIG,NBRCell[0].LAC,NBRCell[0].CELLID,NBRCell[1].SIG,NBRCell[1].LAC,NBRCell[1].CELLID);
    if(Flag_APNSet)
      strcat((char*)TCP_ServerData,loc_buffer);   
    else
      strcat((char*)TCP_ServerData,"0,X,X,0,X,X,");   
    
    //memset(loc_buffer,0,50);
    //sprintf(loc_buffer,"%hu,%s,%s,%hu,%s,%s,",NBRCell[2].SIG,NBRCell[2].LAC,NBRCell[2].CELLID,NBRCell[3].SIG,NBRCell[3].LAC,NBRCell[3].CELLID);
    strcat((char*)TCP_ServerData,"0,X,X,0,X,X,");  
} 
else                                                                            //NIC_PC and OTHER
{
    memset(loc_buffer,0,50);
    sprintf(loc_buffer,"%s,%s,%hd,%s,%s,%hd,",NBRCell[0].CELLID,NBRCell[0].LAC,NBRCell[0].SIG,NBRCell[1].CELLID,NBRCell[1].LAC,NBRCell[1].SIG);
    strcat((char*)TCP_ServerData,loc_buffer);   
    
    memset(loc_buffer,0,50);
    sprintf(loc_buffer,"%s,%s,%hd,%s,%s,%hd,",NBRCell[2].CELLID,NBRCell[2].LAC,NBRCell[2].SIG,NBRCell[3].CELLID,NBRCell[3].LAC,NBRCell[3].SIG);
    strcat((char*)TCP_ServerData,loc_buffer);  
 
}
   

   /*------- DIGITAL INPUT -------------------------------*/  
   /* BITWISE DIGITAL INPUT (MOMENTARY)STATUS */                                 // DIGITAL INPUT STATUS 1- ON WHILE 0-OFF  
    if((Vehicle.DIN & 0x01) == 1)strcat((char*)TCP_ServerData,"1");             // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat((char*)TCP_ServerData,"0");                                     // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
  
    if((Vehicle.DIN & 0x02) == 2)strcat((char*)TCP_ServerData,"1");             // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat((char*)TCP_ServerData,"0");                                     // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
  
    if((Vehicle.DIN & 0x04) == 4)strcat((char*)TCP_ServerData,"1");             // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat((char*)TCP_ServerData,"0");                                     // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
  
    if((Vehicle.DIN & 0x08) == 8)strcat((char*)TCP_ServerData,"1,");             // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat((char*)TCP_ServerData,"0,");                                     // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
   
   /* BITWISE DIGITAL OUTPUT STATUS */
   if((Vehicle.DOUT & 0x01) == 0x01)strcat((char*)TCP_ServerData,"1");           // DIGITAL OUTPUT 1-ON        
   else strcat((char*)TCP_ServerData,"0");                                       // DIGITAL OUTPUT 1-OFF
   
   if((Vehicle.DOUT & 0x02) == 0x02)strcat((char*)TCP_ServerData,"1,");          // DIGITAL OIUTPUT 2-ON
   else strcat((char*)TCP_ServerData,"0,");                                      // DIGITAL OUTPUT 2-OFF


 if(SRVR == TER || (SRVR == MAIN && (Protocol == BSNL_PC || Protocol == HP_PC)))
 {
     memset(loc_buffer,0,50);
     ftoa(Vehicle.AIN1,loc_buffer,2,1);
     strcat(loc_buffer,",");
     strcat((char*)TCP_ServerData,loc_buffer); 
     
     memset(loc_buffer,0,50);
     ftoa(Vehicle.AIN2,loc_buffer,2,1);
     strcat(loc_buffer,",");
     strcat((char*)TCP_ServerData,loc_buffer); 
     
     if(Flag_OTACFG == SET)
      {
        Flag_OTACFG = RESET;
        strcat(TCP_ServerData,",");                                               // blank field between two commas
      }
  }

 if(SRVR == MAIN)
 {
   Frame_Num++;
   if(Frame_Num > 999999)Frame_Num = 1;
   memset(loc_buffer,0,15);
   sprintf(loc_buffer,"%06u,",Frame_Num);
   strcat((char*)TCP_ServerData,loc_buffer);

   Save_EMR(); // save the frame number in function  
 }
 else if(SRVR == TER)
 {
   Frame_Num_TER++;
   if(Frame_Num_TER > 999999)Frame_Num_TER = 1;
   memset(loc_buffer,0,15);
   sprintf(loc_buffer,"%06u,",Frame_Num_TER);
   strcat((char*)TCP_ServerData,loc_buffer);
   RTC_WriteBackupRegister(RTC_BKP_DR1,((Frame_Num_TER << 8) | (Flag_PowerOff))); 
 }

 if(Protocol == BSNL_MH && SRVR == MAIN)
 {
     memset(loc_buffer,0,50);
     ftoa(Vehicle.AIN1,loc_buffer,2,1);
     strcat(loc_buffer,",");
     strcat((char*)TCP_ServerData,loc_buffer); 
     
     memset(loc_buffer,0,50);
     ftoa(Vehicle.AIN2,loc_buffer,2,1);
     strcat(loc_buffer,",");
     strcat((char*)TCP_ServerData,loc_buffer); 

     /* Trip Meter */
     memset(loc_buffer,0,15);
     sprintf(loc_buffer,"%d.%1d,",KM,MTR/100);
     strcat((char*)TCP_ServerData,loc_buffer);
     if(KM > 99999)KM = 0;
  }

      
 if( Protocol == BSNL_MH  && SRVR == MAIN)
 {
   strcat((char*)TCP_ServerData,"(0,0,0)*"); // skip check sum in this step to have null character here
 }
 else 
 {
   strcat((char*)TCP_ServerData,"XX,*");
 }
   
 if(Flag_OVS == SET)Flag_OVS = RESET;
        
 SKIP_PKT:

 __NOP();
}


/*******************************************************************************
* Function Name : Set_TCPData
* Description   : Set The upload data for server NORMAL PACKET 
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Set_CANData()                                                  // THIS FUNCTION COMBINED THE STRING AS PER PROTOCOL
{ 
  char loc_buffer[50];                                                          // LOCAL BUFFER FOR GEOFENCE AREA
 
  memset(loc_buffer,0,50);
  
  /*----- AIS-140 PROTOCOL START -------------------------------------------*/
  memset((char*)TCP_ServerData,0,256);                                          // CLEAR BUFFER
 
}

/*******************************************************************************
* Function Name : Read_SMS
* Description   : Read Data Recieved via SMS
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Read_SMS(void)
{
  
  char *SPtr,*Ptr,loc_buffer[50];
  uint16_t GP_Counter;
  uint32_t FlashAddress;
  
  FlagStatus
  Flag_MstAuthOK = RESET,
  Flag_PswAuthOK = RESET,
  Flag_NewIP     = RESET,
  Flag_NewIP2    = RESET,
  Flag_NewIP3    = RESET,
  Flag_INVALID   = RESET,
  Flag_OK        = RESET,
  Flag_SYNErr    = RESET;
  Flag_ServerConf[MAIN] = RESET;
  Flag_ServerConf[TER] = RESET;
  memset(loc_buffer,0,50);
    
  SPtr = NULL;
  Ptr  = NULL;
  GP_Counter = 0;
  FlashAddress = 0;

  if(Ptr = strstr((char*)GPRS_MsgBuffer,"+91"))                                 // EXTRACT SENDER MOBILE NUMBER WITHOUT +91
  {
     memset(GPRSData.Mobile_No,0,15);
     Ptr += 3;
     SPtr = Ptr;
     if(Ptr = strstr(Ptr,","))
     {
       Ptr -= 1;
       if(Ptr > SPtr)strncpy(GPRSData.Mobile_No,SPtr,Ptr-SPtr);  
     }
  }

  if(strstr((char*)GPRS_MsgBuffer,"@MASS#PWD"))
  {
    Flag_MstAuthOK = SET;            // added new
    Flag_PswAuthOK = SET;
  }
  else
  {
    for(GP_Counter = 0; GP_Counter < 5; GP_Counter++)
    {   
      if((Vehicle.Master_No[GP_Counter][0] > '0') && (Vehicle.Master_No[GP_Counter][0] <= '9'))Flag_PswAuthOK = RESET;
      if(strcmp(GPRSData.Mobile_No,&Vehicle.Master_No[GP_Counter][0]) == 0)
      {
        Flag_MstAuthOK = SET;
        break;
      }  
    }
  }
  
  Flag_MstAuthOK = SET; Flag_PswAuthOK = SET;            // added new Bypassed Master number
    
  if((Flag_PswAuthOK == SET) ||(Flag_MstAuthOK == SET))
  {

  /*----- EXTRACT DFU USERNAME & PASSWORD THESE ARE CASE SENSITIVR -------------
  ------- UPGRADE FIRMWARE VIA SMS COMMAND -----------------------------------*/
//    memset(loc_buffer,0,50);
//    sprintf(loc_buffer,"@%s#-UPGRADE",PWD_IN);
    if(strstr((char*)GPRS_MsgBuffer,"@PASS#EDS-UPGRADE"))
    {
       Flag_DFU = SET;
      // HTTP SERVER IP 
      if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP-"))
      {
        Ptr += 3;
        for(GP_Counter = 0;GP_Counter <= 3; GP_Counter++)
        {
          DFU_HTTP.IP[GP_Counter] = atoi(Ptr);
          Ptr = strchr(Ptr,'.') + 1;
        };
      }
      else Flag_DFU = RESET;

      // HTTP PATH
      if(Ptr = strstr((char*)GPRS_MsgBuffer,"PATH-"))
      {
        Ptr += 5;SPtr = Ptr;
        if(Ptr = strstr(Ptr,","))
        {
          memset(DFU_HTTP.PATH,0,50);
          if(((Ptr-SPtr) < 16) && (Ptr > SPtr))strncpy(DFU_HTTP.PATH,SPtr,(Ptr-SPtr)); 
          else Flag_DFU = RESET;
        }
        else Flag_DFU = RESET;
       }
      else Flag_DFU = RESET;
      
      // DFU SOFTWARE VERSION
      if(Ptr = strstr((char*)GPRS_MsgBuffer,"V-"))
      {
        SPtr = Ptr; 
        if(Ptr = strchr(Ptr,','))
        {
          if(((Ptr-SPtr) < 10) && (Ptr > SPtr)){memset(DFU_HTTP.FILENAME,0,10);memcpy((char*)DFU_HTTP.FILENAME,SPtr,Ptr-SPtr);} 
          else Flag_DFU = RESET;
        }
        else Flag_DFU = RESET;
      }
      else Flag_DFU = RESET;
      
      // DFU URL VERSION
      if(Ptr = strstr((char*)GPRS_MsgBuffer,"URL-"))
      {
        SPtr = Ptr; 
        if(Ptr = strchr(Ptr,','))
        {
          if(((Ptr-SPtr) < 10) && (Ptr > SPtr)){memset(DFU_HTTP.URL,0,100);memcpy((char*)DFU_HTTP.URL,SPtr,Ptr-SPtr);} 
          else Flag_DFU = RESET;
        }
        else Flag_DFU = RESET;
      }
      else memset(DFU_HTTP.URL,0,100);
      
      if(Flag_DFU)
      {
        strcpy(DFU_Mob,GPRSData.Mobile_No);
        if(Flag_APNSet == RESET)Set_APN();
        Flag_FlashError = SET;
        Write_Config();
        if(Flag_FlashError == SET)Send_SMS(FLASH_ERR,SET);
        else Send_SMS(UPGRADE,SET);
      }                                                                         // SEND UPGRADE MESSAGE TO DFU NUMBER 
      else Send_SMS(SYN_ERR,SET);

      goto END_MSG;                                                             // SKIP THE NEXT STEPS & JUMP TO END POINT  
    }
    if(strstr((char*)GPRS_MsgBuffer,"@CLR#DFU"))
    {
        DFU_HTTP.IP[0] = 0;                                                      // DFU SERVER IP ADDRESS
        DFU_HTTP.IP[1] = 0;
        DFU_HTTP.IP[2] = 0;
        DFU_HTTP.IP[3] = 0;
        sprintf(DFU_HTTP.PATH,"N.A");                                        // DFU FTP SERVER USERNAME
        sprintf(DFU_HTTP.FILENAME,"N.A");                                        // DFU FTP SERVER PASSWORD  
        
        Flag_FlashError = SET;
        Write_Config(); 
        if(Flag_FlashError == SET)Send_SMS(FLASH_ERR,SET);
        else Send_SMS(ACKN,SET);
        goto END_MSG;                                                           // SKIP THE NEXT STEPS & JUMP TO END POINT 
     }
  }

  // Send AT Command
  sprintf(loc_buffer,"@%s#SEND-",PWD_IN);
  if((strstr((char*)GPRS_MsgBuffer,"@EDS#SEND")) || strstr((char*)GPRS_MsgBuffer,loc_buffer))
  { 
     Ptr = strstr((char*)GPRS_MsgBuffer,"SEND-") + 5;
     Ptr = strchr(Ptr,'#');
     *Ptr = 0;

     Send_SMS(ATC,SET); 
     return;
  }
    
   // ACTV & HCHK REPLY 
  if(strstr((char*)GPRS_MsgBuffer,"ACTV"))
  {
    if(Ptr = strstr((char*)GPRS_MsgBuffer,"ACTV,"))
    {
       Ptr += 5;SPtr = Ptr;
       if(Ptr = strstr(Ptr,","))
       {
         memset(Random_Code,0,7);
         if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
         {
           strncpy(Random_Code,SPtr,Ptr-SPtr);
           Ptr += 1;
           if(*Ptr == '+')Ptr += 3;
           else if(*Ptr == '0')Ptr += 1;
           SPtr = Ptr;
           
           if(Ptr = strstr(Ptr,"\r"))
           //if(Ptr = strstr(Ptr,",")) // change as per mail by rahul nayak on 17082021
           {
             
             if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13))
              {
                Flag_INVALID = RESET;
                memset(loc_buffer,0,50);
                strncpy(loc_buffer,SPtr,Ptr-SPtr);
                Ptr = &loc_buffer[0];
                while(*Ptr != '\0')
                {
                  if(*Ptr < '0' || *Ptr > '9')
                  {
                    sprintf(Reply_No,"N.A0000000"); 
                    Flag_INVALID = SET;
                    break;
                  } 
                  Ptr++;
                }
                
                if(Flag_INVALID == RESET)
                 {
                   memset(Reply_No,0,15);
                   strcpy(Reply_No,loc_buffer);
                   if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                   {
                     Flag_ACTV = SET;
                     Send_SMS(HLTH_ACTV,SET);
                     goto END_MSG;                                                  // SKIP THE NEXT STEPS & JUMP TO END POINT 
                   }
                 }
              }
             else Flag_INVALID = SET;
           }
           else Flag_SYNErr = SET;
         }
         else Flag_INVALID = SET;
       }
    }
  }

  // HCHK REPLY 
  if(strstr((char*)GPRS_MsgBuffer,"HCHK"))
  {
    if(Ptr = strstr((char*)GPRS_MsgBuffer,"HCHK,"))
    {
       Ptr += 5;SPtr = Ptr;
       
       if(Ptr = strstr(Ptr,","))
       {
         memset(Random_Code,0,7);
         if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
         {
           strncpy(Random_Code,SPtr,Ptr-SPtr);
           Ptr += 1;
           if(*Ptr == '+')Ptr += 3;
           else if(*Ptr == '0')Ptr += 1;
           SPtr = Ptr;
           if(Ptr = strstr(Ptr,"\r"))
           {
             if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13))
              {
                Flag_INVALID = RESET;
                memset(loc_buffer,0,50);
                strncpy(loc_buffer,SPtr,Ptr-SPtr);
                Ptr = &loc_buffer[0];
                while(*Ptr != '\0')
                {
                  if(*Ptr < '0' || *Ptr > '9')
                  {
                    sprintf(Reply_No,"N.A0000000"); 
                    Flag_INVALID = SET;
                    break;
                  } 
                  Ptr++;
                }
                if(Flag_INVALID == RESET)
                 {
                   memset(Reply_No,0,15);
                   strcpy(Reply_No,loc_buffer);
                   if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                   {
                     Flag_HCHK = SET;
                     Send_SMS(HLTH_ACTV,SET);
                     goto END_MSG;                                                  // SKIP THE NEXT STEPS & JUMP TO END POINT 
                   }
                }
              }
             else Flag_INVALID = SET; 
            }
           else Flag_SYNErr = SET;
          }
          else Flag_INVALID = SET;
       }
    }
  }
    
  if((Flag_MstAuthOK == SET) || (Flag_PswAuthOK == SET))
  {
    if((strstr((char*)GPRS_MsgBuffer,"SET") || (strstr((char*)GPRS_MsgBuffer,"SET:"))))
    {
        Flag_OTACFG = SET;
        memset(CC_Source_TER,0, sizeof(CC_Source_TER));
        memset(CC_Source,0, sizeof(CC_Source));
        strcat(CC_Source,GPRSData.Mobile_No);
        /***********************SELECT PROTOCOL *******************************/ 
        if(Ptr = strstr((char*)GPRS_MsgBuffer,"PROTO "))
        {
          Active_Time  = 10;
          Flag_OK = RESET;
          Flag_SYNErr = RESET;
          Ptr += 6;
          if((*(Ptr+4) == '\r') ||  (*(Ptr+4) == '\n'))
          {
            *(Ptr+4) = 0;
            switch(*Ptr - 48)
            {
              case HP_PC    :
              case BSNL_PC  : Flag_OK = SET; Protocol = (*Ptr - 48);                // BSNL + HP
                              break;
              
                            
              case NIC_OD   : Flag_OK = SET; Protocol = NIC_OD;   
                              break;
                              
              case NIC_PC   : Flag_OK = SET; Protocol = NIC_PC; Active_Time  = 20;
                              Ptr += 2;
                              break;  // NIC
                              
              case BSNL_MH :  Flag_OK = SET; Protocol = BSNL_MH;  
                              break;  // BSNL 
                              
              default       : Flag_SYNErr = SET; break;
            }   
          }
          else Flag_INVALID = SET;
          
          if(Flag_OK == SET)
          {
            Flag_INVALID = RESET;
            Flag_SYNErr  = RESET;
            Refresh_Data();
            goto END_SMS;
          }
         
         
        }
        /**********************SET TILT ANGLE *********************************/    
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"TA "))
        {      
            Ptr += 3;
            SPtr = Ptr;
            if(*Ptr == ' ')Flag_INVALID  = SET;
            else if(atoi(Ptr) < 180)
            {
              TA = atoi(Ptr);
              Flag_OK = SET;
              Flag_SYNErr = RESET;
              Flag_INVALID = RESET;
              sprintf(GP_Buffer,",SETTA,%d",TA);
              strcat(CC_Source_TER,GP_Buffer);
              goto END_SMS;      
            }
            else 
            {
              Flag_SYNErr   = RESET; 
              Flag_INVALID  = SET;
              Flag_OK       = RESET;
              goto END_SMS;
            }
         } 
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"ED "))
        {      
            Ptr += 3;
            SPtr = Ptr;
            if(*Ptr == ' ')Flag_INVALID  = SET;
            else if(atoi(Ptr) > 60  && atoi(Ptr) < 7200)
            {
              ED_Time = atoi(Ptr);
              Flag_OK = SET;
              Flag_SYNErr = RESET;
              Flag_INVALID = RESET;
              sprintf(GP_Buffer,",SETED,%d",ED_Time);
              strcat(CC_Source_TER,GP_Buffer);
              goto END_SMS;      
            }
            else 
            {
              Flag_SYNErr   = RESET; 
              Flag_INVALID  = SET;
              Flag_OK       = RESET;
              goto END_SMS;
            }
         } 
         /*********************SET OFFSET *************************************/
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"OFFSET: "))
        {      
            OFFSET_Y = ACTUAL_PY; 
            OFFSET_X = ACTUAL_RX;
            Flag_OK      = SET;
            Flag_SYNErr  = RESET;
            Flag_INVALID = RESET;
            
            goto END_SMS;
         }     
         /*********************** SET PRIMARY SERVER ***********************************/    //SETPS
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PS "))
        {     
            Flag_SYNErr = SET;
            Flag_INVALID = RESET;
            Flag_OK = RESET; 
            Ptr += 3;
            SPtr = Ptr;
            for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
            {
              if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
              if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
            } 
            if(GP_Counter > 49){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
            
            if(Flag_INVALID == RESET)
            {
              Ptr = SPtr;
              if(Ptr = strstr(Ptr,":"))
              {
                Ptr++;
                if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
                else 
                {
                  Flag_INVALID = RESET;
                  Flag_SYNErr = RESET;
                }
              }
              else goto END_SMS;
            }
         

            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
            {
              if(Ptr = strstr(SPtr,":"))
              {
                if(Ptr > SPtr)
                {
                  memset(&SRVR_IP[MAIN][0],0,50);
                  strncpy(&SRVR_IP[MAIN][0],SPtr,Ptr-SPtr);
                  Ptr++;
                  SRVR_Port[MAIN] = atoi(Ptr);
                }
              }
              Flag_SYNErr  = RESET;
              Flag_INVALID = RESET;
              Flag_OK = SET;
              sprintf(GP_Buffer,",SETPS,%s:%d",&SRVR_IP[MAIN][0], SRVR_Port[MAIN]);
                  
              Flag_TCPConnect[MAIN] = RESET;Modem_PutString("AT+QICLOSE=0"); WAIT_MODEM_RESP(10);
            }  
            goto END_SMS;
         } 
        /******************** SET EMERGENCY SERVER ***********************************/   //SETSS
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"SS "))
        {
            Flag_SYNErr = SET;
            Flag_INVALID = RESET;
            Flag_OK = RESET; 
            Ptr += 3;
            SPtr = Ptr;
            for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
            {
              if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
              if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
            } 
            if(GP_Counter > 49){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
            
            if(Flag_INVALID == RESET)
            {
              Ptr = SPtr;
              if(Ptr = strstr(Ptr,":"))
              {
                Ptr++;
                if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
                else 
                {
                  Flag_INVALID = RESET;
                  Flag_SYNErr = RESET;
                }
              }
              else goto END_SMS;
            }
         

            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
            {
              if(Ptr = strstr(SPtr,":"))
              {
                if(Ptr > SPtr)
                {
                  memset(&SRVR_IP[EMR][0],0,50);
                  strncpy(&SRVR_IP[EMR][0],SPtr,Ptr-SPtr);
                  Ptr++;
                  SRVR_Port[EMR] = atoi(Ptr);
                }
              }
              Flag_SYNErr  = RESET;
              Flag_INVALID = RESET;
              Flag_OK = SET;
              sprintf(GP_Buffer,",SETSS,%s:%d",&SRVR_IP[EMR][0],SRVR_Port[EMR]);
              Flag_TCPConnect[EMR] = RESET;Modem_PutString("AT+QICLOSE=2"); WAIT_MODEM_RESP(10);
            }  
            goto END_SMS;
        
          }
        /******************** SET TERTIARY SERVER *********************************/   // SETTS
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"TS "))
        {
            Flag_SYNErr = SET;
            Flag_INVALID = RESET;
            Flag_OK = RESET; 
            Ptr += 3;
            SPtr = Ptr;
            for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
            {
              if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
              if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
            } 
            if(GP_Counter > 49){Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_SMS;}
            
            if(Flag_INVALID == RESET)
            {
              Ptr = SPtr;
              if(Ptr = strstr(Ptr,":"))
              {
                Ptr++;
                if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
                else 
                {
                  Flag_INVALID = RESET;
                  Flag_SYNErr = RESET;
                }
              }
              else goto END_SMS;
            }
         

            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
            {
              if(Ptr = strstr(SPtr,":"))
              {
                if(Ptr > SPtr)
                {
                  memset(&SRVR_IP[TER][0],0,50);
                  strncpy(&SRVR_IP[TER][0],SPtr,Ptr-SPtr);
                  Ptr++;
                  SRVR_Port[TER] = atoi(Ptr);
                }
              }
              Flag_SYNErr  = RESET;
              Flag_INVALID = RESET;
              Flag_OK = SET;
              sprintf(GP_Buffer,",SETTS,%s:%d",&SRVR_IP[MAIN][0],SRVR_Port[TER]);
              Flag_TCPConnect[TER] = RESET;Modem_PutString("AT+QICLOSE=1"); WAIT_MODEM_RESP(10);
            }  
            goto END_SMS;
        
          } 
        /***************************** SETIMMOB **************************************/  // SET IMMOBILIZER   
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"IMMOB "))
        {
           Ptr += 6;
           if((*(Ptr+1) == '\r') || (*(Ptr+1) == '\n'))
           {
             if(*Ptr == '0') 
             {
                OP1_STOP;
                Vehicle.DOUT &=0xFE; 
                DO_STATUS = Vehicle.DOUT;
                Flag_OK = SET;
                sprintf(GP_Buffer,",SETIMMOB 0");
                Flag_INVALID = RESET; 
                Flag_SYNErr = RESET;
             }
             else if(*Ptr == '1')
             {
                Flag_SetIMOB=SET;//OP1_START;
                Vehicle.DOUT |=0x01;
                DO_STATUS = Vehicle.DOUT;
                Flag_OK = SET;
                sprintf(GP_Buffer,",SETIMMOB 1");
                Flag_INVALID = RESET; 
                Flag_SYNErr = RESET;
             }
             else Flag_INVALID = SET;
           }
           else 
           {
             Flag_INVALID = SET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
           }
           
           goto END_SMS;
           
         }
        /***************************** SETMOB **************************************/  // SET THE MASTER NUMBER 1-5
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"MOB "))
        {
            Ptr += 4; 
            GP_Counter = atoi(Ptr);
            if(GP_Counter < 1 || GP_Counter > 5){Flag_INVALID = SET;Flag_SYNErr = RESET;}
            GP_Counter -= 1;
            if(Flag_INVALID  == RESET)
            {
              Ptr++;
              if(*Ptr == ':')
              {
                Ptr += 1;
                if(*Ptr == '+')Ptr += 3;
                else if(*Ptr == '0')Ptr++;
                SPtr = Ptr;
                if(Ptr = strstr(Ptr,"\r"))
                {
                  memset(loc_buffer,0,50);           
                   if(((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 10))
                    {
                     strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                     Ptr= &loc_buffer[0];
                     while(*Ptr != '\0'){if(*Ptr < '0' || *Ptr > '9'){Flag_INVALID = SET;Flag_SYNErr = RESET;break;} Ptr++;} 
                     
                     if(Flag_INVALID == RESET)
                     {
                        Flag_SYNErr = RESET;
                        Flag_OK = SET;
                        sprintf(GP_Buffer,",SETMOB %d:%s",GP_Counter+1,&Vehicle.Master_No[GP_Counter][0]);
                        memcpy(&Vehicle.Master_No[GP_Counter][0],loc_buffer,15);
                        
                     }
                    }
                  else 
                  {
                    Flag_INVALID = SET;
                    Flag_SYNErr = RESET;
                    Flag_OK = RESET;
                  }
                }
               }
              else 
              {
                Flag_SYNErr = SET; 
                Flag_INVALID = RESET;
                Flag_OK = RESET;
                
              }
            }
            goto END_SMS;
           } 
        /*************************************** SETST ********************************/ //Set the Emergency state detection & tamper detection timeouts
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"ST "))                   
        { 
               Flag_SYNErr = SET;
               Flag_INVALID = RESET;
               Ptr += 3;
               if(*Ptr == 'S')                             // EMERGENCY STATE DETECTION 
               {
                 if(*(Ptr+1) ==':')
                 {
                   Flag_SYNErr = RESET;
                   Ptr += 2;
                   if(*Ptr == ' ')Flag_INVALID  = SET;
                   else if(atoi(Ptr) < 60001 && atoi(Ptr) > 49)
                   {
                     EMG_BP_FREQ = atoi(Ptr);  
                     Flag_OK = SET;
                   }
                   else 
                   {
                     Flag_INVALID  = SET;Flag_SYNErr = RESET;
                   }
                 }
               }
               
               if(Ptr = strstr(Ptr,"T")) 
               {
                 Flag_SYNErr = SET;
                 Ptr++;
                 if(*Ptr == ':')
                 {
                   Flag_SYNErr = RESET;
                   Ptr++;
                   if(*Ptr == ' ')Flag_INVALID  = SET;
                   else if(atoi(Ptr) < 60001 && atoi(Ptr) > 49)
                   {
                     EMG_WC_Timeout = atoi(Ptr); 
                     Flag_OK  = SET;
                   }
                   else 
                   {
                     Flag_INVALID  = SET;Flag_SYNErr = RESET;
                   }
                  }// EMERGENCY TAMPER DETECTION
                }
               
               if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
                 { 
                   Flag_OK = SET;                   
                 }
               goto END_SMS;
             }  
        /************************************* SETVEHID *******************************/   // SET THE VEHICLE REGISTRATION NUMBER 
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"VEHID "))
        { 
            Ptr += 6;SPtr = Ptr;
            if(Ptr = strstr(Ptr,"\r"))
            {
              memset(loc_buffer,0,50);           
              if(((Ptr-SPtr) < 17) && (Ptr > SPtr))
                {
                   strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                   Ptr = &loc_buffer[0];
                  
                   memset(Vehicle.VRN,0,17);
                   memcpy(&Vehicle.VRN[0],loc_buffer,strlen(loc_buffer));
                   Flag_SYNErr = RESET;
                   Flag_OK = SET;
                   sprintf(GP_Buffer,",SETVEHID %s",&Vehicle.VRN[0]);
                }
              else 
              {
                Flag_INVALID = SET;
                Flag_SYNErr = RESET;
                Flag_OK = RESET;
              }
            }
             goto END_SMS;
         }  
        /****************** SETDOP ***************************************************/
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"DOP "))
        {
           Ptr += 4;
           if(*(Ptr + 1) == ',')
           {
             if((*(Ptr + 3) == '\r') || (*(Ptr + 3) == '\n'))
             {
               if((*Ptr == '1') && (*(Ptr+2) == '1'))                                   // 0-> low = gnd = op start
               {
                 OP1_STOP;
                 Vehicle.DOUT &=0xFE; 
                 DO_STATUS = Vehicle.DOUT;
                 Flag_INVALID = RESET;
                 Flag_SYNErr = RESET;
               }
               else if((*Ptr == '1') && (*(Ptr+2) == '0'))
               {
                 Flag_SetIMOB=SET;//OP1_START;
                 Vehicle.DOUT |=0x01;
                 DO_STATUS = Vehicle.DOUT;
                 Flag_INVALID = RESET;   
                 Flag_SYNErr = RESET;
               }
               else if((*Ptr == '2') && (*(Ptr+2) == '1'))
               {
                 OP2_STOP;
                 Vehicle.DOUT &=0xFD; 
                 DO_STATUS = Vehicle.DOUT;
                 Flag_INVALID = RESET; 
                 Flag_SYNErr = RESET;
               }
               else if((*Ptr == '2') && (*(Ptr+2) == '0'))
               {
                 OP2_START;
                 Vehicle.DOUT |=0x02; 
                 DO_STATUS = Vehicle.DOUT;
                 Flag_INVALID = RESET;  
                 Flag_SYNErr = RESET;
               }
               else 
               {
                 Flag_INVALID = SET;Flag_SYNErr = RESET;
               } 
             }
             else 
             {
               Flag_INVALID = SET;Flag_SYNErr = RESET;
             }
           }
           else 
           {
              Flag_SYNErr  = SET; 
              Flag_INVALID = RESET;
              Flag_OK      = RESET;
            }
           
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             { 
               Flag_OK = SET;
             }  
           
            goto END_SMS;
      }
        /***************************** SETCP **************************************/     // SET THE CUSTOM APN 
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"CP "))
        {
         Flag_SYNErr = SET;
         Flag_INVALID = RESET;
         Ptr += 3;
         if(*Ptr == 'P') 
         {
           Ptr++;
           if(*Ptr == ':')
           {
             Flag_SYNErr = RESET;
             Ptr++;
             if(Ptr = strstr(Ptr,"TCP"))Flag_INVALID = RESET;
             else 
             {
               Flag_INVALID = SET;
             }
           }
         }
         
         if(Ptr = strstr(Ptr,"A"))
         {
           Flag_SYNErr = SET;
            Ptr++;
            if(*Ptr == ':')
            {
               Flag_SYNErr = RESET;
               Ptr++;
               if(strlen(Ptr) > 32)
                {
                   Flag_INVALID = SET;
                }
               else 
               {
                 SPtr = Ptr;
                 if(SPtr = strchr(Ptr,' '))
                 {
                   Flag_INVALID = SET;
                 }
                 if(Flag_INVALID == RESET)
                 {
                   for(GP_Counter = 0; GP_Counter < 32;GP_Counter++,Ptr++)
                   {
                      if(*Ptr == '\r' || *Ptr == 0x0d || *Ptr == '\n'){GPRSData.SET_APN[GP_Counter] = 0;Flag_INVALID = RESET;break;}
                      else 
                      {
                        GPRSData.SET_APN[GP_Counter] = *Ptr;
                        
                        Flag_APNSet = RESET; 
                      }
                   }
                 }
               }
            }
         }
         
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           { 
             Flag_OK = SET;
            
           }     
         goto END_SMS;
       }  
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"UI "))
        { 
         Flag_SYNErr = SET;
         Ptr += 3;
         if((*Ptr == 'A')|| (*Ptr == 'O')|| (*Ptr == 'H') || (*Ptr == 'S') || (*Ptr == 'E') || (*Ptr == 'T') || (*Ptr == 'I') || (*Ptr == 'C'))
         {
           if(*(Ptr+1)== ':')
           {
              Flag_SYNErr = RESET;
              if(SPtr = strstr(Ptr,"A"))                                        // ACTIVE UPDATE INTERVAL          
              {
                  SPtr++;
                  if(*SPtr == ':')
                  {
                    SPtr++;   
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else Active_Time = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETUI A,%d",Active_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                }
              if(SPtr = strstr(Ptr,"O"))                                        // STANDBY TIME
              {
                  SPtr++;
                  if(*SPtr == ':')
                  {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else StandBy_Time = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETUI O,%d",StandBy_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                  }
                  else Flag_SYNErr   = SET;
               }
              if(SPtr = strstr(Ptr,"H"))                                        // HEALTH UPDATE INTERVAL
              {
                 SPtr++;
                 if(*SPtr == ':')
                 {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else HLTH_FRQ = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETUI H,%d",HLTH_FRQ);
                    strcat(CC_Source_TER,GP_Buffer);
                 }
                 else Flag_SYNErr   = SET;                 
               }
              if(SPtr = strstr(Ptr,"S"))                                        // Stop Mode Entry
              {
                 SPtr++;
                 if(*SPtr == ':')
                 {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else StopMode_Time = atoi(SPtr);
                    
                    sprintf(GP_Buffer,",SETUI S,%d",StopMode_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                 }
                 else Flag_SYNErr   = SET;
               }
              if(SPtr = strstr(Ptr,"E"))                                        // EMERGENCY UPDATE INTERVAL
              {
                 SPtr++;
                 if(*SPtr == ':')
                 {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else ES_Time = atoi(SPtr);  
                    
                    sprintf(GP_Buffer,",SETUI E,%d",ES_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                 }
                 else Flag_SYNErr   = SET;
               }
              if(SPtr = strstr(Ptr,"T"))                                        // TIME TO ENTER INTO SLEEPMODE  AFTER IGNITION OFF
              {
                 SPtr++;
                 if(*SPtr == ':')
                 {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else Time2Sleep = atoi(SPtr);
                    
                    sprintf(GP_Buffer,",SETUI T,%d",Time2Sleep);
                    strcat(CC_Source_TER,GP_Buffer);
                  }
                  else Flag_SYNErr   = SET;
               }                
              if(SPtr = strstr(Ptr,"I"))                                        // Time to wait for generating over speed alert after device overspeeds
              {
                 SPtr++;
                 if(*SPtr == ':')
                 {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else OVS_DeadTime = atoi(SPtr);
                    
                    sprintf(GP_Buffer,",SETUI I,%d",OVS_DeadTime);
                    strcat(CC_Source_TER,GP_Buffer);
                 }
                 else Flag_SYNErr   = SET;
               }
              if(SPtr = strstr(Ptr,"C"))                                       // Tertirary Server Hit Time
              {
                 SPtr++;
                  if(*SPtr == ':')
                  {
                    SPtr++;   
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else TER_HitTime = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETUI C,%d",TER_HitTime);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
               }            
           }
  
         } 
       
         if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
         { 
           Flag_OK = SET;
         }
         goto END_SMS;
       }
        /******************** SETTD *******************************************/    // SET THE THRESHOLD LEVEL
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"TD "))
        { 
           Ptr += 3;
           Flag_SYNErr = SET;
           if(((*Ptr == 'L')&&(*(Ptr+1) == 'B')) || ((*Ptr == 'H')&&(*(Ptr+1) == 'A')) || ((*Ptr == 'H')&&(*(Ptr+1) == 'B')) || ((*Ptr == 'R')&&(*(Ptr+1) == 'T')) || ((*Ptr == 'S')&&(*(Ptr+1) == 'L')) || ((*Ptr == 'W')&&(*(Ptr+1) == 'K')) || ((*Ptr == 'I') && (*(Ptr+1) == 'D'))) 
           {
             if(*(Ptr+2)== ':')
             {
              Flag_SYNErr = RESET;
              if(SPtr = strstr(Ptr,"LB"))                                       // Internal battery Low
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++;
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 50) || (atoi(SPtr) < 5) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      IBL = atoi(SPtr);   
                      sprintf(GP_Buffer,",SETTD IBL,%d",IBL);
                      strcat(CC_Source_TER,GP_Buffer);
                      
                    }
                  }
                  else Flag_SYNErr = SET;
                }            
              if(SPtr = strstr(Ptr,"HA"))                                       // HA Limit
              {
                SPtr += 2; 
                if(*SPtr == ':')
                {
                    SPtr++;
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atof(SPtr) > 2.0) || (atof(SPtr) < 0.1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      HA = atof(SPtr);                                                // ACTIVE UPDATE INTERVAL  
                      sprintf(GP_Buffer,",SETTD HA,%0.2f",HA);
                      strcat(CC_Source_TER,GP_Buffer);
                    }   
                  }
                  else Flag_SYNErr = SET; 
             }             
              if(SPtr = strstr(Ptr,"HB"))                                       // HB Limit
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atof(SPtr) > 2.0) || (atof(SPtr) < 0.1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      HB = atof(SPtr);                                                // ACTIVE UPDATE INTERVAL  
                      sprintf(GP_Buffer,",SETTD HB,%0.2f",HB);
                      strcat(CC_Source_TER,GP_Buffer);
                    }  
                  }
                  else Flag_SYNErr = SET; 
                }    
              if(SPtr = strstr(Ptr,"RT"))                                       // RT Limit
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++;  
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 360) || (atoi(SPtr) < 10) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      RT = atoi(SPtr);                                                // ACTIVE UPDATE INTERVAL 
                      sprintf(GP_Buffer,",SETTD RT,%d",RT);
                      strcat(CC_Source_TER,GP_Buffer);
                    } 
                  }
                  else Flag_SYNErr = SET; 
                }           
              if(SPtr = strstr(Ptr,"SL"))                                       // Over Speed Limit
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 220) || (atoi(SPtr) < 10) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      Speed_Limit = atoi(SPtr);                                                // ACTIVE UPDATE INTERVAL    
                      sprintf(GP_Buffer,",SETTD SL,%d",Speed_Limit);
                      strcat(CC_Source_TER,GP_Buffer);
                    } 
                  }
                  else Flag_SYNErr = SET; 
                }              
              if(SPtr = strstr(Ptr,"WK"))                                       // Motion Wake
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++;  
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atof(SPtr) > 7.9) || (atof(SPtr) < 0.2) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Accel_WK_Thrs = atof(SPtr);                                                
                      Flag_INVALID = RESET;
                        
                  
                      if(Accel_WK_Thrs < 1.0)LIS3DH_WriteReg(0x5B,0x01); //0x02 //wake up threshold
                      else if(Accel_WK_Thrs > 5.0)LIS3DH_WriteReg(0x5B,0x05); //0x02 //wake up threshold
                      else LIS3DH_WriteReg(0x5B,(int)(Accel_WK_Thrs)); //0x02 //wake up threshold
                    }
                  }
                  else Flag_SYNErr = SET; 
                  
                }               
              if(SPtr = strstr(Ptr,"ID"))                                       // Idel TIme
              {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  {
                    SPtr++;   
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 60) || (atoi(SPtr) < 1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Flag_INVALID = RESET;
                      IDL_Time = atoi(SPtr);                                                // IDLE TIME IN MINUTES    
                    } 
                  }
                  else Flag_SYNErr = SET; 
                }
             }
          } 
          
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             { 
               Flag_OK = SET;
             }
           
           goto END_SMS;
         }
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"EIP "))
        {
           Ptr += 4;
          
           Flag_INVALID = RESET;
           Flag_SYNErr = SET;
           if((*(Ptr+1) == ':') && ((*(Ptr+3) == '\r') || (*(Ptr+3) == '\n')))
           {
             Flag_SYNErr = RESET; 
             if(*Ptr == 'P')  
             {
               if(*(Ptr+2) == '1')     
               {
                 if((SRVR_IP[MAIN][0] != 0) && (SRVR_IP[MAIN][0] != ' ') && (SRVR_Port[MAIN] > 0))Flag_MAINServer = SET;
                 else Flag_INVALID = SET;
              
               }
               else if(*(Ptr+2) == '0')Flag_MAINServer = RESET;
               else Flag_INVALID = SET;
               
               sprintf(GP_Buffer,",SETEIP P,%d",Flag_MAINServer);
               strcat(CC_Source_TER,GP_Buffer);
             }
             else if(*Ptr == 'S') 
             {
                if(*(Ptr+2) == '1')     
                {
                  if((SRVR_IP[EMR][0] != 0) && (SRVR_IP[EMR][0] != ' ') && (SRVR_Port[EMR] > 0))Flag_EMRServer = SET;
                  else Flag_INVALID = SET;
                }
                else if(*(Ptr+2) == '0')Flag_EMRServer = RESET;
                else Flag_INVALID = SET;
                
                sprintf(GP_Buffer,",SETEIP S,%d",Flag_EMRServer);
               strcat(CC_Source_TER,GP_Buffer);
             }
             else if(*Ptr == 'T') 
             {
                if(*(Ptr+2) == '1')
                {
                  if((SRVR_IP[TER][0] != 0) && (SRVR_IP[TER][0] != ' ') && (SRVR_Port[TER] > 0))Flag_TERServer = SET;
                  else Flag_INVALID = SET;
                }
                else if(*(Ptr+2) == '0')Flag_TERServer = RESET;
                else Flag_INVALID = SET;
                
                sprintf(GP_Buffer,",SETEIP T,%d",Flag_TERServer);
               strcat(CC_Source_TER,GP_Buffer);
             }
             else 
             {
               Flag_SYNErr  = SET; 
             }
           } 

             
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET; 
         
          goto END_SMS;
         } 
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"CLRSOS "))
        {
           Flag_SYNErr = RESET;
           Ptr += 7;
           if(((*Ptr == '0') || (*Ptr == '1')) && ((*(Ptr+1) == '\r') || (*(Ptr+1) == '\n')))
           {
             if(*Ptr == '0')Flag_STOPMSG = RESET;
             else Flag_STOPMSG = SET;
             
              sprintf(GP_Buffer,",CLRSOS T,%d",Flag_STOPMSG);
              strcat(CC_Source_TER,GP_Buffer);
              Flag_INVALID = RESET;
              Flag_OK = SET;
           }
           else Flag_INVALID = SET;
           
           goto END_SMS;
         }
        else if(Ptr = strstr((char*)GPRS_MsgBuffer,"GEO "))
        {
             Flag_OK = RESET;
             Flag_SYNErr = SET;
             Ptr += 4;
             SPtr = Ptr;
             
             if((*Ptr >= '0') && (*Ptr <= '9'))                                   // VALID ROUTE ID 
             {
               if(*(Ptr+1) == ',')
               {
                 if((*(Ptr+2) >= '0') && (*(Ptr+2) <= '9'))                       // VALID FENCE ID 
                 {
                   if(*(Ptr+3) == ',')
                   {
                     if(((atof(Ptr+4)) > -91)  && ((atof(Ptr+4)) < 91)) 
                     {
                        Ptr += 4;
                        if(Ptr = strstr(Ptr,","))
                        {
                          Flag_SYNErr = RESET;
                          Ptr++;
                           if(((atof(Ptr)) > -181)  && ((atof(Ptr)) < 181)) 
                           {
                                
                             GF[*SPtr - 48].ID = *(SPtr) - 48;
                             GF[*SPtr - 48].LAT[*(SPtr+2) - 48] = atof(SPtr + 4);
                             GF[*SPtr - 48].LON[*(SPtr+2) - 48] = atof(Ptr);
                             
                             Flag_INVALID = RESET;
                             Flag_OK = SET;
                            
                           }
                           else {Flag_INVALID = SET;}
                        }
                     }
                     else {Flag_INVALID = SET; Flag_SYNErr = RESET;}
                   }
                 }
               }
             }
                
             goto END_SMS;              
          }
        else 
        {
           if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP1-"))
           {
             Ptr += 4;
             SPtr = Ptr;
             for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
             {
               if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
               {
                 Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
                 break;
               }
               else 
                {
                   if(*Ptr == ' ')
                   {
                     Flag_INVALID = SET;
                     Flag_SYNErr = RESET;
                     Flag_OK = RESET;
                     break;
                   }
                }
             } 
             
             if(GP_Counter > 49)Flag_INVALID = SET;
             if(GP_Counter < 4)
             {
               Flag_INVALID = SET;
               Flag_SYNErr = RESET;
               Flag_OK = RESET;
             }
             
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
             {
            
               memset(&SRVR_IP[MAIN][0],0,50);
               Ptr = SPtr;
               for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
               {
                 if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[MAIN][GP_Counter] = 0;break;}
                 else 
                  {
                     SRVR_IP[MAIN][GP_Counter] = *Ptr;
                  }
               } 
               sprintf(GP_Buffer,",SETIP1-,%s",SRVR_IP[MAIN]);
               strcat(CC_Source_TER,GP_Buffer);
               Flag_OK = SET;
               Flag_INVALID = RESET;
               Flag_SYNErr = RESET;

               Flag_NewIP = SET;
           }
             else goto END_SMS;
             
           } 
           // SERVER PORT 1
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT1-"))
           {
             Flag_SYNErr = RESET;
             Flag_INVALID = RESET;
             Ptr += 6;
             if(atoi(Ptr) > 65535)
             {
               Flag_INVALID = SET;
             }
             else 
             {
               SRVR_Port[MAIN] = atoi(Ptr);  
               
               Flag_OK = SET;
               Flag_INVALID = RESET;
               Flag_SYNErr = RESET;
               
               Flag_NewIP = SET;
         
             }
             
             if((Flag_SYNErr == RESET) && (Flag_INVALID == RESET))
             {
               Flag_OK = SET;
               sprintf(GP_Buffer,",SETPORT1-,%d",SRVR_Port[MAIN]);
               strcat(CC_Source_TER,GP_Buffer);
             }
             
             
           };
           if(Flag_NewIP == SET){Flag_NewIP = RESET; Flag_TCPConnect[MAIN] = RESET;Modem_PutString("AT+QICLOSE=0"); WAIT_MODEM_RESP(10);}       
           
           // SERVER IP 2
           if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP2-"))
           {
             Ptr += 4;
             SPtr = Ptr; 
     
            for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
             {
               if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
               {
                 Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
                 break;
               }
               else 
                {
                   if(*Ptr == ' ')
                   {
                     Flag_INVALID = SET;
                     Flag_SYNErr = RESET;
                     Flag_OK = RESET;
                     break;
                   }
                }
             } 
             
            if(GP_Counter > 49)Flag_INVALID = SET;
            
             if(GP_Counter < 4)
             {
               Flag_INVALID = SET;
               Flag_SYNErr = RESET;
               Flag_OK = RESET;
             }
            
            
            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
            {
             Ptr = SPtr;
             memset(&SRVR_IP[EMR][0],0,50);
             for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
             {
               if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[EMR][GP_Counter] = 0;break;}
                else 
                {
                   SRVR_IP[EMR][GP_Counter] = *Ptr;
                }
             }    
            
             sprintf(GP_Buffer,",SETIP2,%s",SRVR_IP[EMR]);
             strcat(CC_Source_TER,GP_Buffer);
             
             Flag_OK = SET;
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;

             Flag_NewIP2 = SET;
           }
           else goto END_SMS;
           }
           // SERVER PORT 2
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT2-"))
           {
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Ptr += 6;
             if(atoi(Ptr) > 65535)
             {
               Flag_INVALID = SET;
             }
             else 
             {
             
             SRVR_Port[EMR] = atoi(Ptr);
             
              sprintf(GP_Buffer,",SETPORT2-,%d",SRVR_Port[EMR]);
               strcat(CC_Source_TER,GP_Buffer);
              Flag_OK = SET;
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;

              Flag_NewIP2 = SET;
             
             }
             
           };
           // IF CHANGED CLOSE CONNECTION
           if(Flag_NewIP2 == SET){Flag_NewIP2 = RESET; Flag_TCPConnect[EMR] = RESET;Modem_PutString("AT+QICLOSE=2"); WAIT_MODEM_RESP(10);}     
           
           // SERVER IP 3
           if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP3-"))
           {
             Ptr += 4;
             SPtr = Ptr;
             
            for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
             {
               if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
               {
                 Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
                 break;
               }
               else 
                {
                   if(*Ptr == ' ')
                   {
                     Flag_INVALID = SET;
                     Flag_SYNErr = RESET;
                     Flag_OK = RESET;
                     break;
                   }
                }
             } 
             
             if(GP_Counter > 49)Flag_INVALID = SET;
             if(GP_Counter < 4)
             {
               Flag_INVALID = SET;
               Flag_SYNErr = RESET;
               Flag_OK = RESET;
             }
            
            
            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
            {
             Ptr = SPtr;
             memset(&SRVR_IP[TER][0],0,50);
             for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
             {
               if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[TER][GP_Counter] = 0;break;}
                else 
                {
                  SRVR_IP[TER][GP_Counter] = *Ptr;
                }
             }  
             
             sprintf(GP_Buffer,",SETIP3,%s",SRVR_IP[TER]);
             strcat(CC_Source_TER,GP_Buffer);
             Flag_OK = SET;
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Flag_NewIP3 = SET;
            }
            else goto END_SMS;
             
           }
           // SERVER PORT 3
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT3-"))
           {
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Ptr += 6;
             if(atoi(Ptr) > 65535)
             {
               Flag_INVALID = SET;
             }
             else 
             {
               SRVR_Port[TER] = atoi(Ptr);

               sprintf(GP_Buffer,",SETPORT3-,%d",SRVR_Port[TER]);
               strcat(CC_Source_TER,GP_Buffer);
               Flag_OK = SET;
               Flag_INVALID = RESET;
               Flag_SYNErr = RESET;
               Flag_NewIP3 = SET;
              
             }
           } 
           if(Flag_NewIP3 == SET){ Flag_NewIP3 = RESET; Flag_TCPConnect[TER] = RESET;Modem_PutString("AT+QICLOSE=1"); WAIT_MODEM_RESP(10);}   
           // TRANS TIME
           if(Ptr = strstr((char*)GPRS_MsgBuffer,"TT-"))
           {
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
             Ptr += 3;
             if((atoi(Ptr) > 300) || (atoi(Ptr) < 10))
             {
               Flag_INVALID = SET;
             }
             else 
             {
              Trans_Time = atoi(Ptr);
              Flag_OK = SET;
             }
             
             
           }      
           // NEW PASSWORD 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PWD-"))
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 4;
              SPtr = Ptr;
              if((Ptr = strchr(SPtr,',')) ||(Ptr = strchr(SPtr,'\r')) || (Ptr = strchr(SPtr,'\n')))
              {
                if(((Ptr - SPtr) < 5) && (Ptr > SPtr))
                {
                  Ptr = SPtr;
                  for(GP_Counter = 0; GP_Counter < 4;GP_Counter++,Ptr++) 
                  {
                      if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))break;
                      if(((*Ptr >= 'A') && (*Ptr <= 'Z')) || ((*Ptr >= 'a') && (*Ptr <= 'z'))|| ((*Ptr >= '0') && (*Ptr <= '9')));
                      else 
                      {
                        Flag_INVALID = SET; break;
                      }
                  }
                }
                else Flag_INVALID = SET;
              }
              

              if(GP_Counter < 2)
                {
                    Flag_INVALID = SET;
                }
              else if(Flag_INVALID == RESET)  
              {
              memset(PWD_IN,0,sizeof(PWD_IN));
              for(GP_Counter = 0; GP_Counter < 4;GP_Counter++,SPtr++) 
                {
                  if((*SPtr == ',') || (*SPtr == '\r') || (*SPtr == '\n') || (*SPtr == '\0')){PWD_IN[GP_Counter] = 0;break;}
                  else 
                  {
                      PWD_IN[GP_Counter] = *SPtr;
                   }
                }
              
                Flag_OK = SET;
              }
              
              
             }
           // MEMSE ENABLE / DISABLE
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"MEMS-"))
           {
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
             Ptr += 5;
             if(*Ptr == 'E')Flag_MEMS_ENB = SET;
             else if(*Ptr == 'D')Flag_MEMS_ENB = RESET;
             else
              {
               Flag_INVALID = SET;
              }
             
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             {
               Flag_OK = SET;
             }
             
             
           }
           // SIM PROFILE SWAPPING REQUEST 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"SP-"))                            //Sim Profile
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 3; 
              SP_REQ = *Ptr-48;
              if(SP_REQ == 0 || SP_REQ == 1 || SP_REQ == 2 || SP_REQ == 3)Flag_SWAP = SET;
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
            }
           // SIM SWAPPING ENABLE DISBALE 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"SWAP-"))                          //SWAPPING  
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 5; 
              if(*Ptr == '1' || *Ptr == '0')
              {
                if(*Ptr == '1')Flag_SWAP_ENB = SET;
                else Flag_SWAP_ENB = RESET;
              }
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
            } 
           // Sleep Again
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"SAT-"))                           //Sleep Again Time  
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 4; 
              if(*Ptr >= '0' || *Ptr <= '9')
              {
                 if((atoi(Ptr) > 179) && (atoi(Ptr) < 601))
                {
                 SleepAgainTime = atoi(Ptr);
                }
                else Flag_INVALID = SET;
              }
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
            } 
           // EMERGENCY SELF TIMEOUT TIME 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"ED-"))
           {
             Flag_INVALID = RESET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
             Ptr += 3; 
             if((atoi(Ptr) > 7200) || (atoi(Ptr) < 60))
             {
               Flag_INVALID = SET;
             }
             else 
             {
               ED_Time = atoi(Ptr);
             }
             
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             {
                 Flag_OK = SET;
                 GetConfig_Server();
                 sprintf(GP_Buffer,",SET ED,%d",ED_Time);
                 strcat(CC_Source_TER,GP_Buffer);
             
             }
             
             
           }    
           // External Batt Low
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"EBL-"))                           // EBL  
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 4; 
              if(*Ptr >= '0' || *Ptr <= '9')
              {
                 if((atoi(Ptr) > 4) && (atoi(Ptr) < 31))
                {
                 EBL = atoi(Ptr);
                 Flag_Check_EXT_PWR = SET;
                }
                else Flag_INVALID = SET;
              }
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
            } 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"DSF-"))                           // DeepSleepFreq  
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 4; 
              if(*Ptr >= '0' || *Ptr <= '9')
              {
                if((atoi(Ptr) > 59) && (atoi(Ptr) < 86401))
                {
                 DeepSleepFreq = atoi(Ptr);
                }
                else Flag_INVALID = SET;
              }
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
            } 
           // DEEP SLEEP FREQ ENABLE DISBALE 
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"DSC-"))                           //DSF Counter ENABLE DISABLE  
           {
              Flag_INVALID = RESET;
              Flag_SYNErr = RESET;
              Flag_OK = RESET;
              Ptr += 4; 
              if(*Ptr == '1' || *Ptr == '0')
              {
                if(*Ptr == '1')Flag_DSF_ENB = SET;
                else Flag_DSF_ENB = RESET;
              }
              else
              {
                Flag_INVALID = SET;
              }
               
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
               {
                 Flag_OK = SET;
               }
        } 
           // SETR VEHID
           else if(Ptr = strstr((char*)GPRS_MsgBuffer,"VEHID"))                           //Vehicle ID By SMS 
           {
             Flag_SYNErr = RESET;
             Ptr += 6;SPtr = Ptr;
             
             if(strstr(Ptr,"\r"))Ptr = strstr(Ptr,"\r");
             else Ptr = strstr(Ptr,",");
           
               if(Ptr)
               {
                memset(loc_buffer,0,20);           
                if(((Ptr-SPtr) < 17) && (Ptr > SPtr))
                  {
                   strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                   Ptr = &loc_buffer[0];
                   
                   memset(Vehicle.VRN,0,17);
                   memcpy(&Vehicle.VRN[0],loc_buffer,strlen(loc_buffer));
                   GetConfig_Server();
                   sprintf(GP_Buffer,",SETVEHID,%s",&Vehicle.VRN[0]);
                   strcat(CC_Source_TER,GP_Buffer);
                   Flag_OK = SET;
                  }
                else 
                {
                  Flag_INVALID = SET;Flag_SYNErr = RESET;
                }
            } 
        }
        }
    }
    else if(Ptr = strstr((char*)GPRS_MsgBuffer,"CLR"))
    {
       if(Ptr = strstr(GPRS_MsgBuffer,"PKT"))
       {
          TCP_BackCnt[MAIN] = 0;
          TCP_BackCnt[TER] = 0;
          EMRGY_BKP_No = 0;
          
          RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
                    
          FlashAddress = SPIFLASH_CONFIG;
          memset(loc_buffer,0,50);
          sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
             
           if(Flag_SPIOK == SET)
            {
             SPI_FLASH_SectorErase(FlashAddress);
             SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
             Send_SMS(ACKN,SET);
             goto END_MSG;
            }
           
           
           Flag_INVALID = RESET;
           Flag_SYNErr = RESET;
           goto END_SMS;
        }
       else if(Ptr = strstr(GPRS_MsgBuffer,"MOB "))
       {
         Ptr += 4;
         if((*Ptr > '0') && (*Ptr <= '5'))
         {
            memset(&Vehicle.Master_No[*Ptr - 49][0],0,15);
            sprintf(&Vehicle.Master_No[*Ptr - 49][0],"N.A0000000"); 
            Flag_OK = SET;
            Flag_INVALID = RESET;
            Flag_SYNErr = RESET;
            
            
         }
         else 
         {
           Flag_INVALID = SET;
           Flag_SYNErr = RESET;
         }
         goto END_SMS;
       }
       else if(Ptr = strstr(GPRS_MsgBuffer,"MOBALL"))
       {    
          memset(&Vehicle.Master_No[0][0],0,15);
          memset(&Vehicle.Master_No[1][0],0,15);
          memset(&Vehicle.Master_No[2][0],0,15);
          memset(&Vehicle.Master_No[3][0],0,15);
          memset(&Vehicle.Master_No[4][0],0,15);
          
          sprintf(&Vehicle.Master_No[0][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
          sprintf(&Vehicle.Master_No[1][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
          sprintf(&Vehicle.Master_No[2][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
          sprintf(&Vehicle.Master_No[3][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
          sprintf(&Vehicle.Master_No[4][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
      
          
          
          Flag_OK = SET;
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          goto END_SMS;   
       }
       else if(strstr(GPRS_MsgBuffer,"CLRSOS"))goto CLR;
       else 
       {
          Flag_OK = RESET;
          Flag_INVALID = RESET;
          Flag_SYNErr = SET;
          goto END_SMS;  
       }
      

   }   
    else if(Ptr = strstr((char*)GPRS_MsgBuffer,"RST "))
    {
       Ptr += 4;
       if(*Ptr == 'S') 
       {
         Reset_SystemTime = 30;
         Send_SMS(ACKN,SET);
         Flag_TxENB = RESET;
         Flag_LogIn = RESET;
         Flag_Debug_GNSS = RESET;
         Flag_Debug_GPRS = RESET;
       }
       else if(*Ptr == 'M') {Flag_MODULE_M66_RST = SET; Send_SMS(ACKN,SET);}
       else if(*Ptr == 'H') 
       {
         if(Vehicle.Ignition[0] == 'F')
         {
           Send_SMS(ACKN,SET);   
           TAKE_FUL_BKP(); 
           Write_Oddo(); 
           REG_LM2576(DISABLE); 
         }
         else 
         {
           Send_SMS(NA,SET); 
         }
         
       }
       else
       {
          Send_SMS(INVALID,SET);
       }
       goto END_MSG;
     
   }
    else if(strstr((char*)GPRS_MsgBuffer,"GEOFCTRY"))
    {
     for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)
     { 
        GF[GP_Counter].ID = 0;
        GF[GP_Counter].Last_FS = 0;
        for(FlashAddress = 0; FlashAddress < 10; FlashAddress++)
         {
           GF[GP_Counter].LAT[FlashAddress] = 0;
           GF[GP_Counter].LON[FlashAddress] = 0;
         }
      }
     
      Flag_FlashError = SET;
      Write_Config();
      if(Flag_FlashError == SET)Send_SMS(FLASH_ERR,SET);
      else Send_SMS(ACKN,SET);
      goto END_MSG;
   } 
    else if(strstr((char*)GPRS_MsgBuffer,"FCTRY"))
    {
       Send_SMS(ACKN,SET);
       SET_FACTORY();
       goto END_MSG;
    }    
    else if(strstr((char*)GPRS_MsgBuffer,"@CLR#"))
    {
       // CLEAR ODDO
       if(Ptr = strstr((char*)GPRS_MsgBuffer,"ODDO"))
       {
          Ptr += 4;
          MTR = 0; KM = 0;
          FlashAddress = SPIFLASH_CONFIG_ODO;
          memset(loc_buffer,0,30);
          sprintf(loc_buffer,"KM-%u,MTR-%hu",KM,MTR);  
          if(Flag_SPIOK == SET)
            {
             SPI_FLASH_SectorErase(FlashAddress);
             SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
            } 
           Send_SMS(ACKN,SET);
       }
    }
    else if(((strstr((char*)GPRS_MsgBuffer,"STOP_MSG")) || (strstr((char*)GPRS_MsgBuffer,"CLRSOS")))&& (Flag_Emrgcy == SET))
    {
    CLR:
      if(Flag_STOPMSG == SET)
      {
        ED_Timer = 0;
        Flag_Emrgcy = RESET;
        
        Save_EMR();
        
        Flag_EA = RESET;
        Flag_EA_OFF = SET; 
        Flag_OK = SET;
        Flag_MAIN_PKT = SET;
        Flag_TER_PKT = SET;
        ServerHit_Time = 2; 
        TER_ServerHit_Time = TER_HitTime + 3;
        
        Flag_EM_LED = RESET; 
        Get_EMRGY_PKT(); 
        Flag_HitServ = SET;  
      }
      else 
      {
        Flag_OK = RESET;
        Flag_INVALID = RESET;
        Flag_SYNErr = SET;
        goto END_SMS;
      }
    }   
    else if((strstr((char*)GPRS_MsgBuffer,"GET:")) || (strstr((char*)GPRS_MsgBuffer,"GET")) || (strstr((char*)GPRS_MsgBuffer,"@GET#")))
    {
       if(strstr((char*)GPRS_MsgBuffer,"CONFIG") || strstr((char*)GPRS_MsgBuffer,"GIP#"))Send_SMS(CONFIG,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"IOSTS"))Send_SMS(IOSTATUS,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"STS"))Send_SMS(GETSTS,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"MOB "))
       {  
         Ptr += 4;
         if((*(Ptr + 1) == '\r') || (*(Ptr + 1) == '\n'))
         {
           GetConfig_Server();
           if(*Ptr == '1')      { sprintf(GP_Buffer,",GET MOB 1,%s",Vehicle.Master_No[0]); Send_SMS(MOBNUM1,SET);}
           else if (*Ptr == '2'){ sprintf(GP_Buffer,",GET MOB 2,%s",Vehicle.Master_No[1]); Send_SMS(MOBNUM2,SET);}
           else if (*Ptr == '3'){ sprintf(GP_Buffer,",GET MOB 3,%s",Vehicle.Master_No[2]); Send_SMS(MOBNUM3,SET);}
           else if (*Ptr == '4'){ sprintf(GP_Buffer,",GET MOB 4,%s",Vehicle.Master_No[3]); Send_SMS(MOBNUM4,SET);}
           else if (*Ptr == '5'){ sprintf(GP_Buffer,",GET MOB 5,%s",Vehicle.Master_No[4]); Send_SMS(MOBNUM5,SET);}
           else 
           {
             sprintf(GP_Buffer,",UNKNOWN MOB,N.A");
             Send_SMS(SYN_ERR,SET);strcat(CC_Source_TER,GP_Buffer);
             goto END_MSG;
           }
         }
         else { sprintf(GP_Buffer,",UNKNOWN MOB,N.A"); strcat(CC_Source_TER,GP_Buffer);Send_SMS(SYN_ERR,SET);  goto END_MSG;};
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"VER"))Send_SMS(HW_VER,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"SMSPKT"))Send_SMS(SMSPKT,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"FENCE "))
       {
         Ptr += 6;
         if((*Ptr >= '0') && (*Ptr <= '9'))
         {
           R_ID = *Ptr - 48;
           Send_SMS(GEO_FENCE,SET);
         }
         else 
         {
           Send_SMS(SYN_ERR,SET);
           goto END_MSG;
         }
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"GEO "))
       {
         Ptr += 4;
         if((*Ptr >= '0') && (*Ptr <= '9'))
         {
           R_ID   = *Ptr - 48;
           Ptr++;
           if(*Ptr == ',')
           {
             Ptr++;
             if((*Ptr >= '0') && (*Ptr <= '9'))
             {
                FNC_ID = *Ptr - 48;
                Send_SMS(GEOFENCE,SET);
             }
            else 
            {
              Send_SMS(INVALID,SET);
              goto END_MSG;
            } 
           }
           else 
            {
              Send_SMS(SYN_ERR,SET);
              goto END_MSG;
            }
         }
         else 
         {
           Send_SMS(INVALID,SET);
           goto END_MSG;
         }
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"HPKT"))Send_SMS(HEALTH,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PARAMS"))Send_SMS(PARAMS,SET);
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"NUMBER:"))
       {
         Ptr += 7;
         if(*Ptr == '0')Ptr += 1;
         else if(*Ptr == '+')Ptr += 3;
         if((*Ptr > '0') && (*Ptr <= '9'))
         {
            SPtr = Ptr;
            if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")) || (Ptr = strstr(Ptr,",")))
                {
                  memset(loc_buffer,0,50);           
                   if(((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 10))
                    {
                     strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                     Ptr= &loc_buffer[0];
                     while(*Ptr != '\0'){if(*Ptr < '0' || *Ptr > '9'){Flag_INVALID = SET;Flag_SYNErr = RESET;break;} Ptr++;} 
                     
                     if(Flag_INVALID == RESET)
                     {
                        Flag_SYNErr = RESET;
                        memcpy(&GPRSData.Mobile_No[0],loc_buffer,15);
                        Send_SMS(LOCATION,SET);
                     }
                    }
                  else 
                  {
                    Flag_INVALID = SET;
                    Flag_SYNErr = RESET;
                    Flag_OK = RESET;
                  }
                }
         }
         
       }
       else if(strstr((char*)GPRS_MsgBuffer,"DEBUG"))Send_SMS(DEBUG,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"LOC"))  Send_SMS(LOCATION,SET);
       //else if(strstr((char*)GPRS_MsgBuffer,"GEO"))  Send_SMS(GEO_FENCE,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"GPS"))  Send_SMS(GPSDATA,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"GPRS")) Send_SMS(GPRSDATA,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"INFO")) Send_SMS(INFO,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"CONF")) Send_SMS(CONF,SET);
       else if(strstr((char*)GPRS_MsgBuffer,"DFU"))  { if(Flag_PswAuthOK || Flag_MstAuthOK)Send_SMS (DFU_DETAIL,SET);}
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"VEHID"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GETVEHID,%s",&Vehicle.VRN[0]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(GETVID,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"ED"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET ED,%d",ED_Time);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"TD "))
       {
         GetConfig_Server();
         if(strstr((char*)GPRS_MsgBuffer,"TD BL"))sprintf(GP_Buffer,",GET TD BL,%d",IBL);
         else if(strstr((char*)GPRS_MsgBuffer,"TD HA"))sprintf(GP_Buffer,",GET TD HA,%0.2f",HA);
         else if(strstr((char*)GPRS_MsgBuffer,"TD HB"))sprintf(GP_Buffer,",GET TD HB,%0.2f",HB);
         else if(strstr((char*)GPRS_MsgBuffer,"TD RT"))sprintf(GP_Buffer,",GET TD RT,%d",RT);
         else if(strstr((char*)GPRS_MsgBuffer,"TD SL"))sprintf(GP_Buffer,",GET TD SL,%d",Speed_Limit);
         else
         {
           Flag_OTACFG = RESET;
           sprintf(GP_Buffer,",UNKNOWN COMMAND,0");
         }
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(TD,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"UI "))
       {
         GetConfig_Server();
         if(strstr((char*)GPRS_MsgBuffer,"UI A"))sprintf(GP_Buffer,",GET UI A,%d",Active_Time);
         else if(strstr((char*)GPRS_MsgBuffer,"UI O"))sprintf(GP_Buffer,",GET UI O,%d",StandBy_Time);
         else if(strstr((char*)GPRS_MsgBuffer,"UI S"))sprintf(GP_Buffer,",GET UI S,%d",StopMode_Time);
         else if(strstr((char*)GPRS_MsgBuffer,"UI H"))sprintf(GP_Buffer,",GET UI H,%d",HLTH_FRQ);
         else
         {
           Flag_OTACFG = RESET;
           sprintf(GP_Buffer,",UNKNOWN COMMAND,0");
         }
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(UI,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP1"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP1,%s",SRVR_IP[MAIN]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP1,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP2"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP2,%s",SRVR_IP[EMR]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP2,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"IP3"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP3,%s",SRVR_IP[TER]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP3,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT1"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT1,%d",SRVR_Port[MAIN]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP1,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT2"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT2,%d",SRVR_Port[EMR]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP2,SET);
       }
       else if(Ptr = strstr((char*)GPRS_MsgBuffer,"PORT3"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT3,%d",SRVR_Port[TER]);
         strcat(CC_Source_TER,GP_Buffer);
         Send_SMS(IP3,SET);
       }     
       else  Send_SMS(SYN_ERR,SET);
        
     
    }
    else if(strstr((char*)GPRS_MsgBuffer,"@POLL#")) 
    {
      Send_SMS(ACKN,SET);
      
      Flag_MAIN_PKT = SET;
      Flag_TER_PKT = SET;
      ServerHit_Time = 2; 
      TER_ServerHit_Time = TER_HitTime + 3;
    }    // POLLING 
END_SMS:
    
    /*------------ CHECK FOR VALID DATA ------------------------------------------*/
    if((Active_Time  > 3600)  || (Active_Time  < 5)) Active_Time  = 10;
    if((StandBy_Time > 3600)  || (StandBy_Time < 5)) StandBy_Time = 300;
    if((ES_Time      > 3600)  || (ES_Time      < 5)) ES_Time      = 30;
    if((Trans_Time   > 60)    || (Trans_Time  < 10)) Trans_Time = 20;
    if((StopMode_Time > 3600) || (StopMode_Time < 5))StopMode_Time = 1800;
    if(Time2Sleep   > 3600)Time2Sleep = 60;
    if((OVS_DeadTime > 3600)  || (OVS_DeadTime  < 5))OVS_DeadTime = 5;
    if(( SleepAgainTime  > 600)  || (SleepAgainTime    < 180))SleepAgainTime = 180;
    if((Speed_Limit  > 220)   || (Speed_Limit < 10)) Speed_Limit  = 80;
    if((TER_HitTime  > 3600)  || (TER_HitTime  < 5)) TER_HitTime  = 10;
   
    if(HA > 3.5 || HA < 0.1)HA = 2.5;                                      // VALUE IN G-FORCE 
    if(HB > 3.5 || HB < 0.1)HB = 2.5;
    if(RT > 360 || RT < 10) RT = 35;
    if(IBL > 50 || IBL < 5)IBL = 35;
    if(EBL > 30 || EBL < 5)EBL = 15;
    if(HLTH_FRQ < 5 || HLTH_FRQ > 3600)HLTH_FRQ = 600;
    if(EMG_BP_FREQ    > 60000 || EMG_BP_FREQ    < 50)EMG_BP_FREQ = 3000;
    if(EMG_WC_Timeout > 60000 || EMG_WC_Timeout < 50)EMG_WC_Timeout = 15000;
    if(IDL_Time > 60  || IDL_Time < 1)IDL_Time = 10;
    if(ED_Time > 7200 || ED_Time < 60)ED_Time = 300;
    if(DeepSleepFreq > 86400 || DeepSleepFreq < 60) DeepSleepFreq = 86400;
  
    if(Flag_OK == SET)
    { 
      Flag_FlashError = SET;
      Write_Config();
      Flag_OK = RESET; 
      if(Flag_FlashError == SET)Send_SMS(FLASH_ERR,SET);
      else Send_SMS(ACKN,SET);
    }
    else if(Flag_INVALID == SET){Flag_INVALID = RESET; Send_SMS(INVALID,SET);}
    else if(Flag_SYNErr  == SET){Flag_SYNErr  = RESET; Send_SMS(SYN_ERR,SET);}
 } 
  /*---------- GET COMMANDS , ALARM, ALERTS ------------------------------------*/
  else Send_SMS(INVALID,SET);
  
END_MSG :
   Modem_PutString("AT+CMGD=1,4");WAIT_MODEM_RESP(2); // Delete Received SMS
   if(strlen(GPRS_MsgBuffer) < 255)
   {
      memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
   }
   else 
   {
      memset(GPRS_MsgBuffer,0,250);
   }
   if(Flag_DFU == SET){Flag_DFU = RESET;Run_IAP();}
      
}
/*******************************************************************************
* Function Name : Send_SMS
* Description   : Send SMS via GPRS Module 
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void Send_SMS(uint8_t Subject,FlagStatus GSM_STATUS)
{
     char *Ptr;
     char Temp_Buffer[100];
     uint8_t UpLoad_Time;
     uint16_t GP_Counter;
     
     Ptr = NULL;
     GP_Counter = 0;
     UpLoad_Time = 0;
     
    
     
    if(GSM_STATUS == SET) 
     {
       if((Subject == EMG) || (Subject == EMRN) || (Subject == EMRF) || (Subject == OVS) || (Subject == TLT) || (Subject == GEN) ||  (Subject == GEX)||  (Subject == MBR) ||  (Subject == RNC) ||  (Subject == DT))
       {
         if((Vehicle.Master_No[0][0] == 0 || Vehicle.Master_No[0][0] == 'n' || Vehicle.Master_No[0][0] == 'N')) goto SKIP;
         memset(Temp_Buffer,0,sizeof(Temp_Buffer));
         strncpy(Temp_Buffer,&Vehicle.Master_No[0][0],strlen(&Vehicle.Master_No[0][0])); 
       }
       else if(Subject == DFU)
       {
         if((DFU_Mob[0] == 0 || DFU_Mob[0] == 255 ||  DFU_Mob[0] == 'n' || DFU_Mob[0] == 'N')) goto SKIP;
         memset(Temp_Buffer,0,sizeof(Temp_Buffer));
         strncpy(Temp_Buffer,&DFU_Mob[0],strlen(&DFU_Mob[0])); 
       }
       else if(Subject == HLTH_ACTV)
       {
         if((Reply_No[0] == 0 || Reply_No[0] == 'n' || Reply_No[0] == 'N')) goto SKIP;
         memset(Temp_Buffer,0,sizeof(Temp_Buffer));
         strncpy(Temp_Buffer,&Reply_No[0],strlen(&Reply_No[0])); 
       }
       else 
       {
         if((GPRSData.Mobile_No[0] == 0 || GPRSData.Mobile_No[0] == 'n' || GPRSData.Mobile_No[0] == 'N')) goto SKIP;
         memset(Temp_Buffer,0,sizeof(Temp_Buffer));
         strncpy(Temp_Buffer,&GPRSData.Mobile_No[0],strlen(&GPRSData.Mobile_No[0])); 
       }
       
       strcat(CC_Source_TER,GP_Buffer);
      
       memset(GPRS_Buffer,0,50);GPRS_Count = 0;
       Serial_PutString_GPRS("AT+CMGS=");                                         // SEND COMMAND TO START SMS
       Serial_SendData_GPRS(0x22);                                                // INSERT '"'
       Serial_SendData_GPRS('+');                                                 // FOR ALL MOBILE SMS ROAMING
       Serial_SendData_GPRS('9');
       Serial_SendData_GPRS('1');
       
//       if(Subject == DFU)Serial_PutData_GPRS(DFU_Mob,strlen(DFU_Mob));                         // SEND DFU NUMBER  (DFU SUNJECT)
//       else if(Subject == EMG)Serial_PutData_GPRS(&Vehicle.Master_No[0][0],strlen(&Vehicle.Master_No[0][0])); 
//       else if(Subject == HLTH_ACTV)Serial_PutData_GPRS(Reply_No,strlen(Reply_No));
//       else Serial_PutData_GPRS(GPRSData.Mobile_No,strlen(GPRSData.Mobile_No));                           // SEND MOBILE NUMBER 
       Serial_PutData_GPRS(Temp_Buffer,strlen(Temp_Buffer));
       Serial_SendData_GPRS(0x22);Serial_SendData_GPRS(13);                       // INSERT '"' & LF
     }
     
    
     memset((char*)GPRS_SendBuffer,0,sizeof(GPRS_SendBuffer));
     GPRS_Buffer[160] = 0;
     GPRS_Buffer[250] = 0;
     GPRS_Buffer[498] = 0;
     GPRS_Buffer[499] = 0;
     GPRS_Count = 0;
     
     
     switch(Subject)
     {
      
     case EMRN          :sprintf((char*)GPRS_SendBuffer,"VRN-%s Emergency ON",Vehicle.VRN);break;
     case EMRF          :sprintf((char*)GPRS_SendBuffer,"VRN-%s Emergency OFF",Vehicle.VRN);break;
     case OVS           :sprintf((char*)GPRS_SendBuffer,"VRN-%s Over Speed",Vehicle.VRN);break;
     case TLT           :sprintf((char*)GPRS_SendBuffer,"VRN-%s TILT Alert",Vehicle.VRN);break;
     case GEN           :sprintf((char*)GPRS_SendBuffer,"VRN-%s Fence Entry ",Vehicle.VRN);break;
     case GEX           :sprintf((char*)GPRS_SendBuffer,"VRN-%s Fence Exit ",Vehicle.VRN);break;
     case MBR           :sprintf((char*)GPRS_SendBuffer,"VRN-%s Main Battery Remove ",Vehicle.VRN);break;
     case RNC           :sprintf((char*)GPRS_SendBuffer,"VRN-%s Main Battery Reconnect ",Vehicle.VRN);break;
     case DT            :sprintf((char*)GPRS_SendBuffer,"VRN-%s Device Tamper ",Vehicle.VRN);break;
     
     case GPSDATA       :sprintf((char*)GPRS_SendBuffer,"#");                   // START OF STRING #
                         strcat((char*)GPRS_SendBuffer,(char*)GPRSData.IMEI_No);// ADD IMEI NUMBER 
                         if(Flag_IRNSSFix == SET)                                             
                         {
                            strcat((char*)GPRS_SendBuffer,",");
                            strcat((char*)GPRS_SendBuffer,IRNSSData.DATE);      // ADD CURRENT DATE
                            strcat((char*)GPRS_SendBuffer,IRNSSData.TIME);      // ADD CURRENT TIME
                            strcat((char*)GPRS_SendBuffer,IRNSSData.LAT);       // ADD LAT CO-OR. FROM GPS/GNSS STRING  
                            strcat((char*)GPRS_SendBuffer,IRNSSData.LON);       // ADD LONG CO-OR. FROM GPS/GNSS STRING           
                            strcat((char*)GPRS_SendBuffer,IRNSSData.SPEED);     // ADD SPEED
                            strcat((char*)GPRS_SendBuffer,IRNSSData.DIRECTION); // ADD DIRECTION
                            strcat((char*)GPRS_SendBuffer,IRNSSData.Altitude);  // ADD ALTITUDE VALUE IN METER FROM GPGGA STRING 
                            strcat((char*)GPRS_SendBuffer,",");                // ADD SEPARATOR
                            strcat((char*)GPRS_SendBuffer,IRNSSData.PDOP);      // ADD MINIMUM HDOP VALUE AFTER COMPARING BETWEEN GPS & GNSS 
                            strcat((char*)GPRS_SendBuffer,",");                // ADD SEPARATOR
                            strcat((char*)GPRS_SendBuffer,IRNSSData.HDOP);      // ADD MINIMUM HDOP VALUE AFTER COMPARING BETWEEN GPS & GNSS 
                            strcat((char*)GPRS_SendBuffer,",");                // ADD SEPARATOR
                            GP_Counter = strlen(GPRS_SendBuffer);              // TOTAL SIZE OF SERVER DATA
                            Ptr = &GPRS_SendBuffer[GP_Counter];
                            sprintf(Ptr,"%02d,",IRNSSData.Used_Satellite);
                          }
                         else 
                         {
                           strcat(GPRS_SendBuffer,"IRNSS Not Fix");
                           strcat(GPRS_SendBuffer,"\nLast_Loc & Date:");
                           memset(Temp_Buffer,0,30);
                           sprintf(Temp_Buffer,"%s,%s,%02d%02d%2002d",Last_Lat,Last_Lon,RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year);
                           strcat(GPRS_SendBuffer,Temp_Buffer);
                         }
                         memset(Temp_Buffer,0,30);
                         sprintf(Temp_Buffer,"RTC %02d:%02d:%02d",RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
                         strcat(GPRS_SendBuffer,Temp_Buffer);
                         
                         break;
 
     case GPRSDATA       :
                         sprintf((char*)GPRS_SendBuffer,"#");                     // START OF STRING #
                         strcat((char*)GPRS_SendBuffer,(char*)GPRSData.IMEI_No); // ADD IMEI NUMBER 
                         strcat((char*)GPRS_SendBuffer,",");
                         if(Flag_APNSet == SET)
                           {   
                             //strcat((char*)GPRS_SendBuffer,GPRSData.Cell_Lat);   // ADD LAT CO-OR. FROM GPRS STRING
                            // strcat((char*)GPRS_SendBuffer,GPRSData.Cell_Long);  // ADD LONG CO-OR. FROM GPRS STRING
                              
                              if(Flag_IRNSSFix)
                              {
                                strcat((char*)TCP_ServerData,IRNSSData.LAT);                               // ADD LAT  IN SERVER SENDING DATA        
                                strcat((char*)TCP_ServerData,IRNSSData.LON);
                              }
                             else
                                {
                                  
                                  strcat((char*)TCP_ServerData,"00.000000,N,");
                                  strcat((char*)TCP_ServerData,"000.000000,E,");    
                                }
                             memset(Temp_Buffer,0,30);
                             sprintf(Temp_Buffer,"%s,%s,%s,%hu",GPRSData.Cell_ID,GPRSData.LAC,GPRSData.Operator,GPRSData.Signal_Strength);
                             strcat((char*)GPRS_SendBuffer,Temp_Buffer);
                           }   
                         else {strcat(GPRS_SendBuffer,"GPRS NOT FIXED YET");}
                                      
                         break;                        
                       
     case LOCATION      :Get_EMGY_SMS(); 
                         strcpy(GPRS_SendBuffer,EM_BUFFER);
                         break;  
                          
     case CONFIG       : memset(GPRS_SendBuffer,0,256);
                         memset(Temp_Buffer,0,10);
                         switch(Protocol)
                         {
                           case BSNL_PC  : sprintf(Temp_Buffer,"BSNL"); break;
                           case HP_PC    : sprintf(Temp_Buffer,"HP");   break;
                           case NIC_OD   : sprintf(Temp_Buffer,"NIC-OD");  break;
                           case NIC_PC   : sprintf(Temp_Buffer,"NIC");  break;
                           case BSNL_MH : sprintf(Temp_Buffer,"BSNL_MH"); break;
                           default : break;
                         }
                         sprintf(GPRS_SendBuffer,"PROT:TCP %s\nAPN:%s\nP:%d,S:%d,T:%d\nPRI:%s:%hu\nSEC:%s:%hu\nTRI:%s:%hu\nFOTA:%d.%d.%d.%d\\%s\nOK",Temp_Buffer,GPRSData.SET_APN,Flag_MAINServer,Flag_EMRServer,Flag_TERServer,&SRVR_IP[MAIN][0],SRVR_Port[MAIN],&SRVR_IP[EMR][0],SRVR_Port[EMR],&SRVR_IP[TER][0],SRVR_Port[TER],DFU_HTTP.IP[0],DFU_HTTP.IP[1],DFU_HTTP.IP[2],DFU_HTTP.IP[3],&DFU_HTTP.PATH[0]);                         
                                               
                         break;
     case CONF        :  // ALL SERVER DETAIL
                          memset(GPRS_SendBuffer,0,strlen(GPRS_SendBuffer));memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                          sprintf(GPRS_MsgBuffer,"P:%s:%d\nS:%s:%d\nT:%s:%d\nPWD:%s,",&SRVR_IP[MAIN][0],SRVR_Port[MAIN],&SRVR_IP[EMR][0],SRVR_Port[EMR],&SRVR_IP[TER][0],SRVR_Port[TER],PWD_IN);
                          
                          strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                          memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                          //sprintf(GPRS_MsgBuffer,"HTTP:%d.%d.%d.%d PATH:%s\nEIP:P:%d,S:%d,T:%d\nUA:%s,CA:%s,SE:%d,P:",DFU_HTTP.IP[0],DFU_HTTP.IP[1],DFU_HTTP.IP[2],DFU_HTTP.IP[3],&DFU_HTTP.PATH[0],Flag_MAINServer,Flag_EMRServer,Flag_TERServer,&GPRSData.APN_Name[0],&GPRSData.SET_APN[0],Flag_SWAP_ENB);
                          switch(Protocol)
                          {
                              case BSNL_PC  : strcat(GPRS_MsgBuffer,"BSNL"); break;
                              case HP_PC    : strcat(GPRS_MsgBuffer,"HP");   break;
                              case NIC_OD   : strcat(GPRS_MsgBuffer,"NIC-OD");  break;
                              case NIC_PC   : strcat(GPRS_MsgBuffer,"NIC");  break;
                              case BSNL_MH  : strcat(GPRS_MsgBuffer,"BSNL_MH"); break;
                              default : break;
                          }

                         
                          strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                          // Remaining Major Size String Like Numbers & 
                          // all hitting intervals
                          // Other Timer/count values 
                          memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                          sprintf(GPRS_MsgBuffer,"\nM0:%s\nM1:%s\nM2:%s\nM3:%s\nM4:%s\nVRN:%s\nA:%d,O:%d,E:%d,H:%d,S:%d,C:%d\nID:%d,SL:%d,OSDT:%d,TT:%d,EBP:%d,EWC:%d,ED:%d,ESM:%d",&Vehicle.Master_No[0][0],&Vehicle.Master_No[1][0],&Vehicle.Master_No[2][0],&Vehicle.Master_No[3][0],&Vehicle.Master_No[4][0],Vehicle.VRN,Active_Time,StandBy_Time,ES_Time,HLTH_FRQ,StopMode_Time,TER_HitTime,IDL_Time,Speed_Limit,OVS_DeadTime,Trans_Time,EMG_BP_FREQ,EMG_WC_Timeout,ED_Time,Flag_STOPMSG);
                         strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                         
                          // mems & sleep time , IBL & other 
                          memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                          sprintf(GPRS_MsgBuffer,"\nMEMS:%d,WK:%3.1f,TA:%d,OX:%d,OY:%d\nHA:%3.1f,HB:%3.1f,RT:%d\nLB:%d,EBL:%d,T2S:%d,SAT:%d,DSF:%u,DSC:%d",Flag_MEMS_ENB,Accel_WK_Thrs,TA,OFFSET_X,OFFSET_Y,HA,HB,RT,IBL,EBL,Time2Sleep,SleepAgainTime,DeepSleepFreq,Flag_DSF_ENB);
                          strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                          break;
                        
     case SYN_ERR        : memset(GPRS_SendBuffer,0,10);
                           sprintf(GPRS_SendBuffer,"ERROR");                         
                          break;
     case GETSTS     : memset(GPRS_SendBuffer,0,250);
                        if(Flag_IRNSSFix){sprintf(GPRS_SendBuffer,"%s\n%s\nGPS:1,%s,%s,%s%d.%d\nIRNSS:%d,%s,%s\nGSM:%d,%d,%d\nTIME:1,%s%s\nL:%d,U:0,W:%d,DS:%d\nOK",GPRSData.IMEI_No,GPRSData.ICCID,Last_Lat,Last_Lon,IRNSSData.SPEED,KM,MTR/100,Flag_IRNSSFix,Last_Lat,Last_Lon,Flag_APNSet,GPRSData.Signal_Strength,Flag_APNSet,IRNSSData.TIME,IRNSSData.DATE,FOTA_IMAGE,Wake_Mode,DS); }                        
                        else{ sprintf(GPRS_SendBuffer,"%s\n%s\nGPS:0,,,,%d.%d\nIRNSS:0,,\nGSM:%d,%d,%d\nTIME:0,,\nL:%d,U:0,W:%d,DS:%d\nOK",GPRSData.IMEI_No,GPRSData.ICCID,KM,MTR/100,Flag_APNSet,GPRSData.Signal_Strength,Flag_APNSet,FOTA_IMAGE,Wake_Mode,DS);}   
                        
                        break;
                        
    case GETVID      : memset(GPRS_SendBuffer,0,100);
                       sprintf(GPRS_SendBuffer,"VEH:%s\nOK",Vehicle.VRN);  
                        
                        break;
                        
    case MOBNUM1     : memset(GPRS_SendBuffer,0,25);
                       sprintf(GPRS_SendBuffer,"MOB:%s\nOK",&Vehicle.Master_No[0][0]);  
                       
                        break;
                        
    case MOBNUM2     : memset(GPRS_SendBuffer,0,25);
                       sprintf(GPRS_SendBuffer,"MOB:%s\nOK",&Vehicle.Master_No[1][0]);  
                       
                        break;

    case MOBNUM3     : memset(GPRS_SendBuffer,0,25);
                       sprintf(GPRS_SendBuffer,"MOB:%s\nOK",&Vehicle.Master_No[2][0]);  
                        
                        break;
                        
    case MOBNUM4     : memset(GPRS_SendBuffer,0,25);
                       sprintf(GPRS_SendBuffer,"MOB:%s\nOK",&Vehicle.Master_No[3][0]);  
                        
                        break;
                        
    case MOBNUM5     : memset(GPRS_SendBuffer,0,25);
                       sprintf(GPRS_SendBuffer,"MOB:%s\nOK",&Vehicle.Master_No[4][0]);  
                       
                        break;
                        
     case IOSTATUS    : memset(GPRS_SendBuffer,0,250);
                        sprintf(GPRS_SendBuffer,"S:%d\nD1:",Flag_Emrgcy); 
                        if((Vehicle.DIN & 0x01) == 0x01)strcat(GPRS_SendBuffer,"1\nI:");
                        else strcat(GPRS_SendBuffer,"0\nI:");
                        if(Vehicle.Ignition[0] == 'I')strcat(GPRS_SendBuffer,"1\nD2:");
                        else strcat(GPRS_SendBuffer,"0\nD2:");
                        if((Vehicle.DIN & 0x02) == 0x02 )strcat(GPRS_SendBuffer,"1\nD3:");
                        else strcat(GPRS_SendBuffer,"0\nD3:");
                        if((Vehicle.DIN & 0x04) == 0x04 )strcat(GPRS_SendBuffer,"1\nTMP:");
                        else strcat(GPRS_SendBuffer,"0\nTMP:");
                        if(Flag_Tampered == SET)strcat(GPRS_SendBuffer,"O\nBV:");
                        else strcat(GPRS_SendBuffer,"C\nBV:");
                        
                        memset(Temp_Buffer,0,10);
                        ftoa(Vehicle.INT_VOLT,Temp_Buffer,2,2);strcat(Temp_Buffer,"\nA1:");strcat(GPRS_SendBuffer,Temp_Buffer);
                        
                        memset(Temp_Buffer,0,10);
                        ftoa(Vehicle.AIN1,Temp_Buffer,2,2);strcat(Temp_Buffer,"\nA2:");strcat(GPRS_SendBuffer,Temp_Buffer);
                        
                        memset(Temp_Buffer,0,10);
                        ftoa(Vehicle.AIN2,Temp_Buffer,2,2);strcat(Temp_Buffer,"\nMV:");strcat(GPRS_SendBuffer,Temp_Buffer);
                        
                        memset(Temp_Buffer,0,10);
                        ftoa(Vehicle.EXT_VOLT,Temp_Buffer,2,2);strcat(Temp_Buffer,"\nDT:");strcat(GPRS_SendBuffer,Temp_Buffer);
                        
                        memset(Temp_Buffer,0,5);
                        sprintf(Temp_Buffer,"%d",Flag_EMRGY_TMP);
                        strcat(GPRS_SendBuffer,Temp_Buffer);
                        strcat(GPRS_SendBuffer,"\nDO1:");
                        if((Vehicle.DOUT & 0x01) == 0x01)strcat(GPRS_SendBuffer,"0\nDO2:");                            
                        else strcat(GPRS_SendBuffer,"1\nDO2:");
                        if((Vehicle.DOUT & 0x02) == 0x02)strcat(GPRS_SendBuffer,"0\nOK");                            
                        else strcat(GPRS_SendBuffer,"1\nOK");

                        
                        break;
                        
     case HW_VER     :  memset(GPRS_SendBuffer,0,30);  
                        sprintf(GPRS_SendBuffer,"HW\nRev:%s\nDate:%s\nSW\nV-%s\nDate:%s\nOK",HW_Ver,HW_DATE_TIME,SW_Ver,SW_Date); 
                        break;
     case SMSPKT     :  memset(GPRS_SendBuffer,0,256); 
                        if(Flag_IRNSSFix)
                         {
                           sprintf(GPRS_SendBuffer,"%s,%s%s1,%s%s,%s",GPRSData.IMEI_No,IRNSSData.LAT,IRNSSData.LON,IRNSSData.SPEED,GPRSData.Cell_ID,GPRSData.LAC);
                         }
                         else 
                         {
                           sprintf(GPRS_SendBuffer,"%s,000.000000,0,000.000000,0,0,0,%s,%s",GPRSData.IMEI_No,GPRSData.Cell_ID,GPRSData.LAC); 
                         }
                         strcat(GPRS_SendBuffer,"!\nOK");
                              
                        break; 
     case GEOFENCE   :  memset(GPRS_SendBuffer,0,250);
                        sprintf(GPRS_SendBuffer,"R:%d\nI:%hu\nLAT:- %9.6f\nLON:- %9.6f\nOK",R_ID,FNC_ID,GF[R_ID].LAT[FNC_ID],GF[R_ID].LON[FNC_ID]);              
                        break;
                        
     case DEBUG      :  memset(GPRS_SendBuffer,0,strlen(GPRS_SendBuffer));
                        memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                        if(Flag_IRNSSFix == RESET){sprintf(IRNSSData.HDOP,"N.A");IRNSSData.Used_Satellite = 0;}
                        if((strstr(&GPRSData.Operator[0],"N.A")) || (GPRSData.Operator[0] == 0))
                        {
                           sprintf(GPRS_MsgBuffer,"%s\nF:%d,I:%d,US:%d,IVS:%d,GVS:%d,H:%s\nF:%d,S:%d,O:%s,SP:x\nUR:%d,P:%d,%02hu:%02hu:%02hu,B:%d,S:%d,%02hu:%02hu:%02hu,B:%d,T:%d,%02hu:%02hu:%02hu,B:%d\nMMRY:%d,MEMS:%d\nDIP ",GPRSData.IMEI_No,Flag_IRNSSFix,Flag_IRNSSFix,IRNSSData.Used_Satellite,IRNSS_Visible_Satellite,GPS_Visible_Satellite,IRNSSData.HDOP,Flag_GPRSFix,GPRSData.Signal_Strength,GPRSData.Operator,Current_Hit_Time,Flag_TCPConnect[MAIN],Last_HitTime[MAIN][0],Last_HitTime[MAIN][1],Last_HitTime[MAIN][2],TCP_BackCnt[MAIN],Flag_TCPConnect[EMR],Last_HitTime[EMR][0],Last_HitTime[EMR][1],Last_HitTime[EMR][2],EMRGY_BKP_No,Flag_TCPConnect[TER],Last_HitTime[TER][0],Last_HitTime[TER][1],Last_HitTime[TER][2],TCP_BackCnt[TER],Flag_SPIOK,Flag_MEMSOK);
                        }
                        else sprintf(GPRS_MsgBuffer,"%s\nF:%d,I:%d,US:%d,IVS:%d,GVS:%d,H:%s\nF:%d,S:%d,O:%s,SP:%d\nUR:%d,P:%d,%02hu:%02hu:%02hu,B:%d,S:%d,%02hu:%02hu:%02hu,B:%d,T:%d,%02hu:%02hu:%02hu,B:%d\nMMRY:%d,MEMS:%d\nDIP ",GPRSData.IMEI_No,Flag_IRNSSFix,Flag_IRNSSFix,IRNSSData.Used_Satellite,IRNSS_Visible_Satellite,GPS_Visible_Satellite,IRNSSData.HDOP,Flag_GPRSFix,GPRSData.Signal_Strength,GPRSData.Operator,Flag_SIM2,Current_Hit_Time,Flag_TCPConnect[MAIN],Last_HitTime[MAIN][0],Last_HitTime[MAIN][1],Last_HitTime[MAIN][2],TCP_BackCnt[MAIN],Flag_TCPConnect[EMR],Last_HitTime[EMR][0],Last_HitTime[EMR][1],Last_HitTime[EMR][2],EMRGY_BKP_No,Flag_TCPConnect[TER],Last_HitTime[TER][0],Last_HitTime[TER][1],Last_HitTime[TER][2],TCP_BackCnt[TER],Flag_SPIOK,Flag_MEMSOK);
                         
                        strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                        memset(GPRS_MsgBuffer,0,strlen(GPRS_MsgBuffer));
                        /*----- DIGITAL INPUT STATUS ----------------------------*/
                        if(Vehicle.Ignition[0] == 'I')strcat(GPRS_MsgBuffer,"I:1,");               // IGINITION ON
                        else strcat(GPRS_MsgBuffer,"I:0,");                                        // IGINITION OFF
                        if((Vehicle.DIN & 0x01) == 0x01)strcat(GPRS_MsgBuffer,"DI:1");             // DIGITAL INPUT 1-ON        
                        else strcat(GPRS_MsgBuffer,"DI:0");                                        // DIGITAL INPUT 1-OFF
                        if((Vehicle.DIN & 0x02) == 0x02)strcat(GPRS_MsgBuffer,"1");                // DIGITAL INPUT 2-ON
                        else strcat(GPRS_MsgBuffer,"0");                                           // DIGITAL INPUT 2-OFF
                        if((Vehicle.DIN & 0x04) == 0x04)strcat(GPRS_MsgBuffer,"1");                // DIGITAL INPUT 2-ON
                        else strcat(GPRS_MsgBuffer,"0");                                           // DIGITAL INPUT 2-OFF
                        if((Vehicle.DIN & 0x08) == 0x08)strcat(GPRS_MsgBuffer,"1,");                 // DIGITAL INPUT 2-ON
                        else strcat(GPRS_MsgBuffer,"0,");                                          // DIGITAL INPUT 2-OFF
                        
                        /*----- DIGITAL OUTPUT STATUS ---------------------------*/
                        if((Vehicle.DOUT & 0x01) == 0x01)strcat(GPRS_MsgBuffer,"DO:1");                    // DIGITAL OUTPUT 1-ON        
                        else strcat(GPRS_MsgBuffer,"DO:0");                                           // DIGITAL OUTPUT 1-OFF
                        if((Vehicle.DOUT & 0x02) == 0x02)strcat(GPRS_MsgBuffer,"1\n");                    // DIGITAL OUTPUT 2-ON
                        else strcat(GPRS_MsgBuffer,"0\n");                                            // DIGITAL OUTPUT 2-OFF
                           
                        /*----- POWER STATUS ------------------------------------*/ 
                        if(Vehicle.Battery[0] == 'C')strcat(GPRS_MsgBuffer,"PWR-E,");               // EXTERNAL POWER CONNECTED
                        else strcat(GPRS_MsgBuffer,"PWR-I,");                                       // INTERNAL POWER CONNECTED
                         
                        /*----- INTERNAL BATTERY STATUS -------------------------*/
                        strcat(GPRS_MsgBuffer,"IBV:");
                        memset(Temp_Buffer,0,30);
                        ftoa(Vehicle.INT_VOLT,Temp_Buffer,1,2);strcat(Temp_Buffer,"V,");
                        strcat(GPRS_MsgBuffer,Temp_Buffer);
                          
                        /*----- EXTERNAL BATTERY STATUS -------------------------*/
                        strcat(GPRS_MsgBuffer,"EBV:");
                        memset(Temp_Buffer,0,30);
                        ftoa(Vehicle.EXT_VOLT,Temp_Buffer,2,1);strcat(Temp_Buffer,"V,");
                        strcat(GPRS_MsgBuffer,Temp_Buffer);
                         
                        /*----- ANALOG INPUT STATUS & SOFTWARE VERSION ----------*/
                        strcat(GPRS_MsgBuffer,"AIN:");
                        memset(Temp_Buffer,0,30);
                        ftoa(Vehicle.AIN1,Temp_Buffer,2,1);strcat(Temp_Buffer,"V,");
                        strcat(GPRS_MsgBuffer,Temp_Buffer);
                         
                        memset(Temp_Buffer,0,30);
                        ftoa(Vehicle.AIN2,Temp_Buffer,2,1);strcat(Temp_Buffer,"V\n");
                        strcat(GPRS_MsgBuffer,Temp_Buffer);
                         
                        memset(Temp_Buffer,0,30);
                        sprintf(Temp_Buffer,"ODO:%d.%d,X:%d,Y:%d,E:%d",KM,MTR/100,ACTUAL_RX,ACTUAL_PY,Flag_Emrgcy);
                         
                        strcat(GPRS_MsgBuffer,Temp_Buffer);  
                        strcat(GPRS_MsgBuffer,",SW-");  
                        strcat(GPRS_MsgBuffer,SW_Ver);  
                        strcat(GPRS_SendBuffer,GPRS_MsgBuffer);
                        break;
                     
                    
    case UNATHU         : sprintf(GPRS_SendBuffer,"Unathurized Access");               // UNAUTHORIZED ACCESS 
                          break;
                     
    case UPGRADE        : sprintf(GPRS_SendBuffer,"Firmware Upgrade Requested");       // FIRMWARE UPGRADATION REQUEST
                          break; 
                     
    case DFU            : sprintf(GPRS_SendBuffer,"Firmware Upgraded successfully");     
                          
                         break;                 
    case OP1_STARTED    : sprintf(GPRS_SendBuffer,"OUTPUT 1 STARTED");break;    // DIGITAL OUTPUT 1 START
    case OP2_STARTED    : sprintf(GPRS_SendBuffer,"OUTPUT 2 STARTED");break;    // DIGITAL OUTPUT 2 START 
    case OP1_STOPPED    : sprintf(GPRS_SendBuffer,"OUTPUT 1 STOPPED");break;    // DIGITAL OUTPUT 1 STOP 
    case OP2_STOPPED    : sprintf(GPRS_SendBuffer,"OUTOUT 2 STOPPED");break;    // DIGITAL OUTPUT 2 STOP
    case ACKN           : sprintf(GPRS_SendBuffer,"OK");break;                  // REQUEST RECEIVED ACKNOWLEDGMENT
    
     case INFO          : sprintf(GPRS_SendBuffer,"IMEI-%s\nSIM- %s\nVID-%s, VRN-%s \nHDR-%s",GPRSData.IMEI_No,GPRSData.ICCID,VENDOR_ID, Vehicle.VRN,HEADER_ID); 
                          break;


     case EMG          :  Get_EMGY_SMS();
                          strcpy(GPRS_SendBuffer,EM_BUFFER);  
                          
                          break;                         
                          
      
    case DFU_DETAIL    :  if(DFU_HTTP.URL[0] != 'h')
                          sprintf(DFU_HTTP.URL,"http://%d.%d.%d.%d/%s/%s.bin",DFU_HTTP.IP[0],DFU_HTTP.IP[1],DFU_HTTP.IP[2],DFU_HTTP.IP[3],&DFU_HTTP.PATH[0],&DFU_HTTP.FILENAME[0]);
    
                          sprintf(GPRS_SendBuffer,"DFU DETAIL IP-%hu.%hu.%hu.%hu,\n PATH- %s, \n FILE-%s,\nURL-%s",DFU_HTTP.IP[0],DFU_HTTP.IP[1],DFU_HTTP.IP[2],DFU_HTTP.IP[3],&DFU_HTTP.PATH[0],&DFU_HTTP.FILENAME[0],&DFU_HTTP.URL[0]);                         
                          break;
                          
    case DFU_DNF        : sprintf(GPRS_SendBuffer,"DFU details not found");       // FIRMWARE UPGRADATION REQUEST
                          break;  
                          
                          
     case HEALTH      :   Get_HLTH_PKT(); 
                          strcpy(GPRS_SendBuffer,HP_BUFFER); 
                          strcat(GPRS_SendBuffer,"\nOK");      
                          break; 
     case PARAMS      :   sprintf(GPRS_SendBuffer,"UI\nA:%hu,O:%hu,H:%hu,E:%hu,EBP:%hu,EWC:%hu,T2S:%hu,S:%hu,C:%hu,OSDT:%hu\nTD\nLB:%hu,HA:%0.2f,HB:%0.2f,RT:%d,SL:%hu,WK:%1.1f,ID:%hu\nOK",Active_Time,StandBy_Time,HLTH_FRQ,ES_Time,EMG_BP_FREQ,EMG_WC_Timeout,Time2Sleep,StopMode_Time,TER_HitTime,OVS_DeadTime,IBL,HA,HB,RT,Speed_Limit,Accel_WK_Thrs,IDL_Time);                         
                          break;
                          
     case UI          :   sprintf(GPRS_SendBuffer,"UI\nA:%hu,O:%hu,H:%hu,E:%hu,ET:%hu,ST:%hu,TS:%hu,S:%hu,C:%hu,OI:%hu",Active_Time,StandBy_Time,HLTH_FRQ,ES_Time,EMG_BP_FREQ,EMG_WC_Timeout,Time2Sleep,StopMode_Time,TER_HitTime,OVS_DeadTime);                         
                          break; 
     case TD          :   sprintf(GPRS_SendBuffer,"TD\nLB:%hu,HA:%0.2f,HB:%0.2f,RT:%d,SL:%hu,WK:%1.1f,ID:%hu\nOK",IBL,HA,HB,RT,Speed_Limit,Accel_WK_Thrs,IDL_Time);                         
                          break;
     case IP1         :   sprintf(GPRS_SendBuffer,"IP1-%s:%d",SRVR_IP[MAIN],SRVR_Port[MAIN]);                         
                          break;    
     case IP2         :   sprintf(GPRS_SendBuffer,"IP2-%s:%d",SRVR_IP[EMR],SRVR_Port[EMR]);                         
                          break;    
     case IP3         :   sprintf(GPRS_SendBuffer,"IP3-%s:%d",SRVR_IP[TER],SRVR_Port[TER]);                         
                          break;    
                          
     case HLTH_ACTV   :  memset(GPRS_SendBuffer,0,100);
                         Send_ACTV_HCHK_Resp();
                         strcat(GPRS_SendBuffer,"\nOK");
                         strcpy(GPRS_SendBuffer,RESP_BUFFER);
                         break; 
                          
     case GEO_FENCE   :  sprintf(GPRS_SendBuffer,"%d\n",R_ID);
                         for(GP_Counter = 0; GP_Counter < 10; GP_Counter++)
                         {
                           if(GSM_STATUS == SET){strcat(GPRS_SendBuffer,"\n");Serial_PutString_GPRS(GPRS_SendBuffer);}
                           else Serial_PutString_232(GPRS_SendBuffer,3);
                          
                           memset(GPRS_SendBuffer,0,strlen(GPRS_SendBuffer));
                           sprintf(GPRS_SendBuffer,"%d,%10.6f,%10.6f\n",GP_Counter,GF[R_ID].LAT[GP_Counter],GF[R_ID].LON[GP_Counter]);
                         }
                         
                         break; 
     case WARNING       : sprintf(GPRS_SendBuffer,"FOTA is restricted due to low Internal Battery voltage");break;                   
     case INVALID       : sprintf(GPRS_SendBuffer,"INVALID COMMAND");
                          break;
     case FLASH_ERR     : sprintf(GPRS_SendBuffer,"FLASH ERROR");break;
     case NA            : sprintf(GPRS_SendBuffer,"NOT APPLICABLE");break;
     
                         
     default: break;               
      };
  
     if(GSM_STATUS == SET)
     {
       Serial_PutString_GPRS(GPRS_SendBuffer);
       mSec_Delay(100);
       Serial_SendData_GPRS(13);Serial_SendData_GPRS(10);Serial_SendData_GPRS(26);// TERMINATE STRING OVER GPRS BY SENDING CR,LF & END OF QISEND COOMAND 
       mSec_Delay(1000);                                                          
       
              
     UpLoad_Time = 30;Ptr = NULL;Flag_Error = RESET; if(Protocol == HP_PC)Flag_ReadData = SET; else Flag_ReadData = RESET;                           // WAIT FOR SMS SENDING  
     while(Ptr == NULL && (UpLoad_Time > 0) && Flag_Error == RESET)             // WAIT TILL SMS SENT
     {
      if(Ptr = strstr((char*)GPRS_Buffer,"GS:"))
      {
        switch(SMS_ID)
        {
          case 1 : Flag_SMS_EMRN = RESET; break;
          case 2 : Flag_SMS_EMRF = RESET; break;
          case 3 : Flag_SMS_OVS = RESET; break;
          case 4 : Flag_SMS_TL = RESET; break;
          case 5 : Flag_SMS_GE = RESET; break; 
          case 6 : Flag_SMS_GX = RESET; break;
          case 7 : Flag_SMS_MBR = RESET; break;
          case 8 : Flag_SMS_RNC = RESET; break;
          case 9 : Flag_SMS_DT = RESET; break;
          default :break;
        }
        UpLoad_Time = 0;
        break;
      }
      else if(Ptr = strstr((char*)GPRS_Buffer,"CMS"))
      {
        UpLoad_Time=0; 
        break;
      }
      else if(Ptr = strstr((char*)GPRS_Buffer,"sms error"))
      {
        UpLoad_Time=0; 
        break;
      }
      if(Flag_Second == SET)
        {
          Flag_Second = RESET;UpLoad_Time--;IWDG_ReloadCounter();
          if(GNSS_Time <= 1 ){GNSS_Time = GNSS_INTERVAL;if(Flag_IRNSSFix){Get_Location();}};
        
           if(Flag_SerialConfig == SET)
           {
             Check_Config();
             Flag_SerialConfig = RESET;
           }
        };
     }
     Flag_ReadData = RESET;

     mSec_Delay(100);
     Modem_PutString("AT+CMGD=1,4");WAIT_MODEM_RESP(2);
   }
     else Serial_PutString_232(GPRS_SendBuffer,3);
     
   
   
SKIP:
     if(Flag_GPRSUpdate == RESET)GPRS_Count = 0;     
}
/*******************************************************************************
* Function Name : Get_CellLocation
* Description   : Reads Location info from LOC String
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Get_CellLocation(void)
{
 char *Ptr;
 Ptr = NULL;
 //LatPtr = NULL;
 GP_Counter = 0;
 
 if(Flag_ValidNetwork == SET && Flag_APNSet == RESET)Set_APN();                 // IF APN NOT SET GOTO SET APN FUNCTION
 if(Flag_ValidNetwork && Flag_APNSet)                                           // IF VALID NETWORK & APN SET
 {

 /*****************************************************************************/
  Flag_GPRSFix = SET;
   
  if(strlen(GPRS_Buffer) < 500)
  {
   memset(GPRS_Buffer,0,strlen(GPRS_Buffer));
   }
  else 
  {
      memset(GPRS_Buffer,0,490);
   }
  GPRS_Count = 0;
  
  Modem_PutString("AT+CCLK?"); WAIT_MODEM_RESP(2);
  
  if(Ptr = strstr((char*)GPRS_Buffer,"CLK:")) 
  {
   Ptr+=6;                                                                 // SET POINTER TO IDENTITY SUCCESS 
   
   if((*Ptr > 47) && (*Ptr < 58))
   {
   /*----- EXTRACT CELL DATE -----------------------------------------------*/
    GPRSData.Cell_Date[6] = *Ptr;Ptr++;                                 /********************************/
    GPRSData.Cell_Date[7] = *Ptr;Ptr += 2;                                 /*      GPRS CELL DATE FORMATE  */
    GPRSData.Cell_Date[2] = *Ptr;Ptr++;                                 /*      YYYY/MM/DD              */
    GPRSData.Cell_Date[3] = *Ptr;Ptr += 2;                                // HERE POINTER +1 JUMP FOR '/' CHARACTER  
    GPRSData.Cell_Date[0] = *Ptr;Ptr++;                                 
    GPRSData.Cell_Date[1] = *Ptr;Ptr+=2;                                // HERE POINTER +1 JUMP FOR '/' CHARACTER 
    GPRSData.Cell_Date[4] = '2';                                 
    GPRSData.Cell_Date[5] = '0';  
    GPRSData.Cell_Date[8] = ',';
    GPRSData.Cell_Date[9] = 0;
   
    memcpy(IRNSSData.DATE,GPRSData.Cell_Date,10);       // SAVE DATE IN GPS DATA 
         
    /*----- TIME------------------------------------------------------------*/
    GPRSData.Cell_Time[0] = *Ptr;Ptr++;
    GPRSData.Cell_Time[1] = *Ptr;Ptr+=2;                                // POINTER +1 JUMP FOR SKIP SEPARATOR
    GPRSData.Cell_Time[2] = *Ptr;Ptr++;
    GPRSData.Cell_Time[3] = *Ptr;Ptr+=2;                                // POINTER +1 JUMP FOR SKIP SEPARATOR                              
    GPRSData.Cell_Time[4] = *Ptr;Ptr++;
    GPRSData.Cell_Time[5] = *Ptr;Ptr+=2;                                
    GPRSData.Cell_Time[6] = 0;
    GPRSData.Cell_Time[7] = 0;
    Flag_GPRSTimeFix = SET;                                                   // CALL GET TIME FUNTION TO UPDATE VALID TIME FROM EITHER GPS, GNSS OR GPRS  
   
    SET_RTC();
    
   }
  };                                                                          

   memset(GPRS_Buffer,0,490);
 }                                                                        // END OF MODEM OK
                                            
}
/*******************************************************************************
* Function Name : Get_Backup
* Description   : Get Backup of TCP server data @ defined time interval on FLASH 
* Input         : SERVER 
* Output        : None
* Return        : None
*******************************************************************************/
void Get_Backup(uint8_t SRVR)
{
  char loc_buffer[50];
  char *Ptr;
  Ptr = NULL;
  uint32_t FlashAddress;
  
  FlashAddress = 0;

  memset(loc_buffer,0,50);
  Ptr = &TCP_BACKUP[0];
  if(Verify_UploadData(Ptr))
  {
  /*----- AS PER PROTOCOL L- LIVE H-HISTORY ----------------------------------*/
    
  if(Ptr = strstr((char*)TCP_BACKUP,",CC,12,"));                                // SKIP CONFIGURATION PACKET
  else if((Ptr = strstr(&TCP_BACKUP[5],",L,")))
    {
      *(Ptr+1) = 'H';                                                           // REPLACE LIVE 'L' TO'H' INDICATION FOR HISTORY TO WRITE ON FLASH ONLY ONCE 
     
        if(Ptr = strstr((char*)TCP_BACKUP,",NR,1,"))
        {
          Ptr += 4;
          *Ptr = '2';
        }  
      
      if(SRVR == MAIN){FlashAddress = SPIFLASH_BAKUP1 + (TCP_BackCnt[MAIN]* 0x100); } // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH    
      else if(SRVR == TER) {FlashAddress = SPIFLASH_BAKUP2 + (TCP_BackCnt[TER]* 0x100);}
                    
      TCP_BackCnt[SRVR]++;                                                          // INCREMENT TCP PACKETS NUMBER 
      if(TCP_BackCnt[MAIN] > 40000)    TCP_BackCnt[MAIN] = 40001;
      else if(TCP_BackCnt[TER] > 20000)TCP_BackCnt[TER]  = 20001;                   // LIMIT TO MAX 15000 PACKETS 
      
      if(Flag_SPIOK == SET)                                                     // CHECK SERIAL FLASH STATUS
        {
         if(FlashAddress % 4096 == 0)SPI_FLASH_SectorErase(FlashAddress);           // IF ADDRESS IS ENDING ADDRESS ERASE ALL PAGES
         SPI_FLASH_PageWrite((char*)TCP_BACKUP,FlashAddress,256);                   // WRITE BACK UP ON FLASH MEMORY
        
         /*----- WRITE TO FLASH AFTER EVERY 10 COUNTS ----------------------------*/
         if(TCP_BackCnt[SRVR] % 16 == 0)                                             // A COMPLETE SECTOR 
          {
           FlashAddress = SPIFLASH_CONFIG;                                          // SET FLASH ADDRESS TO 0
           sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
            
           SPI_FLASH_SectorErase(FlashAddress);                                     // ERASE SECTION OF FLASH TO WRITE NEW DATA
           SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));// WRITE GPRS SEND BUFFER DATA TO FLASH PAGES 
          };
       };
      RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER])); // TAKE BACK UP OF COUNTS/PACKETS
     
    }
  }
}
/*******************************************************************************
* Function Name : Get_RAMBackup
* Description   : Get temporary backup in RAM before hitting server 
* Input         : SERVER 
* Output        : None
* Return        : None
*******************************************************************************/
void Get_RAMBackup(uint8_t SRVR)
{
  char *Ptr;
  Ptr  = NULL;
  Ptr = &RAM_BACKUP[SRVR][RAM_BakCnt[SRVR]][0];                                 // TAKE ADDRESS OF LOCAL BACK UP REGISTER
  RAM_BakCnt[SRVR]++;                                                           // INCREMENT REGISTER 
  if(RAM_BakCnt[SRVR] > 19)
  {
    memcpy(Ptr,TCP_ServerData,256);
    Take_Backup(SRVR);
  }                                                                             // TAKE BACKUP OF ALL RAM DATA INTO EXTERNAL FLASH
  else memcpy(Ptr,TCP_ServerData,256);                                              // CONTINUE TAKING BACK UP TILL LAST AVAILABLE LOCAL BACK UP REGISTER 

  if(strstr(TCP_ServerData, "CC,12") && SRVR == TER) RAM_BakCnt[SRVR]++;  // Exta Config Data
  if(strstr(TCP_ServerData, "OT,12") && SRVR == MAIN) RAM_BakCnt[SRVR]++;   
}
/*******************************************************************************
* Function Name : Gen_PKT
* Description   : GENERATE TCP PACKET OF BOTH SERVERS  
* Input         : SERVER 
* Output        : None
* Return        : None
*******************************************************************************/
void Gen_PKT(void)
{
  if(GPRSData.IMEI_No[0] == 'N')return; // no need to generate packet without IMEI       
  Flag_PKTErr = RESET;
      
  if(Flag_MAINServer == RESET)Flag_MAIN_PKT = RESET;
  if(Flag_TERServer  == RESET)Flag_TER_PKT  = RESET;

   
   if(Flag_DeepSleep == SET){Flag_MAIN_PKT = SET;Flag_TER_PKT = SET;}
   if((Flag_MAINServer == SET) && (Flag_MAIN_PKT == SET))
   {
     ServerHit_Time = Current_Hit_Time-1;
     Flag_MAIN_PKT = RESET; 
     Set_TCPData(MAIN);                                                             // SET TCP SERVER DATA TO SERVER VIA GPRS 
     if(Flag_PKTErr == RESET)Get_RAMBackup(MAIN);
   }                                                                        // GENERATE A PACKET OF PRIMARY SERVER 
   if((Flag_TERServer  == SET) && (Flag_PKTErr == RESET) && ((Flag_TER_PKT  == SET) || (AID > 1)))
   {
     Flag_TER_PKT = RESET; 
     if(Vehicle.Ignition[0] == 'I')TER_ServerHit_Time = TER_HitTime-1;                                 // RELOAD SERVER HIT TIME 
     else TER_ServerHit_Time = Current_Hit_Time-1;
      
     switch(AID)
     {
       case 0  : Flag_HealthCheck  = SET; break;
       case 3  : Flag_Remove       = SET; break;
       case 4  : Flag_BattLo_Alert = SET; break;
       case 5  : Flag_IBVChg       = SET; break;
       case 6  : Flag_Power_RNC    = SET; break;
       case 7  : Flag_IgnOn        = SET; break;
       case 8  : Flag_IgnOff       = SET; break;
       case 9  : Flag_BoxTemper    = SET; break;
       case 10 : Flag_EA           = SET; break; 
       case 11 : Flag_EA_OFF       = SET; break;
       case 12 : Flag_OTACFG       = SET; break;
       case 13 : Flag_HB           = SET; break;
       case 14 : Flag_HA           = SET; break;
       case 15 : Flag_RT           = SET; break;
       case 16 : Flag_EWT          = SET; break;
       case 17 : Flag_OVS          = SET; break;
       case 18 : Flag_GEOEntry     = SET; break;
       case 19 : Flag_GEOExit      = SET; break;
       case 20 : Flag_ID           = SET; break;
       case 22 : Flag_TILT_Alert   = SET; break;
       case 59 : Flag_BoxTemper_Closed = SET; break;
       case 66 : Flag_DTC          = SET; break;
       default : break;
     }
     Set_TCPData(TER);                                                             // SET TCP SERVER DATA TO SERVER VIA GPRS 
     if(Flag_PKTErr == RESET)Get_RAMBackup(TER); 
     
     if(Vehicle.Ignition[0] == 'I')TER_ServerHit_Time = TER_HitTime-1;                                 // RELOAD SERVER HIT TIME 
     else TER_ServerHit_Time = Current_Hit_Time-1;
   }                                                                        // CREATE A PACKET OF SECONDARUY SERVER 
   if(Flag_PKTErr == RESET)Flag_HitServ = SET;                                                        // READY TO HIT SERVERS
    
    if(Flag_PKTErr == SET)
    {
      Flag_PKTErr = RESET;
      memset(TCP_ServerData,0,256);
    }
    
    AID = 1;
    if(Flag_DIN1)Flag_DIN1 = RESET;
    if(Flag_DIN2)Flag_DIN2 = RESET;
    if(Flag_DIN3)Flag_DIN3 = RESET;
    if(Flag_DIN4)Flag_DIN4 = RESET;
    

}
/*******************************************************************************
* Function Name : Set_APN
* Description   : Set Access Point Name based on service provider
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Set_APN(void)
{
  char *Ptr;
  Ptr = 0;
  
  memset(GPRS_Buffer,0,50);GPRS_Count = 0;
  
  if(Flag_ModemOK == SET){ memset(GPRS_Buffer,0,50);GPRS_Count = 0; Modem_PutString("AT+QSPN");WAIT_MODEM_RESP(2);}
 
  if(Ptr = strstr((char*)GPRS_Buffer,"PN:")) 
  {
     Ptr += 4;
     if(*Ptr == 0x22)
     {
       Ptr += 1;
       for(GP_Counter = 0;GP_Counter < 10; GP_Counter ++,Ptr++)
       {
          if(*Ptr == 0x22){GPRSData.Operator[GP_Counter] = 0; break;}
          else 
            {
             GPRSData.Operator[GP_Counter] = *Ptr;
             if(islower(GPRSData.Operator[GP_Counter]))GPRSData.Operator[GP_Counter] = toupper(GPRSData.Operator[GP_Counter]);
            };
        };
    };     
    
   memset(GPRS_Buffer,0,50);GPRS_Count = 0;
   char APN[100] = {'\0'};
   
   // GET APN Name from Operator
   if((strstr((char*)GPRSData.SET_APN,"no-apn")) || (strstr((char*)GPRSData.SET_APN,"NO-APN")))  
   {
    if(strstr((char*)GPRSData.Operator,"AIRTEL"))sprintf(APN,"taisysnet");
    else if(strstr((char*)GPRSData.Operator,"BSNL"))sprintf(APN,"bsnlnet");
    else if(strstr((char*)GPRSData.Operator,"VODAFONE"))sprintf(APN,"m2misafe");
   }
   else  sprintf(APN,"%s",GPRSData.SET_APN);
   
   // Request APN Name to check
   Modem_PutString("AT+CGDCONT?");WAIT_MODEM_RESP(2);
   if(Ptr = strstr((char*)GPRS_Buffer,"CONT: "))
   {
      if(Ptr = strstr((char*)GPRS_Buffer,APN))
      {
        Flag_APNSet = SET;
        memset(GPRSData.APN_Name,0,19);
        for(GP_Counter = 0;GP_Counter < 19; GP_Counter ++,Ptr++)
        {
          if (*Ptr == 0x22){GPRSData.APN_Name[GP_Counter] = 0; break;}
          else GPRSData.APN_Name[GP_Counter] = *Ptr;
        };
      }
    };
  
   // If not set then SET APN Name
   if(Flag_APNSet == RESET)
   {
     memset(GPRS_MsgBuffer,0,sizeof(GPRS_MsgBuffer));
     sprintf(GPRS_MsgBuffer,"AT+CGDCONT=1,\"IP\",\"%s\"",APN);
     Modem_PutString(GPRS_MsgBuffer); WAIT_MODEM_RESP(2);
   }
 }
  
  Flag_ModemOK = RESET;
}


/*******************************************************************************
* Function Name : Wait Modem Response
* Description   : Waits for the Modem Response
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void WAIT_MODEM_RESP(uint8_t Wait_Time)                           
{
  
  char *Ptr;
  Ptr = 0;
  
  Flag_Error = RESET; Flag_ModemOK = RESET;         // RESET ALL MODEM RELATED FLAGS
  while(Flag_ModemOK == RESET && Wait_Time > 0 && Flag_Error == RESET)
  {
     if(Flag_Second == SET)
     {
       IWDG_ReloadCounter();
       Flag_Second = RESET; Wait_Time--;               // DECREMENT UPLOAD TIME 
       if(Flag_SerialConfig == SET){ Check_Config();Flag_SerialConfig = RESET;}
     };
     if(Flag_KeyScan == SET && Flag_SelfTest == RESET){Flag_KeyScan = RESET;Key_InputScan(); }             // KEY SCAN EVERY 200 MILI SECONDS
     
     // Send OK or OK 
     if(Ptr = strstr((char*)GPRS_Buffer,"OK")){Flag_ModemOK = SET;GPRS_Count = 0;Modem_NoResp = 0;}
     if(Ptr = strstr((char*)GPRS_Buffer,"\rK\r")){Flag_ModemOK = SET;GPRS_Count = 0;Modem_NoResp = 0;}
     // Send Fail or Fail
     else if(Ptr = strstr((char*)GPRS_Buffer,"FAIL")){Flag_Error = SET; Flag_ModemOK = RESET;GPRS_Count = 0;};
  
     if(Ptr != NULL){*Ptr = ' '; Ptr++;*Ptr = ' '; Ptr = NULL;}; // INSERT 2 BLANKS 
  };
  if(Wait_Time == 0){Modem_NoResp++;Flag_Error = SET;};                       // UPLOAD ERROR 
  mSec_Delay(50);
  
}

/*******************************************************************************
* Function Name : Check_Signal_Nw
* Description   : Check Signal Strength & Network Parameters
* Input         : Parameter name & place
* Output        : Parameter new value
* Return        : Paremeter
*******************************************************************************/
void Check_Signal_Nw_GPRS(void)
{
  if(Response_Time[MAIN] > 100 || Response_Time[TER] > 100)return;
  char *Ptr;     
  Ptr = 0; DPtr = 0;

  Flag_ValidNetwork = RESET;                                                    // TO AUTHENTICATE CORRECT EXECUTION
  GPRS_Count = 0;
  Modem_PutString("AT+CSQ");WAIT_MODEM_RESP(2);                                 // CHECK SIGNAL STRENGTH 
  Flag_CSQErr = SET;
  
  /*----- GET SIGNAL STRENGTH & REQUEST NETWORK INFORMATION ------------------*/
  if(Flag_ModemOK == SET)
    {
      Flag_ModemOK = RESET;
      if(Ptr = strstr((char*)GPRS_Buffer,"Q:"))
       {
         Ptr += 3;GPRSData.Signal_Strength = atoi(Ptr);                         // EXTRACT SIGNAL STRENGTH
         if(GPRSData.Signal_Strength >= 35 )GPRSData.Signal_Strength = 0;       // SET SIGNAL STRENGTH TO 0 IF INVALID/GARBAGE
         if(GPRSData.Signal_Strength >= VALID_SIGNAL )                          // CHECK AUTHENTICATION OF SIGNAL STRENGTH
         {
           Flag_CSQErr = RESET;
           Flag_GetCGREG = SET;
           GPRS_Buffer[15] = 0;
           GPRS_Buffer[16] = 0;
           GPRS_Buffer[498] = 0;
           GPRS_Buffer[499] = 0;
           GPRS_Count = 0;
           Modem_PutString("AT+CGREG?");WAIT_MODEM_RESP(2);
         } 
       }
      else Flag_ModemOK = RESET;
      
      if(Ptr = strstr((char*)GPRS_Buffer,"Q:")){*Ptr = ' ';Ptr++;*Ptr = ' ';};// INSERT BLANK 
    }
  
  /*----- GET CELL ID & LOCATION & GPRS AVAILABILITY -------------------------*/
  if(Flag_ModemOK == SET)                                                                       
    {  
      if(Ptr = strstr((char*)GPRS_Buffer,"EG:"))
      {
         Ptr += 4; Flag_GetCGREG = RESET;
         if(*Ptr != '2')
         {
            GPRS_Buffer[15] = 0;
            GPRS_Buffer[16] = 0;
            GPRS_Buffer[498] = 0;
            GPRS_Buffer[499] = 0;
            GPRS_Count = 0;
            Modem_PutString("AT+CGREG=2");WAIT_MODEM_RESP(2);
            Ptr = strstr((char*)GPRS_Buffer,"REG:")+5;
         };
         
         Ptr += 2;
         GPRSData.Network_Status = *Ptr;
         if(GPRSData.Network_Status == '1'){GPRSData.Home_Network = SET;GPRSData.Roaming_Network = RESET; SwapCnt = 0;}       // HOME NETWORK DETECTED  
         else if (GPRSData.Network_Status == '5'){GPRSData.Roaming_Network = SET;GPRSData.Home_Network = RESET; SwapCnt = 0;} // ROAMING NETWORK DETECTED
         else if (GPRSData.Network_Status == '3'){GPRSData.Roaming_Network = RESET;GPRSData.Home_Network = RESET; if(SwapCnt > 3)SP_REQ = 1; Flag_SWAP = SET;} // If registration Denied Switch Immedieately
         else 
         {
           Flag_ModemOK = RESET;
           Reg_Error++; SIM_Err++;
           if(Flag_Error){ Flag_Error = RESET; GPRS_ModemErr++;}
         };
       
         
        
         if(GPRSData.Network_Status == '1' || GPRSData.Network_Status == '5')
         {
              Ptr += 3;
              *(Ptr + 4) = 0;
              
              sprintf(GPRSData.LAC,"%s",Ptr);
              Ptr += 7;
              char *SC;
              SC = strtok(Ptr, "\"");
              sprintf(GPRSData.Cell_ID,"%s",SC);
               
              GPRS_ModemErr = 0;
              Reg_Error = 0;
              SIM_Err = 0;
              GPRS_Buffer[15] = 0;
              GPRS_Buffer[16] = 0;
              GPRS_Buffer[498] = 0;
              GPRS_Buffer[499] = 0;
              GPRS_Count = 0;
              Modem_PutString("AT+CGATT?");WAIT_MODEM_RESP(2);
           
          /*----- CHECK NETWORK VALIDATION -------------------------------------------*/
          if((Ptr = strstr((char*)GPRS_Buffer,"TT:")) && (Flag_ModemOK == SET))
            {
               Ptr += 4;
               if(*Ptr == '1'){Flag_ValidNetwork = SET; Reg_Error = 0; SIM_Err = 0;}
               else {Reg_Error++; SIM_Err++;}
            }
           }
          
      }
      else{Flag_ModemOK = RESET;Reg_Error++; SIM_Err++;};
    }
  
    Flag_GetCGREG = RESET;                                                      // RESET FORCEFULLY 
  
    
    
    if(Reg_Error > 70 || GPRS_ModemErr > 5)
    {
      Modem_PutString("AT+QIDEACT");WAIT_MODEM_RESP(2);
      if(GPRSData.SIM_Present[0] == '1')EC200_ONTime = 15;
      GPRS_ModemErr = 0; Reg_Error = 0;
      
    }; // RESET APPLICATION
}
/*******************************************************************************
* Function Name : Verify_UploadData
* Description   : Check the data to be uploaded
* Input         : Parameter name & place
* Output        : Parameter new value
* Return        : Paremeter
*******************************************************************************/
FlagStatus Verify_UploadData(char *s)
{
  char *Ptr;
  Ptr = NULL;
  unsigned char DataCounter;
  Ptr = s+2;

  if(strlen(Ptr) < 180)return RESET;                                            // wrong History upload Data jump
  if(strchr(Ptr,'$'))return RESET;
  else if(strchr(Ptr,'?'))return RESET;                                         // ?
  else if(strchr(Ptr,'@'))return RESET;                                         // ?
  else if(strchr(Ptr,'"'))return RESET;                                         // "
  else if(strchr(Ptr,0xFF)){Ptr = strchr(Ptr,0xFF);*Ptr = 0;return SET;}        // Empty - FF
  else 
     {
      for(DataCounter = 0; ; DataCounter++)
        {
         if(*Ptr == 0)break;
         else if(*Ptr < 32 || *Ptr > 127)return RESET;
         if(DataCounter == 255)break;
         Ptr++;
        };
     };
  
  return SET;
 
}
  
/*******************************************************************************
* Function Name : Check_Config
* Description   : Upload Configuration to server
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void Check_Config(void)
{
    char loc_buffer[50];
    char *Ptr,*SPtr;
    uint16_t GP_Counter;
    uint32_t FlashAddress;
   

    FlagStatus
    Flag_SKIP    = RESET, 
    Flag_OK      = RESET,
    Flag_SYNErr  = RESET,
    Flag_INVALID = RESET;
   
    FlashAddress = 0;
    GP_Counter = 0;
    Ptr  = NULL; 
    SPtr = NULL; 

    memset(loc_buffer,0,50);
    
    if(Flag_NewConf == SET && Protocol == BSNL_MH)
    {
      if(strstr((char*)Config_Buffer,"+S*R:"))
      {
        Flag_OTACFG = SET;
        
        if(strstr((char*)Config_Buffer,"SET:"))
        {
          if(Ptr = strstr((char*)Config_Buffer,"IP#"))
          {
              char SV = 0;
              if(strstr((char*)Config_Buffer,"EIP#")){ SV = 2; Flag_TCPCloseAfterResp[EMR] = SET;} 
              else if(strstr((char*)Config_Buffer,"PIP#")){ SV = 1; Flag_TCPCloseAfterResp[TER] = SET;} 
              else  { SV = 0; Flag_TCPCloseAfterResp[MAIN] = SET;} 
              if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(5);
              
              Flag_INVALID = RESET;
              Flag_OK = RESET; 
              Ptr += 3;
              SPtr = Ptr;
              for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
              {
                if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET;break;}
                if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
              } 
              if(GP_Counter > 49)Flag_INVALID = SET;
              
              if(Flag_INVALID == RESET)
              {
                Ptr = SPtr;
                if(Ptr = strstr(Ptr,","))
                {
                  Ptr++;
                  if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
                  else 
                  {
                    Flag_INVALID = RESET;
                    Flag_SYNErr = RESET;
                  }
                }
                
              }
            
              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
              {
                
                if(SV == 0)sprintf(GP_Buffer,",SET:GIP,%s:%d", SRVR_IP[0],SRVR_Port[0]);
                else if(SV == 1)sprintf(GP_Buffer,",SET:PIP,%s:%d", SRVR_IP[1],SRVR_Port[1]);
                else if(SV == 2)sprintf(GP_Buffer,",SET:EIP,%s:%d", SRVR_IP[2],SRVR_Port[2]);
                
                if(Ptr = strstr(SPtr,","))
                {
                  if(Ptr > SPtr)
                  {
                    memset(&SRVR_IP[SV][0],0,50);
                    strncpy(&SRVR_IP[SV][0],SPtr,Ptr-SPtr);
                    Ptr++;
                    SRVR_Port[SV] = atoi(Ptr);
                  }
                }
                Flag_SYNErr  = RESET;
                Flag_INVALID = RESET;
                Flag_OK = SET;
                
                
              }
          }
          else if(Ptr = strstr((char*)Config_Buffer,"APN#"))
          {
             Flag_SYNErr = SET;
             Ptr += 3;
             if(*Ptr == '#')
             {
                 Flag_SYNErr = RESET;
                 Ptr++;
                 if(strlen(Ptr) > 32)
                 {
                   Flag_INVALID = SET;
                 }
                 else 
                 {
                   SPtr = Ptr;
                   if(SPtr = strchr(Ptr,';'))
                   {
                     *SPtr = 0x0d;
                   }
                   if(SPtr = strchr(Ptr,' '))
                   {
                     Flag_INVALID = SET;
                   }
                   if(Flag_INVALID == RESET)
                   {
                     for(GP_Counter = 0; GP_Counter < 32;GP_Counter++,Ptr++)
                     {
                        if(*Ptr == '\r' || *Ptr == 0x0d || *Ptr == '\n' ){GPRSData.SET_APN[GP_Counter] = 0;Flag_INVALID = RESET;break;}
                        else 
                        {
                          GPRSData.SET_APN[GP_Counter] = *Ptr;
                          
                          Flag_APNSet = RESET; 
                          Flag_MODULE_M66_RST = SET;                                // Initialize
                        }
                      }
                   }
                 }
              }
           
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             { 
               sprintf(GP_Buffer,",SET:APN,%s", GPRSData.SET_APN);
               Flag_OK = SET;
             } 
            
          }
          else if(Ptr = strstr((char*)Config_Buffer,"SOS#"))
          {
            Flag_SYNErr   = RESET;
            Flag_OK       = RESET;
            Flag_INVALID  = RESET;
             
            Ptr += 3; 
           
              if(*Ptr == '#')
              {
                Ptr += 1;
                if(*Ptr == '+')Ptr += 3;
                else if(*Ptr == '0')Ptr++;
                    
                 SPtr = Ptr;
                //if(Ptr = strstr(Ptr,"\r"))
                if(Flag_SerialConfig == SET)
                 {
                   Ptr = strstr(Ptr,"\r");
                 }
                 else 
                 {
                   Ptr = strstr(Ptr,"\n");
                 }
                 //if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")))
                 if(Ptr)
                 {
                  memset(loc_buffer,0,20);           
                  if(((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 11)  || ((Ptr-SPtr) == 12))
                    {
                      if((Ptr-SPtr) == 11 )Ptr--; // from GPRS
                      if((Ptr-SPtr) == 12 )Ptr-=2;
                     strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                     
                     Ptr= &loc_buffer[0];
                     while(*Ptr != '\0'){if(*Ptr < '0' || *Ptr > '9'){Flag_INVALID = SET;Flag_SYNErr = RESET;break;} Ptr++;} 
                     
                     if(Flag_INVALID == RESET)
                     {
                        sprintf(GP_Buffer,",SET:SOS,%s", Vehicle.Master_No[0]);
                        Flag_SYNErr = RESET;
                        Flag_OK = SET;
                        memcpy(&Vehicle.Master_No[GP_Counter][0],loc_buffer,15);
                        
                     }
                    }
                  else 
                  {
                    Flag_INVALID = SET;
                    Flag_SYNErr = RESET;
                  }
                }
               }
              else 
              {
                Flag_SYNErr = SET; 
                Flag_INVALID = RESET;
                Flag_OK = RESET;
              }
            
          }
          else if(Ptr = strstr((char*)Config_Buffer,"VRN#"))
          {
             Flag_SYNErr = RESET;
             Ptr += 4;SPtr = Ptr;
             
             Ptr = strstr(Ptr,";");
             if(Ptr)
             {
              memset(loc_buffer,0,20);           
              if(((Ptr-SPtr) < 17) && (Ptr > SPtr))
                {
                 strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                 Ptr = &loc_buffer[0];
                 
                 sprintf(GP_Buffer,",SET:VRN,%s", Vehicle.VRN);
                 memset(Vehicle.VRN,0,17);
                 memcpy(&Vehicle.VRN[0],loc_buffer,strlen(loc_buffer));
                 Flag_OK = SET;
                }
              else 
              {
                Flag_INVALID = SET;Flag_SYNErr = RESET;
              }
            }
             goto END_FUNC;
           }     
          else if(SPtr = strstr((char*)Config_Buffer,"LOGS#"))
          {
              SPtr += 4;
              if(*SPtr == '#')
              {
                SPtr++; 
                if(*SPtr == ' ')Flag_INVALID  = SET;
                else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                else Active_Time = atoi(SPtr);
                sprintf(GP_Buffer,",SET:LOGS,%d", Active_Time);
                Flag_OK = SET;
              }
              else Flag_SYNErr   = SET;
           }
          else if(SPtr = strstr((char*)Config_Buffer,"LOG2#"))
          {
              SPtr += 4;
              if(*SPtr == '#')
              {
                SPtr++; 
                if(*SPtr == ' ')Flag_INVALID  = SET;
                else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                else StandBy_Time = atoi(SPtr);  
                sprintf(GP_Buffer,",SET:LOG2,%d", StandBy_Time);
                Flag_OK = SET;
              }
              else Flag_SYNErr   = SET;
           }
          else if(SPtr = strstr((char*)Config_Buffer,"HPTI#"))
          {
               SPtr += 4;
               if(*SPtr == '#')
               {
                SPtr++; 
                if(*SPtr == ' ')Flag_INVALID  = SET;
                else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                else HLTH_FRQ = atoi(SPtr);                                                       // HEALTH UPDATE INTERVAL
                sprintf(GP_Buffer,",SET:HPTI,%d", HLTH_FRQ);
                Flag_OK = SET;
               }
               else Flag_SYNErr   = SET;
           }
          else if(SPtr = strstr((char*)Config_Buffer,"EPTI#"))
          {
               SPtr += 4;
               if(*SPtr == '#')
               {
                SPtr++; 
                if(*SPtr == ' ')Flag_INVALID  = SET;
                else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                else ES_Time = atoi(SPtr);                                                       // HEALTH UPDATE INTERVAL
                sprintf(GP_Buffer,",SET:EPTI,%d", ES_Time);
                Flag_OK = SET;
               }
               else Flag_SYNErr   = SET;
           }
          else if(SPtr = strstr((char*)Config_Buffer,"EMTD#"))
          {
               SPtr += 5;
               if(atoi(SPtr) > 7200 || atoi(SPtr) < 60)Flag_INVALID = SET;
               else 
               {
                 ED_Time = atoi(SPtr);
                 Flag_OK = SET;
                 sprintf(GP_Buffer,",SET:EMTD,%d", ED_Time);
               }
          }     
          else if(SPtr = strstr((char*)Config_Buffer,"OSL#"))
          {
               SPtr += 4;
               if(atoi(SPtr) > 220 || atoi(SPtr) < 10)Flag_INVALID = SET;
               else 
               {
                 Speed_Limit = atoi(SPtr);
                 Flag_OK = SET;
                 sprintf(GP_Buffer,",SET:OSL,%d", Speed_Limit);
               }
          }               
          else if(Ptr = strstr((char*)Config_Buffer,"PGF#"))
          {
             //Flag_SYNErr = RESET;
             Ptr += 4;
             
             for(int n = 0; n < 4; n++)GF[0].LAT[n] = GF[0].LON[n] = 0;           
             for(int n = 0; n < 4; n++)
             {
               SPtr = Ptr;
               if(((atof(Ptr)) > -91)  && ((atof(Ptr)) < 91)) 
               {
                  if(Ptr = strstr(Ptr,","))
                  {
                    Ptr++;
                    if(((atof(Ptr)) > -181)  && ((atof(Ptr)) < 181)) 
                    {                     
                       GF[0].ID = 0;
                       GF[0].LAT[n] = atof(SPtr);
                       GF[0].LON[n] = atof(Ptr);
                       Ptr = strstr((Ptr+1),",");
                       Ptr++;
                     }
                     else {Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_FUNC;}
                  }
                  else 
                  {
                     Flag_SYNErr  = SET; 
                     Flag_INVALID = SET;
                     Flag_OK      = RESET;  
                     goto END_FUNC;
                  }
                  
               }
               else{Flag_INVALID = SET;Flag_SYNErr = RESET; goto END_FUNC;}     
               }
               if(Flag_INVALID == RESET)
               {                      
                   sprintf(GP_Buffer,",SET:PFG,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f",GF[0].LAT[0],GF[0].LON[0],GF[0].LAT[1],GF[0].LON[1],GF[0].LAT[2],GF[0].LON[2],GF[0].LAT[3],GF[0].LON[3]);
                   Flag_SYNErr = RESET;
                   Flag_OK = SET;
               }
             
          }          
        }
        else if(strstr((char*)Config_Buffer,"GET:"))
        {
          if(strstr((char*)Config_Buffer,"IP#"))
          {
            if(strstr((char*)Config_Buffer,"EIP#"))sprintf(GP_Buffer,",GET:EIP,%s:%d", SRVR_IP[2],SRVR_Port[2]);
            else if(strstr((char*)Config_Buffer,"PIP#"))sprintf(GP_Buffer,",GET:PIP,%s:%d", SRVR_IP[1],SRVR_Port[1]);
            else sprintf(GP_Buffer,",GET:GIP,%s:%d", SRVR_IP[0],SRVR_Port[0]);;
          }
          else if(strstr((char*)Config_Buffer,"APN#")) sprintf(GP_Buffer,",GET:APN,%s", GPRSData.SET_APN);    
          else if(strstr((char*)Config_Buffer,"SOS#")) sprintf(GP_Buffer,",GET:SOS,%s", Vehicle.Master_No[0]);
          else if(strstr((char*)Config_Buffer,"VRN#")) sprintf(GP_Buffer,",GET:VRN,%s", Vehicle.VRN);
          else if(strstr((char*)Config_Buffer,"LOGS#"))sprintf(GP_Buffer,",GET:LOGS,%d", Active_Time);
          else if(strstr((char*)Config_Buffer,"LOG2#"))sprintf(GP_Buffer,",GET:LOG2,%d", StandBy_Time);
          else if(strstr((char*)Config_Buffer,"HPTI#"))sprintf(GP_Buffer,",GET:HPTI,%d", HLTH_FRQ);
          else if(strstr((char*)Config_Buffer,"EPTI#"))sprintf(GP_Buffer,",GET:EPTI,%d", ES_Time);
          else if(strstr((char*)Config_Buffer,"EMTD#"))sprintf(GP_Buffer,",GET:EMTD,%d", ED_Time);
          else if(strstr((char*)Config_Buffer,"OSL#")) sprintf(GP_Buffer,",GET:OSL,%d", Speed_Limit);
          else if(strstr((char*)Config_Buffer,"PGF#")) sprintf(GP_Buffer,",GET:PFG,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f",GF[0].LAT[0],GF[0].LON[0],GF[0].LAT[1],GF[0].LON[1],GF[0].LAT[2],GF[0].LON[2],GF[0].LAT[3],GF[0].LON[3]);
        }
        else if(strstr((char*)Config_Buffer,"CLR:"))
        {
          Flag_OK = SET; // write to Flash
          if(strstr((char*)Config_Buffer,"IP#"))
          { 
            if(strstr((char*)Config_Buffer,"EIP#")){ sprintf(SRVR_IP[2],"mhsos.nipponsecura.in"); SRVR_Port[2] = 1090; sprintf(GP_Buffer,",EIP,%s:%d", SRVR_IP[2],SRVR_Port[2]);}
            else if(strstr((char*)Config_Buffer,"PIP#")){ sprintf(SRVR_IP[1],"nipponsecura.in"); SRVR_Port[1] = 1166;sprintf(GP_Buffer,"PIP,%s:%d", SRVR_IP[1],SRVR_Port[1]);}
            else { sprintf(SRVR_IP[0],"mh.nipponsecura.in"); SRVR_Port[0] = 2648; sprintf(GP_Buffer,",GIP,%s:%d", SRVR_IP[0],SRVR_Port[0]);}
            if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(5);
          }
          else if(strstr((char*)Config_Buffer,"APN#")) { sprintf(GP_Buffer,",CLR:APN,%s", GPRSData.SET_APN); sprintf(GPRSData.SET_APN,"taisysnet"); Flag_APNSet = RESET; }    
          else if(strstr((char*)Config_Buffer,"SOS#")) { sprintf(GP_Buffer,",CLR:SOS,%s", Vehicle.Master_No[0]); sprintf(Vehicle.Master_No[0],"N.A0000000"); }
          else if(strstr((char*)Config_Buffer,"VRN#")) { sprintf(GP_Buffer,",CLR:VRN,%s", Vehicle.VRN); sprintf(Vehicle.VRN,"N.A0000000");  }
          else if(strstr((char*)Config_Buffer,"LOGS#")){ sprintf(GP_Buffer,",CLR:LOGS,%d", Active_Time); Active_Time = 10;}
          else if(strstr((char*)Config_Buffer,"LOG2#")){ sprintf(GP_Buffer,",CLR:LOG2,%d", StandBy_Time); StandBy_Time = 1800; }
          else if(strstr((char*)Config_Buffer,"HPTI#")){ sprintf(GP_Buffer,",CLR:HPTI,%d", HLTH_FRQ); HLTH_FRQ = 600; } 
          else if(strstr((char*)Config_Buffer,"EPTI#")){ sprintf(GP_Buffer,",CLR:EPTI,%d", ES_Time); ES_Time = 30; }
          else if(strstr((char*)Config_Buffer,"EMTD#")){ sprintf(GP_Buffer,",CLR:EMTD,%d", ED_Time); ED_Time = 1800; }
          else if(strstr((char*)Config_Buffer,"OSL#")) { sprintf(GP_Buffer,",CLR:OSL,%d", Speed_Limit); Speed_Limit = 80; }
          else if(strstr((char*)Config_Buffer,"PGF#")) { sprintf(GP_Buffer,",CLR:PFG,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f",GF[0].LAT[0],GF[0].LON[0],GF[0].LAT[1],GF[0].LON[1],GF[0].LAT[2],GF[0].LON[2],GF[0].LAT[3],GF[0].LON[3]); GF[0].LAT[0] = GF[0].LON[0] = GF[0].LAT[1] = GF[0].LON[1] = GF[0].LAT[2] = GF[0].LON[2] = GF[0].LAT[3] = GF[0].LON[3] =  0;}
        }
        else if(strstr((char*)Config_Buffer,"IMON#"))
        {
           Flag_SetIMOB=SET;//OP1_START;
           Vehicle.DOUT |=0x01;
           DO_STATUS = Vehicle.DOUT;
           Flag_INVALID = RESET;   
           sprintf(GP_Buffer,",IMON,ON");   
        }
        else if(strstr((char*)Config_Buffer,"IMOFF#"))
        {
           OP1_STOP;
           Vehicle.DOUT &=0xFE; 
           DO_STATUS = Vehicle.DOUT;
           Flag_INVALID = RESET;
           sprintf(GP_Buffer,",IMOFF,OFF");
        }
        else if(strstr((char*)Config_Buffer,"RST#"))
        {
            Flag_INVALID = RESET;
            if(strstr((char*)Config_Buffer,"RST#0")){sprintf(GP_Buffer,",RST#0,0"); Flag_MODULE_M66_RST = SET; } 
            else if(strstr((char*)Config_Buffer,"RST#1"))
            {
              sprintf(GP_Buffer,",RST#1,0");                                        
              Flag_MODULE_M66_RST = SET;
              
              TCP_BackCnt[MAIN] = 0;
              TCP_BackCnt[TER] = 0;
              EMRGY_BKP_No = 0;
              
              RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
              char loc_buffer[50]; uint32_t FlashAddress;      
              FlashAddress = SPIFLASH_CONFIG;
              memset(loc_buffer,0,50);
              sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
                 
              if(Flag_SPIOK == SET)
              {
                SPI_FLASH_SectorErase(FlashAddress);
                SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
              };                                        
            }
        }
      
        goto END_FUNC;
      }
      else if(strstr((char*)Config_Buffer,"ACTV"))
      {
          if(Ptr = strstr((char*)Config_Buffer,"ACTV,"))
          {
             Ptr += 5;SPtr = Ptr;
             if(Ptr = strstr(Ptr,","))
             {
               memset(Random_Code,0,7);
               if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
               {
                 strncpy(Random_Code,SPtr,Ptr-SPtr);
                 Ptr += 1;
                 if(*Ptr == '+')Ptr += 3;
                 else if(*Ptr == '0')Ptr += 1;
                 SPtr = Ptr;
                 if(Flag_SerialConfig == SET)
                 {
                   Ptr = strstr(Ptr,"\r");
                 }
                 else 
                 {
                   Ptr = strstr(Ptr,"\n");
                 }
                 if(Ptr == 0)Ptr = &Config_Buffer[strlen(Config_Buffer)];
                 if(Ptr)
                 {
                  if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 11))
                  {
                    if((Ptr-SPtr) == 11)Ptr--;
                   Flag_INVALID = RESET;
                   memset(loc_buffer,0,20);
                   strncpy(loc_buffer,SPtr,Ptr-SPtr);
                   Ptr = &loc_buffer[0];
                   while(*Ptr != '\0')
                   {
                     if(*Ptr == 13 || *Ptr == 10){ *Ptr = 0; break;}
                     if(*Ptr < '0' || *Ptr > '9')
                     {
                       sprintf(Reply_No,"N.A0000000"); 
                       Flag_INVALID = SET;
                       break;
                     } 
                     Ptr++;
                   }
                   if(Flag_INVALID == RESET)
                   {
                     memset(Reply_No,0,15);
                     strcpy(Reply_No,loc_buffer);
                     if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                     {
                      Flag_ACTV = SET; 
                      strcat(GP_Buffer,RESP_BUFFER);
                      
                     }
                   }
                 }
                  else Flag_INVALID = SET;
               }
             }
             };
          };
          goto END_FUNC;
      }
      else if(strstr((char*)Config_Buffer,"HCHK"))
      {
        if(Ptr = strstr((char*)Config_Buffer,"HCHK,"))
        {
           Ptr += 5;SPtr = Ptr;
           
           if(Ptr = strstr(Ptr,","))
           {
             memset(Random_Code,0,7);
             if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
             {
               strncpy(Random_Code,SPtr,Ptr-SPtr);
               Ptr += 1;
               if(*Ptr == '+')Ptr += 3;
               else if(*Ptr == '0')Ptr += 1;
               SPtr = Ptr;
               
               //if(Ptr = strstr(Ptr,"\r"))
               if(Flag_SerialConfig == SET)
               {
                 Ptr = strstr(Ptr,"\r");
               }
               else 
               {
                 Ptr = strstr(Ptr,"\n");
               }
               
               if(Ptr == 0)Ptr = &Config_Buffer[strlen(Config_Buffer)];
               //if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")))
               if(Ptr)
               {
                 
                 if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 11))
                  {
                    if((Ptr-SPtr) == 11)Ptr--;
                   Flag_INVALID = RESET;
                   memset(loc_buffer,0,20);
                   strncpy(loc_buffer,SPtr,Ptr-SPtr);

                   Ptr = &loc_buffer[0];
                   while(*Ptr != '\0')
                   {
                     if(*Ptr == 13 || *Ptr == 10){ *Ptr = 0; break;}
                     if(*Ptr < '0' || *Ptr > '9')
                     {
                       sprintf(Reply_No,"N.A0000000"); 
                       Flag_INVALID = SET;
                       break;
                     } 
                     Ptr++;
                   }
                   if(Flag_INVALID == RESET)
                   {
                     memset(Reply_No,0,15);
                     strcpy(Reply_No,loc_buffer);
                     if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                     {
                      Flag_HCHK = SET;
                     }
                   }
                 }
                 else Flag_INVALID = SET;
               }
               else Flag_SYNErr = SET;
             }
             else Flag_INVALID = SET;
           }
        }
        goto END_FUNC;
      }
    }
   
     /*-----  CONVERT ENTIRE STRING INTO UPPER CASE ------------------------------*/
     //if(strstr((char*)Config_Buffer,"@PASS#EFP-UPGRADE"))
     memset(loc_buffer,0,50);
     sprintf(loc_buffer,"@%s#-UPGRADE",PWD_IN);
     if((strstr((char*)Config_Buffer,"@PASS#EDS-UPGRADE")) || (strstr((char*)Config_Buffer,loc_buffer)))  
     {
        Flag_DFU = SET;
        // HTTP SERVER IP 
        if(Ptr = strstr((char*)Config_Buffer,"IP-")){Ptr += 3;for(GP_Counter = 0;GP_Counter <= 3; GP_Counter++){DFU_HTTP.IP[GP_Counter] = atoi(Ptr);Ptr = strchr(Ptr,'.') + 1;};};
        // HTTP PATH
        if(Ptr = strstr((char*)Config_Buffer,"PATH-"))
        {
          Ptr += 5;SPtr = Ptr;
          if(Ptr = strstr(Ptr,","))
          {
            memset(DFU_HTTP.PATH,0,50);
            if(((Ptr-SPtr) < 16) && (Ptr > SPtr))strncpy(DFU_HTTP.PATH,SPtr,(Ptr-SPtr)); 
            else Flag_DFU = RESET;
          };
        }
        // HTTP FILE
        if(Ptr = strstr((char*)Config_Buffer,"FILE-"))
        {
          Ptr += 5;SPtr = Ptr;
          if(Ptr = strstr(Ptr,","))
          {
            memset(DFU_HTTP.FILENAME,0,15);
            if(((Ptr-SPtr) < 16) && (Ptr > SPtr))strncpy(DFU_HTTP.FILENAME,SPtr,(Ptr-SPtr)); 
            else Flag_DFU = RESET;
          };
        }
        // DFU SOFTWARE VERSION
        if(Ptr = strstr((char*)Config_Buffer,"V-"))
        {
          SPtr = Ptr; 
          if(Ptr = strchr(Ptr,','))
          {
            if(((Ptr-SPtr) < 10) && (Ptr > SPtr)){memset(SW_VER,0,10);memcpy((char*)SW_VER,SPtr,Ptr-SPtr);} 
            else Flag_DFU = RESET;
          }
        }
        if(Flag_DFU)
        {
          if(Flag_APNSet == RESET)Set_APN();
          Flag_OK = SET;
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          goto END_FUNC;
        }                                                        // SEND UPGRADE MESSAGE TO DFU NUMBER 
      }
     else if(strstr((char*)Config_Buffer,"@CLR#DFU"))
     {
        DFU_HTTP.IP[0] = 0;                                                        // DFU SERVER IP ADDRESS
        DFU_HTTP.IP[1] = 0;
        DFU_HTTP.IP[2] = 0;
        DFU_HTTP.IP[3] = 0;
        sprintf(DFU_HTTP.PATH,"N.A");                                          // DFU FTP SERVER USERNAME
        sprintf(DFU_HTTP.FILENAME,"N.A");                                          // DFU FTP SERVER PASSWORD  
       
        Flag_OK = SET;
        Flag_INVALID = RESET;
        Flag_SYNErr = RESET;
        goto END_FUNC;
     }
     else if(strstr((char*)Config_Buffer,"ACTV"))
     {
        if(Ptr = strstr((char*)Config_Buffer,"ACTV,"))
        {
           Ptr += 5;SPtr = Ptr;
           if(Ptr = strstr(Ptr,","))
           {
             memset(Random_Code,0,7);
             if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
             {
               strncpy(Random_Code,SPtr,Ptr-SPtr);
               Ptr += 1;
               if(*Ptr == '+')Ptr += 3;
               else if(*Ptr == '0')Ptr += 1;
               SPtr = Ptr;
               
               if(Flag_SerialConfig == SET)
               {
                 Ptr = strstr(Ptr,"\r");
               }
               else 
               {
                 Ptr = strstr(Ptr,"\n");
               }
              
               //if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")))
               if(Ptr)
               {
                 if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13))
                  {
                   Flag_INVALID = RESET;
                   memset(loc_buffer,0,20);
                   strncpy(loc_buffer,SPtr,Ptr-SPtr);
                   Ptr = &loc_buffer[0];
                   while(*Ptr != '\0')
                   {
                     if(*Ptr == 13 || *Ptr == 10){ *Ptr = 0; break;}
                     if(*Ptr < '0' || *Ptr > '9')
                     {
                       sprintf(Reply_No,"N.A0000000"); 
                       Flag_INVALID = SET;
                       break;
                     } 
                     Ptr++;
                   }
                   if(Flag_INVALID == RESET)
                   {
                     memset(Reply_No,0,15);
                     strcpy(Reply_No,loc_buffer);
                     if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                     {
                      Flag_ACTV = SET; 
                     }
                   }
                 }
                 else Flag_INVALID = SET;
               }
               else Flag_SYNErr = SET;
             }
             else Flag_INVALID = SET;
           }
        }
        goto END_FUNC;
     }
     else if(strstr((char*)Config_Buffer,"HCHK"))
     {
        if(Ptr = strstr((char*)Config_Buffer,"HCHK,"))
        {
           Ptr += 5;SPtr = Ptr;
           
           if(Ptr = strstr(Ptr,","))
           {
             memset(Random_Code,0,7);
             if(((Ptr-SPtr) < 7) && (Ptr > SPtr))
             {
               strncpy(Random_Code,SPtr,Ptr-SPtr);
               Ptr += 1;
               if(*Ptr == '+')Ptr += 3;
               else if(*Ptr == '0')Ptr += 1;
               SPtr = Ptr;
               
               //if(Ptr = strstr(Ptr,"\r"))
               if(Flag_SerialConfig == SET)
               {
                 Ptr = strstr(Ptr,"\r");
               }
               else 
               {
                 Ptr = strstr(Ptr,"\n");
               }
               //if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")))
               if(Ptr)
               {
                 
                 if(((Ptr-SPtr) == 10) || ((Ptr-SPtr) == 13))
                  {
                   Flag_INVALID = RESET;
                   memset(loc_buffer,0,20);
                   strncpy(loc_buffer,SPtr,Ptr-SPtr);

                   Ptr = &loc_buffer[0];
                   while(*Ptr != '\0')
                   {
                     if(*Ptr == 13 || *Ptr == 10){ *Ptr = 0; break;}
                     if(*Ptr < '0' || *Ptr > '9')
                     {
                       sprintf(Reply_No,"N.A0000000"); 
                       Flag_INVALID = SET;
                       break;
                     } 
                     Ptr++;
                   }
                   if(Flag_INVALID == RESET)
                   {
                     memset(Reply_No,0,15);
                     strcpy(Reply_No,loc_buffer);
                     if((Reply_No[0] != 'N') && (Reply_No[0] != 0))
                     {
                      Flag_HCHK = SET;
                     }
                   }
                 }
                 else Flag_INVALID = SET;
               }
               else Flag_SYNErr = SET;
             }
             else Flag_INVALID = SET;
           }
        }
        goto END_FUNC;
      }
     else if((strstr((char*)Config_Buffer,"@SET#")) || (strstr((char*)Config_Buffer,"SET")) || (strstr((char*)Config_Buffer,"#SET_CONFIG")))
     {
        Flag_OTACFG = SET;
        if(Flag_SerialConfig == RESET)  
        {
  ////        memset(CC_Source_TER,0,50);
  ////        memset(CC_Source,0,50);
  ////        if(Flag_ServerConf[MAIN] == SET)strcat(CC_Source_TER,&SRVR_IP[MAIN][0]);
  ////        else if(Flag_ServerConf[TER] == SET)strcat(CC_Source_TER,&SRVR_IP[TER][0]);
         
        }else sprintf(CC_Source,"%s","SERIAL");
        
          
        /***********************SELECT PROTOCOL ***************************************/ 
        if(Ptr = strstr((char*)Config_Buffer,"PROTO "))
        {
            Flag_SYNErr  = RESET;
            Flag_INVALID = RESET;
            Flag_OK = RESET;
            Active_Time = 10;
            Ptr += 6;
            if((*(Ptr+4) == '\r') ||  (*(Ptr+4) == '\n') ||  (*(Ptr+5) == '\n'))
            {
              *(Ptr+4) = 0;
              switch(*Ptr - 48)
              {
                case HP_PC    :
                case BSNL_PC  : Flag_OK = SET; Protocol = (*Ptr - 48);                // BSNL + HP
                                Ptr += 2;
                                break;
                
                case NIC_OD   : Flag_OK = SET; Protocol = NIC_OD;   
                                break;
                                
                case NIC_PC   : Flag_OK = SET; Protocol = NIC_PC;   
                                Ptr += 2; Active_Time = 20;
                                break;  // NIC
                                
                case BSNL_MH :  Flag_OK = SET; Protocol = BSNL_MH;  
                                break;  // BSNL 
                                
                default       : Flag_SYNErr = SET; break;
              }   
            }
            else Flag_INVALID = SET;
            
            if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
            {
              Flag_OK = SET;
              Refresh_Data();
              Flag_ServerConf[TER] = SET;
            }
            goto END_FUNC;
          }   
        /*********************** SET TILT ANGLE ***************************************/    
        else if(Ptr = strstr((char*)Config_Buffer,"TA "))
        {      
          Ptr += 3;
          SPtr = Ptr;
          if(*Ptr == ' ')Flag_INVALID  = SET;
          else if(atoi(Ptr) < 180)
          {
            TA = atoi(Ptr);
            Flag_INVALID  = RESET;
            Flag_SYNErr   = RESET;
            Flag_OK = SET;
            sprintf(GP_Buffer,",SETTA,%d",TA);
            strcat(CC_Source_TER,GP_Buffer);
          }
          else 
          {
            Flag_SYNErr   = RESET; 
            Flag_INVALID  = SET;
            Flag_OK       = RESET;
          }
          goto END_FUNC;
       }  
        else if(Ptr = strstr((char*)Config_Buffer,"OFFSET: "))
        {      
            OFFSET_Y = ACTUAL_PY;
            OFFSET_X = ACTUAL_RX;
            Flag_OK = SET;
            Flag_INVALID  = RESET;
            Flag_SYNErr  = RESET;
            
            sprintf(GP_Buffer,",SETOFFSET,0");
            strcat(CC_Source_TER,GP_Buffer);
            
            goto END_FUNC;
         }     
        /*********************** SET PRIMARY SERVER ***********************************/// SETPS
        else if(Ptr = strstr((char*)Config_Buffer,"PS "))
        {               
          Flag_SYNErr  = SET;
          Flag_INVALID = RESET;
          Flag_OK = RESET; 
          Ptr += 3;
          SPtr = Ptr;
          for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
          {
            if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET;break;}
            if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
          } 
          if(GP_Counter > 49)Flag_INVALID = SET;
          
          if(Flag_INVALID == RESET)
          {
            Ptr = SPtr;
            if(Ptr = strstr(Ptr,":"))
            {
              Ptr++;
              if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
              else 
              {
                Flag_INVALID = RESET;
                Flag_SYNErr = RESET;
              }
            }
            
          }
       

          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
          {
           if(Ptr = strstr(SPtr,":"))
            {
              if(Ptr > SPtr)
              {
                memset(&SRVR_IP[MAIN][0],0,50);
                strncpy(&SRVR_IP[MAIN][0],SPtr,Ptr-SPtr);
                Ptr++;
                SRVR_Port[MAIN] = atoi(Ptr);
                
                sprintf(GP_Buffer,",SETPS,%s:%d",SRVR_IP[MAIN],SRVR_Port[MAIN]);
                strcat(CC_Source_TER,GP_Buffer);
              }
            }
            Flag_SYNErr  = RESET;
            Flag_INVALID = RESET;
            Flag_OK = SET;
            Flag_TCPConnect[MAIN] = RESET;Modem_PutString("AT+QICLOSE=0"); 
            if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
          }  
          goto END_FUNC;
       }
        /******************** SET EMERGENCY SERVER ***********************************/ // SETSS
        else if(Ptr = strstr((char*)Config_Buffer,"SS "))
        {
          
              Flag_SYNErr  = SET;
              Flag_INVALID = RESET;
              Flag_OK = RESET; 
              Ptr += 3;
              SPtr = Ptr;
              for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
              {
                 if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET;break;}
                 if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
              } 
              if(GP_Counter > 49)Flag_INVALID = SET;
              
              if(Flag_INVALID == RESET)
              {
                Ptr = SPtr;
                if(Ptr = strstr(Ptr,":"))
                {
                  Ptr++;
                  if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
                  else 
                  {
                    Flag_INVALID = RESET;
                    Flag_SYNErr = RESET;
                  }
                }
                
              }
           

              if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
              {
                if(Ptr = strstr(SPtr,":"))
                {
                  if(Ptr > SPtr)
                  {
                    memset(&SRVR_IP[EMR][0],0,50);
                    strncpy(&SRVR_IP[EMR][0],SPtr,Ptr-SPtr);
                    Ptr++;
                    SRVR_Port[EMR] = atoi(Ptr);
                    sprintf(GP_Buffer,",SETSS,%s:%d",SRVR_IP[EMR],SRVR_Port[EMR]);
                    strcat(CC_Source_TER,GP_Buffer);
                  }
                }
                Flag_SYNErr  = RESET;
                Flag_INVALID = RESET;
                Flag_OK = SET;
                Flag_TCPConnect[EMR] = RESET;Modem_PutString("AT+QICLOSE=2"); 
                if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
              }  
          
         goto END_FUNC;
       }  
        /******************** SET TERTIARY SERVER *********************************/    // SETTS
        else if(Ptr = strstr((char*)Config_Buffer,"TS "))
        {
          Flag_SYNErr  = SET;
          Flag_INVALID = RESET;
          Flag_OK = RESET; 
          Ptr += 3;
          SPtr = Ptr;
          for(GP_Counter = 0;GP_Counter < 51; GP_Counter++,Ptr++)                   // SERVER IP MAIN
          {
            if(*Ptr == ' '){Flag_INVALID = SET;Flag_SYNErr = RESET;break;}
            if((*Ptr == '\r') || (*Ptr == '\n') || (*Ptr == '\0'))break;
          } 
          if(GP_Counter > 49)Flag_INVALID = SET;
          
          if(Flag_INVALID == RESET)
          {
            Ptr = SPtr;
            if(Ptr = strstr(Ptr,":"))
            {
              Ptr++;
              if((atoi(Ptr) == 0) || (atoi(Ptr) > 65535))Flag_INVALID = SET;
              else 
              {
                Flag_INVALID = RESET;
                Flag_SYNErr = RESET;
              }
            }
            
          }
       

          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
          {
            if(Ptr = strstr(SPtr,":"))
            {
              if(Ptr > SPtr)
              {
                memset(&SRVR_IP[TER][0],0,50);
                strncpy(&SRVR_IP[TER][0],SPtr,Ptr-SPtr);
                Ptr++;
                SRVR_Port[TER] = atoi(Ptr);
                sprintf(GP_Buffer,",SETSS,%s:%d",SRVR_IP[TER],SRVR_Port[TER]);
                strcat(CC_Source_TER,GP_Buffer);
              }
            }
            Flag_SYNErr  = RESET;
            Flag_INVALID = RESET;
            Flag_OK = SET;
            Flag_TCPConnect[TER] = RESET;Modem_PutString("AT+QICLOSE=1"); 
            if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
          }                                              // IF CHANGED CLOSE CONNECTION
          goto END_FUNC;
         }   
        /***************************** SETIMMOB **************************************/  // SET IMMOBILIZER   
        else if(Ptr = strstr((char*)Config_Buffer,"IMMOB "))
        {
           Flag_SYNErr   = RESET;
           Flag_OK       = RESET;
           Flag_INVALID  = RESET;
           
           Ptr += 6;
           if((*(Ptr+1) == '\r') || (*(Ptr+1) == '\n'))
           {
             if(*Ptr == '0') 
             {
                OP1_STOP;
                Vehicle.DOUT &=0xFE; 
                DO_STATUS = Vehicle.DOUT;
                Flag_OK = SET;
                Flag_INVALID = RESET; 
                sprintf(GP_Buffer,",SETIMOB,ON");
                strcat(CC_Source_TER,GP_Buffer);
             }
             else if(*Ptr == '1') 
             {
                OP1_START;
                Vehicle.DOUT |=0x01;
                DO_STATUS = Vehicle.DOUT;
                Flag_OK = SET;
                Flag_INVALID = RESET; 
                sprintf(GP_Buffer,",SETIMOB,OFF");
                strcat(CC_Source_TER,GP_Buffer);
             }
             else Flag_INVALID = SET;
           }
           else 
           {
             Flag_INVALID = SET;
           }
           
          goto END_FUNC; 
         }
        /***************************** SETMOB **************************************/  // SET THE MASTER NUMBER 1-5
        else if(Ptr = strstr((char*)Config_Buffer,"MOB "))
        {
          Flag_SYNErr   = RESET;
          Flag_OK       = RESET;
          Flag_INVALID  = RESET;
           
          Ptr += 4; 
          GP_Counter = atoi(Ptr);
          if(GP_Counter < 1 || GP_Counter > 5){Flag_INVALID = SET;Flag_SYNErr = RESET;}
          GP_Counter -= 1;
          if(Flag_INVALID  == RESET)
          {
            Ptr++;
            if(*Ptr == ':')
            {
              Ptr += 1;
              if(*Ptr == '+')Ptr += 3;
              else if(*Ptr == '0')Ptr++;
                  
               SPtr = Ptr;
              //if(Ptr = strstr(Ptr,"\r"))
              if(Flag_SerialConfig == SET)
               {
                 Ptr = strstr(Ptr,"\r");
               }
               else 
               {
                 if(strstr(Ptr,","))Ptr = strstr(Ptr,",");
                 else Ptr = strstr(Ptr,"\r");
               }
               //if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")))
               if(Ptr)
               {
                memset(loc_buffer,0,20);           
                if(((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 10))
                  {
                   strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                   
                   Ptr= &loc_buffer[0];
                   while(*Ptr != '\0'){if(*Ptr < '0' || *Ptr > '9'){Flag_INVALID = SET;Flag_SYNErr = RESET;break;} Ptr++;} 
                   
                   if(Flag_INVALID == RESET)
                   {
                      Flag_SYNErr = RESET;
                      Flag_OK = SET;
                      memcpy(&Vehicle.Master_No[GP_Counter][0],loc_buffer,15);
                      sprintf(GP_Buffer,",SETMOB %d,%s",GP_Counter+1,&Vehicle.Master_No[GP_Counter][0]);
                      strcat(CC_Source_TER,GP_Buffer);
                   }
                  }
                else 
                {
                  Flag_INVALID = SET;
                  Flag_SYNErr = RESET;
                }
              }
             }
            else 
            {
              Flag_SYNErr = SET; 
              Flag_INVALID = RESET;
              Flag_OK = RESET;
            }
          }
          
          goto END_FUNC; 
         }     
        /*************************************** SETST ********************************/ //Set the Emergency state detection & tamper detection timeouts
        else if(Ptr = strstr((char*)Config_Buffer,"ST "))                   
        { 
          Flag_SYNErr   = SET;
          Flag_OK       = RESET;
          Flag_INVALID  = RESET;
          Ptr += 3;
           if(*Ptr == 'S')                                                        // EMERGENCY STATE DETECTION 
           {
             if(*(Ptr+1) ==':')
             {
               Flag_SYNErr   = RESET;
               Ptr += 2;
               if(*Ptr == ' ')Flag_INVALID  = SET;
               else if(atoi(Ptr) < 60001 && atoi(Ptr) > 49)EMG_BP_FREQ = atoi(Ptr);  
               else Flag_INVALID  = SET;
             }
             else {Flag_SYNErr   = SET; goto END_FUNC;}
           }
           
           if(Ptr = strstr(Ptr,"T")) 
           {
             Flag_SYNErr   = SET;
             Ptr++;
             if(*Ptr == ':')
             {
               Flag_SYNErr = RESET;
               Ptr++;
               if(*Ptr == ' ')Flag_INVALID  = SET;
               else if(atoi(Ptr) < 60001 && atoi(Ptr) > 49)
               {
                 EMG_WC_Timeout = atoi(Ptr);  
                 
                 sprintf(GP_Buffer,",SETST,%d",EMG_WC_Timeout);
                 strcat(CC_Source_TER,GP_Buffer);
               }
               else Flag_INVALID  = SET;
             }                                                                    // EMERGENCY TAMPER DETECTION
             else {Flag_SYNErr   = SET; goto END_FUNC;}
           }
           
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
             { 
               Flag_OK = SET;
             }
           goto END_FUNC;
         }   
        /************************************* SETVEHID *******************************/   // SET THE VEHICLE REGISTRATION NUMBER 
        else if(Ptr = strstr((char*)Config_Buffer,"VEHID "))
        {
             Flag_SYNErr = RESET;
             Ptr += 6;SPtr = Ptr;
             if(strstr(Ptr,"\r"))Ptr = strstr(Ptr,"\r");
             else Ptr = strstr(Ptr,",");
             
               if(Ptr)
               {
                memset(loc_buffer,0,20);           
                if(((Ptr-SPtr) < 17) && (Ptr > SPtr))
                  {
                   strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                   Ptr = &loc_buffer[0];
                   
                   memset(Vehicle.VRN,0,17);
                   memcpy(&Vehicle.VRN[0],loc_buffer,strlen(loc_buffer));
                   sprintf(GP_Buffer,",SETVEHID,%s",&Vehicle.VRN[0]);
                   strcat(CC_Source_TER,GP_Buffer);
                   Flag_OK = SET;
                  }
                else 
                {
                  Flag_INVALID = SET;Flag_SYNErr = RESET;
                }
              }
              goto END_FUNC;
           }        
        /****************** SETDOP ***************************************************/
        else if(Ptr = strstr((char*)Config_Buffer,"DOP "))
        {
             Flag_INVALID = RESET;
             Flag_SYNErr  = RESET;
             Flag_OK      = RESET;
             
             Ptr += 4;
             if(*(Ptr + 1) == ',')  
             {
               if((*(Ptr + 3) == '\r') || (*(Ptr + 3) == '\n'))
               {
                 if((*Ptr == '1') && (*(Ptr+2) == '1'))                                   // 0-> low = gnd = op start
                 {
                   OP1_STOP;
                   Vehicle.DOUT &=0xFE; 
                   DO_STATUS = Vehicle.DOUT;
                   Flag_INVALID = RESET;
                   sprintf(GP_Buffer,",SETDO1,ON");
                   strcat(CC_Source_TER,GP_Buffer);
                 }
                 else if((*Ptr == '1') && (*(Ptr+2) == '0'))
                 {
                   OP1_START;
                   Vehicle.DOUT |=0x01;
                   DO_STATUS = Vehicle.DOUT;
                   Flag_INVALID = RESET;  
                   sprintf(GP_Buffer,",SETDO1,OFF");
                   strcat(CC_Source_TER,GP_Buffer);
                 }
                 else if((*Ptr == '2') && (*(Ptr+2) == '1'))
                 {
                   OP2_STOP;
                   Vehicle.DOUT &=0xFD; 
                   DO_STATUS = Vehicle.DOUT;
                   Flag_INVALID = RESET; 
                   sprintf(GP_Buffer,",SETDO2,ON");
                   strcat(CC_Source_TER,GP_Buffer);
                 }
                 else if((*Ptr == '2') && (*(Ptr+2) == '0'))
                 {
                   OP2_START;
                   Vehicle.DOUT |=0x02; 
                   DO_STATUS = Vehicle.DOUT;
                   Flag_INVALID = RESET;  
                   sprintf(GP_Buffer,",SETDO2,OFF");
                   strcat(CC_Source_TER,GP_Buffer);
                 }
                 else 
                 {
                   Flag_INVALID = SET;Flag_SYNErr = RESET; 
                 }
               }
             else 
               {
                 Flag_INVALID = SET;Flag_SYNErr = RESET; 
               }
             
             }
             else 
             {
                Flag_SYNErr  = SET; 
                Flag_INVALID = RESET;
                Flag_OK      = RESET;
              }
             
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET;
              
             goto END_FUNC;
             }           
        /***************************** SETCP **************************************/     // SET THE CUSTOM APN 
        else if(Ptr = strstr((char*)Config_Buffer,"CP "))
        {
             Flag_SYNErr = SET;
             Flag_INVALID = RESET;
             Flag_OK = RESET;
             
             Ptr += 3;
             if(*Ptr == 'P') 
             {
               Ptr++;
               if(*Ptr == ':')
               {
                 Flag_SYNErr = RESET;
                 Ptr++;
  //               switch(*Ptr - 48)
  //                {
  //                case BSNL_PC  : Flag_SYNErr = RESET; Flag_INVALID = RESET; Protocol = BSNL_PC;  break;      // BSNL 
  //                case DIMTS_PC : Flag_SYNErr = RESET; Flag_INVALID = RESET; Protocol = DIMTS_PC; break;      // DIMTS 
  //                case UK_PC    : Flag_SYNErr = RESET; Flag_INVALID = RESET; Protocol = UK_PC;    break;      // UK
  //                default       : Flag_INVALID = SET; break;
  //                }
                 if(Ptr = strstr(Ptr,"TCP"))Flag_INVALID = RESET;
                 else Flag_INVALID = SET;
               }
             }
             
             if(Ptr = strstr(Ptr,"A"))
             {
               Flag_SYNErr = SET;
               Ptr++;
                if(*Ptr == ':')
                {
                   Flag_SYNErr = RESET;
                   Ptr++;
                   if(strlen(Ptr) > 32)
                   {
                     Flag_INVALID = SET;
                   }
                   else 
                   {
                     SPtr = Ptr;
                     if(SPtr = strchr(Ptr,' '))
                     {
                       Flag_INVALID = SET;
                     }
                     if(Flag_INVALID == RESET)
                     {
                       for(GP_Counter = 0; GP_Counter < 32;GP_Counter++,Ptr++)
                       {
                          if(*Ptr == '\r' || *Ptr == ' ' || *Ptr == '\n' ){GPRSData.SET_APN[GP_Counter] = 0;Flag_INVALID = RESET;break;}
                          else 
                          {
                            if(isupper(*Ptr))*Ptr = tolower(*Ptr);
                            GPRSData.SET_APN[GP_Counter] = *Ptr;
                            
                            Flag_APNSet = RESET; 
                            Flag_MODULE_M66_RST = SET;                                // Initialize
                          }
                        }
                        sprintf(GP_Buffer,",SETCP A,%s",GPRSData.SET_APN);
                        strcat(CC_Source_TER,GP_Buffer);
                     }
                   }
                }
             }
             
             if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET;
             goto END_FUNC;
           }    
        /***************************** SETUI **************************************/    // SET THE UPDATE INTERVALS      
        else if(Ptr = strstr((char*)Config_Buffer,"UI "))
        { 
           Flag_SYNErr   = SET;
           
           Ptr += 3;
           if((*Ptr == 'A')|| (*Ptr == 'O')|| (*Ptr == 'H') || (*Ptr == 'S') || (*Ptr == 'E') || (*Ptr == 'T') || (*Ptr == 'I') || (*Ptr == 'C'))
           {
             if(*(Ptr+1)== ':')
             {
                 Flag_SYNErr   = RESET;
                 if(SPtr = strstr(Ptr,"A"))
                 {
                    SPtr++;
                    if(*SPtr == ':')
                    {
                      SPtr++; 
                      if(*SPtr == ' ')Flag_INVALID  = SET;
                      else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                      else Active_Time = atoi(SPtr);
                      
                      sprintf(GP_Buffer,",SETUI A,%d",Active_Time);
                      strcat(CC_Source_TER,GP_Buffer);
                    }
                    else Flag_SYNErr   = SET;
                  }
                 if(SPtr = strstr(Ptr,"O"))
                 {
                    SPtr++;
                    if(*SPtr == ':')
                    {
                      SPtr++; 
                      if(*SPtr == ' ')Flag_INVALID  = SET;
                      else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                      else StandBy_Time = atoi(SPtr);     
                      
                      sprintf(GP_Buffer,",SETUI O,%d",StandBy_Time);
                      strcat(CC_Source_TER,GP_Buffer);
                    }
                    else Flag_SYNErr   = SET;
                 }
                 if(SPtr = strstr(Ptr,"H"))
                 {
                   SPtr++;
                   if(*SPtr == ':')
                   {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else HLTH_FRQ = atoi(SPtr);                                                       // HEALTH UPDATE INTERVAL
                    
                    sprintf(GP_Buffer,",SETUI H,%d",HLTH_FRQ);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                   
                 }
                 if(SPtr = strstr(Ptr,"S"))
                 {
                   SPtr++;
                   if(*SPtr == ':')
                   {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else StopMode_Time = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETUI S,%d",StopMode_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                 }
                 if(SPtr = strstr(Ptr,"E"))
                 {
                   SPtr++;
                   if(*SPtr == ':')
                   {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else ES_Time = atoi(SPtr);                                                   // EMERGENCY UPDATE INTERVAL
                    
                    sprintf(GP_Buffer,",SETUI E,%d",ES_Time);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                 }             
                 if(SPtr = strstr(Ptr,"T"))
                 {
                   SPtr++;
                   if(*SPtr == ':')
                   {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else Time2Sleep = atoi(SPtr);                                           // TIME TO ENTER INTO SLEEPMODE  AFTER IGNITION OFF
                   
                    sprintf(GP_Buffer,",SETUI T,%d",Time2Sleep);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                 }                 
                 if(SPtr = strstr(Ptr,"I"))
                 {
                   SPtr++;
                   if(*SPtr == ':')
                   {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                    else OVS_DeadTime = atoi(SPtr);                                            // Time to wait for generating over speed alert after device overspeeds
                  
                    sprintf(GP_Buffer,",SETUI I,%d",OVS_DeadTime);
                    strcat(CC_Source_TER,GP_Buffer);
                   }
                   else Flag_SYNErr   = SET;
                 }
                 if(SPtr = strstr(Ptr,"C"))
                 {
                     SPtr++;
                     if(*SPtr == ':')
                     {
                       SPtr++; 
                       if(*SPtr == ' ')Flag_INVALID  = SET;
                       else if((atoi(SPtr) > 3600) || (atoi(SPtr) < 5)){Flag_INVALID  = SET;Flag_SYNErr = RESET;}
                       else TER_HitTime = atoi(SPtr);
                       
                       sprintf(GP_Buffer,",SETUI C,%d",TER_HitTime);
                       strcat(CC_Source_TER,GP_Buffer);
                     }
                     else Flag_SYNErr   = SET;
                 }
               
             }
           } 
           
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET;
           goto END_FUNC;
         }          
        /***************************** SETTD **************************************/    // SET THE THRESHOLD LEVEL
        else if(Ptr = strstr((char*)Config_Buffer,"TD "))
        { 
           Flag_SYNErr = SET;
           Ptr += 3;
           if(((*Ptr == 'L')&&(*(Ptr+1) == 'B')) || ((*Ptr == 'H')&&(*(Ptr+1) == 'A')) || ((*Ptr == 'H')&&(*(Ptr+1) == 'B')) || ((*Ptr == 'R')&&(*(Ptr+1) == 'T')) || ((*Ptr == 'S')&&(*(Ptr+1) == 'L')) || ((*Ptr == 'W')&&(*(Ptr+1) == 'K')) || ((*Ptr == 'I') && (*(Ptr+1) == 'D'))) 
           {
             if(*(Ptr+2)== ':')
             {
                Flag_SYNErr = RESET;
                if(SPtr = strstr(Ptr,"LB"))
                {
                    SPtr += 2; 
                    if(*SPtr == ':')
                    {
                      SPtr++;
                      if(*SPtr == ' ')Flag_INVALID  = SET;
                      else if((atoi(SPtr) > 50) || (atoi(SPtr) < 5) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                      else IBL = atoi(SPtr);                                                // ACTIVE UPDATE INTERVAL   
                      
                       sprintf(GP_Buffer,",SETTD LB,%d",IBL);
                       strcat(CC_Source_TER,GP_Buffer);
                    } 
                    else Flag_SYNErr = SET;
                  }            
                if(SPtr = strstr(Ptr,"HA"))
                {
                    SPtr += 2; 
                    if(*SPtr == ':')
                    {
                      SPtr++; 
                      if(*SPtr == ' ')Flag_INVALID  = SET;
                      else if((atof(SPtr) > 2.0) || (atof(SPtr) < 0.1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                      else HA = atof(SPtr);                                                // ACTIVE UPDATE INTERVAL    
                      
                      sprintf(GP_Buffer,",SETTD HA,%0.2f",HA);
                      strcat(CC_Source_TER,GP_Buffer);
                    } 
                    else Flag_SYNErr = SET;
                  }           
                if(SPtr = strstr(Ptr,"HB"))
                {
                    SPtr += 2; 
                    if(*SPtr == ':')
                    { 
                      SPtr++;
                      if(*SPtr == ' ')Flag_INVALID  = SET;
                      else if((atof(SPtr) > 2.0) || (atof(SPtr) < 0.1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                      else HB = atof(SPtr);  
                      
                      sprintf(GP_Buffer,",SETTD HB,%0.2f",HB);
                      strcat(CC_Source_TER,GP_Buffer);
                    } 
                    else Flag_SYNErr = SET;      
                  } 
                if(SPtr = strstr(Ptr,"RT"))
                {
                  SPtr += 2; 
                  if(*SPtr == ':')
                  { 
                    SPtr++;
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 360) || (atoi(SPtr) < 10) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else RT = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETTD RT,%d",RT);
                    strcat(CC_Source_TER,GP_Buffer);
                  }
                 else Flag_SYNErr = SET;          
                }        
                if(SPtr = strstr(Ptr,"SL"))
                {
                  SPtr += 2;
                  if(*SPtr == ':')
                  {
                    SPtr++; 
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atoi(SPtr) > 220) || (atoi(SPtr) < 10) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else Speed_Limit = atoi(SPtr); 
                    
                    sprintf(GP_Buffer,",SETTD SL,%d",Speed_Limit);
                    strcat(CC_Source_TER,GP_Buffer);
                  }
                  else Flag_SYNErr = SET; 
                } 
                if(SPtr = strstr(Ptr,"WK"))
                {
                  SPtr += 2;
                  if(*SPtr == ':')
                  {
                    SPtr++;  
                    if(*SPtr == ' ')Flag_INVALID  = SET;
                    else if((atof(SPtr) > 7.9) || (atof(SPtr) < 0.2)  || (*SPtr == ' ')){Flag_INVALID  = SET;}
                    else 
                    {
                      Accel_WK_Thrs = atof(SPtr);                                                // ACTIVE UPDATE INTERVAL          
                    
                      
                      if(Accel_WK_Thrs < 1.0)LIS3DH_WriteReg(0x5B,0x01); //0x02 //wake up threshold
                      else if(Accel_WK_Thrs > 5.0)LIS3DH_WriteReg(0x5B,0x05); //0x02 //wake up threshold
                      else LIS3DH_WriteReg(0x5B,(int)(Accel_WK_Thrs)); //0x02 //wake up threshold
                    }
                  }
                  else Flag_SYNErr = SET;
                }
                if(SPtr = strstr(Ptr,"ID"))
                {
                SPtr += 2;
                if(*SPtr == ':')
                {
                  SPtr++;
                  if(*SPtr == ' ')Flag_INVALID  = SET;
                  else if((atoi(SPtr) > 60) || (atoi(SPtr) < 1) || (*SPtr == ' ')){Flag_INVALID  = SET;}
                  else IDL_Time = atoi(SPtr);                                                // IDLE TIME IN MINUTES  
                }
                else Flag_SYNErr = SET;
              }
            }
             
           } 
          
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET;
           goto END_FUNC;
         }    
        /************************* SETPWD *******************************************/
        else if(Ptr = strstr((char*)Config_Buffer,"PWD "))
        {
         Ptr += 4;
         memset(loc_buffer,0,12);
         for(GP_Counter = 0; GP_Counter < 11;)
         {
           loc_buffer[GP_Counter] = *Ptr;
           Ptr++;
           GP_Counter++;
           if((*Ptr == 0) || (*Ptr == '\r') || (*Ptr == '\n'))break;
         }
         if((GP_Counter < 3) || (GP_Counter > 10)){Flag_INVALID = SET;Flag_SYNErr = RESET;}
         else 
         {
           memset(SER_PWD,0,12);
           strcpy(SER_PWD,loc_buffer);
           sprintf(GP_Buffer,",SETPWD,%s",SER_PWD);
           strcat(CC_Source_TER,GP_Buffer);
           Flag_OK = SET;
         }       
     }
        /*---------------  ONLY SERIAL -------------*/
        /************************* SETUSN *******************************************/
        else if(Ptr = strstr((char*)Config_Buffer,"USN "))
        {
             Ptr += 4;
             memset(loc_buffer,0,12);
             for(GP_Counter = 0; GP_Counter < 11;)
             {
               loc_buffer[GP_Counter] = *Ptr;
               Ptr++;
               GP_Counter++;
               if((*Ptr == 0) || (*Ptr == '\r') || (*Ptr == '\n'))break;
             }
             if((GP_Counter < 3) || (GP_Counter > 10)){Flag_INVALID = SET;Flag_SYNErr = RESET;}
             else 
             {
               memset(SER_USN,0,12);
               strcpy(SER_USN,loc_buffer);
               Flag_OK = SET;
               sprintf(GP_Buffer,",SETUSN,%s",SER_USN);
               strcat(CC_Source_TER,GP_Buffer);
             }       
         }       
        /************************** SETEIP *****************************************/    // ENABLE/DISABLE PRIMARY/SECONDARY/TERTIARY SERVER        
        else if(Ptr = strstr((char*)Config_Buffer,"EIP "))
        {
           Flag_SYNErr = SET;
           Ptr += 4;
           if((*(Ptr+1) == ':') && ((*(Ptr+3) == '\r') || (*(Ptr+3) == '\n')))
           {
             Flag_SYNErr = RESET;
             if(*Ptr == 'P')  
             {
               if(*(Ptr+2) == '1')     
               {
                 if((SRVR_IP[MAIN][0] != 0) && (SRVR_IP[MAIN][0] != ' ') && (SRVR_Port[MAIN] > 0))Flag_MAINServer = SET;
                 else Flag_INVALID = SET;
               }
               else if(*(Ptr+2) == '0')Flag_MAINServer = RESET;
               else Flag_INVALID = SET;
               
               sprintf(GP_Buffer,",SETEIP P,%d",Flag_MAINServer);
               strcat(CC_Source_TER,GP_Buffer);
             }
             else if(*Ptr == 'S') 
             {
                if(*(Ptr+2) == '1')     
                {
                  if((SRVR_IP[EMR][0] != 0) && (SRVR_IP[EMR][0] != ' ') && (SRVR_Port[EMR] > 0))Flag_EMRServer = SET;
                  else Flag_INVALID = SET;
                }
                else if(*(Ptr+2) == '0')Flag_EMRServer = RESET;
                else Flag_INVALID = SET;
                
                sprintf(GP_Buffer,",SETEIP S,%d",Flag_EMRServer);
                strcat(CC_Source_TER,GP_Buffer);
             }
             else if(*Ptr == 'T') 
             {
                if(*(Ptr+2) == '1')     
                {
                   if((SRVR_IP[TER][0] != 0) && (SRVR_IP[TER][0] != ' ') && (SRVR_Port[TER] > 0))Flag_TERServer = SET;
                   else Flag_INVALID = SET;
                }
                else if(*(Ptr+2) == '0')Flag_TERServer = RESET;
                else Flag_INVALID = SET;
                
                sprintf(GP_Buffer,",SETEIP T,%d",Flag_TERServer);
                strcat(CC_Source_TER,GP_Buffer);
             }
             else 
             {
               Flag_SYNErr  = SET; 
               Flag_INVALID = RESET;
               Flag_OK      = RESET;
             }
           } 
           else 
             {
               Flag_SYNErr  = SET; 
               Flag_INVALID = RESET;
               Flag_OK      = RESET;
             }
             
           if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))Flag_OK = SET;
           goto END_FUNC;
         }
        /***************** SETCLRSOS **********************************************/    // CLR SOS FROM TERTIARY SERVER
        else if(Ptr = strstr((char*)Config_Buffer,"CLRSOS "))
        {
           Ptr += 7;
           if(((*Ptr == '0') || (*Ptr == '1')) && ((*(Ptr+1) == '\r') || (*(Ptr+1) == '\n')))
           {
             if(*Ptr == '0')Flag_STOPMSG = RESET;
             else Flag_STOPMSG = SET;
             
              Flag_OK = SET;
              
              sprintf(GP_Buffer,",CLRSOS T,%d",Flag_STOPMSG);
              strcat(CC_Source_TER,GP_Buffer);
           }
           else 
           {
             Flag_INVALID = SET;Flag_SYNErr = RESET;
           }
            goto END_FUNC;
         }        
        /*********************** SETGEO **********************************************/
        else if(Ptr = strstr((char*)Config_Buffer,"GEO "))
        {
           //Flag_SYNErr = RESET;
           Ptr += 4;
           SPtr = Ptr;
           
           if((*Ptr >= '0') && (*Ptr <= '9'))                                   // VALID ROUTE ID 
           {
             if(*(Ptr+1) == ',')
             {
               if((*(Ptr+2) >= '0') && (*(Ptr+2) <= '9'))                       // VALID FENCE ID 
               {
                 if(*(Ptr+3) == ',')
                 {
                   if(((atof(Ptr+4)) > -91)  && ((atof(Ptr+4)) < 91)) 
                   {
                      Ptr += 4;
                      if(Ptr = strstr(Ptr,","))
                      {
                        Ptr++;
                         if(((atof(Ptr)) > -181)  && ((atof(Ptr)) < 181)) 
                         {
                            
                           GF[*SPtr - 48].ID = *(SPtr) - 48;
                           GF[*SPtr - 48].LAT[*(SPtr+2) - 48] = atof(SPtr + 4);
                           GF[*SPtr - 48].LON[*(SPtr+2) - 48] = atof(Ptr);
                           Flag_SYNErr = RESET;
                           Flag_OK = SET;
                            
                         }
                         else {Flag_INVALID = SET;Flag_SYNErr = RESET; }
                      }
                      else 
                       {
                         Flag_SYNErr  = SET; 
                         Flag_INVALID = RESET;
                         Flag_OK      = RESET;
                         
                        }
                   }
                   else{Flag_INVALID = SET;Flag_SYNErr = RESET;}
                 }
                 else 
                 {
                   Flag_SYNErr  = SET; 
                   Flag_INVALID = RESET;
                   Flag_OK      = RESET;
                 }
               }
               else Flag_INVALID = SET;
             }
             else 
             {
               Flag_SYNErr  = SET; 
               Flag_INVALID = RESET;
               Flag_OK      = RESET;

             }
           }
            else 
            {Flag_INVALID = SET;Flag_SYNErr = RESET;}
            goto END_FUNC;             
        }       
        // SERVER IP 1
        else if(Ptr = strstr((char*)Config_Buffer,"IP1-"))
        {
         
         Ptr += 4;
         SPtr = Ptr;
         for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
         {
           if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
           {
             Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
             break;
           }
           else 
            {
               if(*Ptr == ' ')
               {
                 Flag_INVALID = SET;
                 Flag_SYNErr = RESET;
                 Flag_OK = RESET;
                 break;
               }
            }
         } 
         
         if(GP_Counter > 49)Flag_INVALID = SET;
         if(GP_Counter < 4)
         {
           Flag_INVALID = SET;
           Flag_SYNErr = RESET;
           Flag_OK = RESET;
         }
        
        
        if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
        {
         Ptr = SPtr;
         memset(&SRVR_IP[MAIN][0],0,50);
         for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
         {
           if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[MAIN][GP_Counter] = 0;break;}
            else 
            {
               SRVR_IP[MAIN][GP_Counter] = *Ptr;
            }
         }
         sprintf(GP_Buffer,",SETIP1,%s",SRVR_IP[MAIN]);
         strcat(CC_Source_TER,GP_Buffer);
         Flag_OK = SET; 
           // IF CHANGED CLOSE CONNECTION
          Flag_TCPConnect[MAIN] = RESET;Modem_PutString("AT+QICLOSE=0"); 
         if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
        }
       goto END_FUNC;
       }   
        // SERVER PORT 1
        else if(Ptr = strstr((char*)Config_Buffer,"PORT1-"))
        {
           Ptr += 6;
           if(atoi(Ptr) < 65536)
           {
             SRVR_Port[MAIN] = atoi(Ptr);
             Flag_OK = SET;
             
             sprintf(GP_Buffer,",SETPORT1,%d",SRVR_Port[MAIN]);
             strcat(CC_Source_TER,GP_Buffer);
           }
         else 
         {
           Flag_INVALID = SET;
         }
           
           
         // IF CHANGED CLOSE CONNECTION
          Flag_TCPConnect[MAIN] = RESET;Modem_PutString("AT+QICLOSE=0"); 
         if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
         goto END_FUNC;
         }
        // SERVER IP 2
        else if(Ptr = strstr((char*)Config_Buffer,"IP2-"))
        {        
           Ptr += 4;
           SPtr = Ptr;
           for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
           {
             if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
             {
               Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
               break;
             }
             else 
              {
                 if(*Ptr == ' ')
                 {
                   Flag_INVALID = SET;
                   Flag_SYNErr = RESET;
                   Flag_OK = RESET;
                   break;
                 }
              }
           } 
           
           if(GP_Counter > 49)Flag_INVALID = SET;
           
           if(GP_Counter < 4)
           {
             Flag_INVALID = SET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
           }
          
          
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
          {
           Ptr = SPtr;
           memset(&SRVR_IP[EMR][0],0,50);
           for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
           {
             if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[EMR][GP_Counter] = 0;break;}
              else 
              {
                 SRVR_IP[EMR][GP_Counter] = *Ptr;
              }
           }  
           Flag_OK = SET;
           sprintf(GP_Buffer,",SETIP2,%s",SRVR_IP[EMR]);
           strcat(CC_Source_TER,GP_Buffer);
         
            Flag_TCPConnect[EMR] = RESET;Modem_PutString("AT+QICLOSE=2"); 
           if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);  
          }
          goto END_FUNC;
         }      
        // SERVER PORT 2
        else if(Ptr = strstr((char*)Config_Buffer,"PORT2-"))
        {
           
           Ptr += 6;
           if(atoi(Ptr) < 65536)
           {
             SRVR_Port[EMR] = atoi(Ptr);
             Flag_OK = SET;
             sprintf(GP_Buffer,",SETPORT2,%d",SRVR_Port[EMR]);
             strcat(CC_Source_TER,GP_Buffer);
           }
          else 
          {
            Flag_INVALID = SET;
          }
         // IF CHANGED CLOSE CONNECTION
            Flag_TCPConnect[EMR] = RESET;Modem_PutString("AT+QICLOSE=2"); 
           if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
           goto END_FUNC;
         }
        // SERVER IP 3
        else if(Ptr = strstr((char*)Config_Buffer,"IP3-"))
        {        
           Ptr += 4;
           SPtr = Ptr;
           for(GP_Counter = 0; GP_Counter < 51;GP_Counter++,Ptr++)
           {
             if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))
             {
               Flag_INVALID = RESET;Flag_SYNErr = RESET;Flag_OK = RESET;
               break;
             }
             else 
              {
                 if(*Ptr == ' ')
                 {
                   Flag_INVALID = SET;
                   Flag_SYNErr = RESET;
                   Flag_OK = RESET;
                   break;
                 }
              }
           } 
           
           if(GP_Counter > 49)Flag_INVALID = SET;
           
           if(GP_Counter < 4)
           {
             Flag_INVALID = SET;
             Flag_SYNErr = RESET;
             Flag_OK = RESET;
           }
          
          
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET) && (GP_Counter > 3))
          {
           Ptr = SPtr;
           memset(&SRVR_IP[TER][0],0,50);
           for(GP_Counter = 0; GP_Counter < 50;GP_Counter++,Ptr++)
           {
             if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n')){SRVR_IP[TER][GP_Counter] = 0;break;}
              else 
              {
                 SRVR_IP[TER][GP_Counter] = *Ptr;
              }
           }  
           sprintf(GP_Buffer,",SETIP3,%s",SRVR_IP[TER]);
           strcat(CC_Source_TER,GP_Buffer);
           Flag_OK = SET;
            Flag_TCPConnect[TER] = RESET;Modem_PutString("AT+QICLOSE=1"); 
           if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
          }
          goto END_FUNC;
         }
        // SERVER PORT 3
        else if(Ptr = strstr((char*)Config_Buffer,"PORT3-"))
        {
           
           Ptr += 6;
           if(atoi(Ptr) < 65536)
           {
             SRVR_Port[TER] = atoi(Ptr);
             Flag_OK = SET;
             
             sprintf(GP_Buffer,",SETPORT3,%d",SRVR_Port[TER]);
             strcat(CC_Source_TER,GP_Buffer);
           }
           else 
           {
             Flag_INVALID = SET;
           }
         // IF CHANGED CLOSE CONNECTION
         Flag_TCPConnect[TER] = RESET;Modem_PutString("AT+QICLOSE=1"); 
         if(Flag_SerialConfig == RESET)WAIT_MODEM_RESP(10);
         goto END_FUNC;
         }
        else if(Ptr = strstr((char*)Config_Buffer,"TT-"))
        {
         Flag_INVALID = RESET;
         Flag_SYNErr = RESET;
         Flag_OK = RESET;
         Ptr += 3;
         if((atoi(Ptr) > 300) || (atoi(Ptr) < 10))
          {
             Flag_INVALID = SET;
          }
          else 
          {
            Trans_Time = atoi(Ptr);
            Flag_OK = SET;
          }
         goto END_FUNC;
      }     
        // NEW PASSWORD 
        else if(Ptr = strstr((char*)Config_Buffer,"PWD-"))
        {
          Flag_INVALID = RESET;
          Flag_OK = RESET;
          Ptr += 4;
          SPtr = Ptr;
          if((Ptr = strchr(SPtr,',')) ||(Ptr = strchr(SPtr,'\r')) || (Ptr = strchr(SPtr,'\n')))
          {
            if(((Ptr - SPtr) < 5) && (Ptr > SPtr))
            {
              Ptr = SPtr;
              for(GP_Counter = 0; GP_Counter < 4;GP_Counter++,Ptr++) 
              {
                  if((*Ptr == ',') || (*Ptr == '\r') || (*Ptr == '\n'))break;
                  if(((*Ptr >= 'A') && (*Ptr <= 'Z')) || ((*Ptr >= 'a') && (*Ptr <= 'z'))|| ((*Ptr >= '0') && (*Ptr <= '9')));
                  else 
                  {
                    Flag_INVALID = SET; break;
                  }
              }
              
            }
            else Flag_INVALID = SET;
          }
        if(GP_Counter < 2)
          {
              Flag_INVALID = SET;
          }
        else if(Flag_INVALID == RESET)  
        {
        memset(PWD_IN,0,sizeof(PWD_IN));
        for(GP_Counter = 0; GP_Counter < 4;GP_Counter++,SPtr++) 
          {
            if((*SPtr == ',') || (*SPtr == '\r') || (*SPtr == '\n') || (*SPtr == '\0')){PWD_IN[GP_Counter] = 0;break;}
            else 
            {
                PWD_IN[GP_Counter] = *SPtr;
             }
          }
          sprintf(GP_Buffer,",SETPWD-,%s",PWD_IN);
          strcat(CC_Source_TER,GP_Buffer);
        
          Flag_OK = SET;
        }
      
   goto END_FUNC;
  }
        else if(Ptr = strstr((char*)Config_Buffer,"MEMS-E")){Flag_MEMS_ENB = SET; Flag_OK = SET; goto END_FUNC;}
        else if(Ptr = strstr((char*)Config_Buffer,"MEMS-D")){Flag_MEMS_ENB = RESET; Flag_OK = SET; goto END_FUNC;}     
        // SIM PROFILE SWAPPING REQUEST 
        else if(Ptr = strstr((char*)Config_Buffer,"SP-"))                         //Sim Profile
        {
          Ptr += 3; 
          SP_REQ = *Ptr-48;
          if(SP_REQ == 0 || SP_REQ == 1 || SP_REQ == 2 || SP_REQ == 3)
          {
            Flag_SWAP = SET;
            Flag_OK = SET;
          }
          else 
          {
            Flag_INVALID = SET;
          }
          goto END_FUNC;
        }
        // SIM SWAPPING ENABLE DISBALE 
        else if(Ptr = strstr((char*)Config_Buffer,"SWAP-"))                       //SWAPPING  
        {
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          Flag_OK = RESET;
          Ptr += 5; 
          if(*Ptr == '1' || *Ptr == '0')
          {
            if(*Ptr == '1')Flag_SWAP_ENB = SET;
            else Flag_SWAP_ENB = RESET;
          }
          else
          {
            Flag_INVALID = SET;
          }
           
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           {
             Flag_OK = SET;
           }
           goto END_FUNC;
        } 
        else if(Ptr = strstr((char*)Config_Buffer,"SAT-"))                        //Sleep Again Time  
        {
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          Flag_OK = RESET;
          Ptr += 4; 
          if(*Ptr >= '0' || *Ptr <= '9')
          {
            if((atoi(Ptr) > 179) && (atoi(Ptr) < 601))
            {
             SleepAgainTime = atoi(Ptr);
            }
            else Flag_INVALID = SET;
          }
          else
          {
            Flag_INVALID = SET;
          }
           
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           {
             Flag_OK = SET;
           }
           goto END_FUNC;
        }
        else if(Ptr = strstr((char*)Config_Buffer,"EBL-"))                        // EBL 
        {
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          Flag_OK = RESET;
          Ptr += 4; 
          if(*Ptr >= '0' || *Ptr <= '9')
          {
            if((atoi(Ptr) > 4) && (atoi(Ptr) < 31))
            {
             EBL = atoi(Ptr);
             Flag_Check_EXT_PWR = SET;
            }
            else Flag_INVALID = SET;
          }
          else
          {
            Flag_INVALID = SET;
          }
           
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           {
             Flag_OK = SET;
             
             sprintf(GP_Buffer,",SETTD ELB,%d",EBL);
             strcat(CC_Source_TER,GP_Buffer);
           }
           goto END_FUNC;
        }
        else if(Ptr = strstr((char*)Config_Buffer,"DSF-"))                        // DeepSleepFreq 
        {
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          Flag_OK = RESET;
          Ptr += 4; 
          if(*Ptr >= '0' || *Ptr <= '9')
          {
            if((atoi(Ptr) > 59) && (atoi(Ptr) < 86401))
            {
             DeepSleepFreq = atoi(Ptr);
            }
            else Flag_INVALID = SET;
          }
          else
          {
            Flag_INVALID = SET;
          }
           
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           {
             Flag_OK = SET;
           }
           goto END_FUNC;
        }    
        // DEEP SLEEP FRQ ENABLE DISBALE 
        else if(Ptr = strstr((char*)Config_Buffer,"DSC-"))                        //DSC ENABLE DISABLE  
        {
          Flag_INVALID = RESET;
          Flag_SYNErr = RESET;
          Flag_OK = RESET;
          Ptr += 4; 
          if(*Ptr == '1' || *Ptr == '0')
          {
            if(*Ptr == '1')Flag_DSF_ENB = SET;
            else Flag_DSF_ENB = RESET;
          }
          else
          {
            Flag_INVALID = SET;
          }
           
          if((Flag_INVALID == RESET) && (Flag_SYNErr == RESET))
           {
             Flag_OK = SET;
           }
           goto END_FUNC;
        } 
        else if(Ptr = strstr((char*)Config_Buffer,"ED-"))
        {
         Ptr += 3;
         if(atoi(Ptr) > 7200 || atoi(Ptr) < 60)Flag_INVALID = SET;
         else 
         {
           ED_Time = atoi(Ptr);
           Flag_OK = SET;
           sprintf(GP_Buffer,",SETED,%d",ED_Time);
           strcat(CC_Source_TER,GP_Buffer);
           
         }
         goto END_FUNC;
       }
        else 
        {
           Flag_SYNErr  = SET;
           Flag_INVALID = RESET;
           Flag_OK      = RESET;
        }
      }
     else if(Ptr = strstr((char*)Config_Buffer,"CLR"))
     {
         if(Ptr = strstr(Config_Buffer,"PKT"))
         {
            TCP_BackCnt[MAIN] = 0;
            TCP_BackCnt[TER] = 0;
            EMRGY_BKP_No = 0;
            
            RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
                       
            FlashAddress = SPIFLASH_CONFIG;
            memset(loc_buffer,0,50);
            sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
               
             if(Flag_SPIOK == SET)
              {
               SPI_FLASH_SectorErase(FlashAddress);
               SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
              }
             
             Flag_OK = SET;
             Flag_SKIP = SET;
          }
         else if(Ptr = strstr(Config_Buffer,"MOB "))
         {
   
           Ptr += 4;
           if((*Ptr > '0') && (*Ptr <= '5'))
           {
              memset(&Vehicle.Master_No[*Ptr - 49][0],0,15);
              sprintf(&Vehicle.Master_No[*Ptr - 49][0],"N.A0000000"); 
              Flag_OK = SET;
              
           }
           else Flag_INVALID = SET;
         }
         else if(Ptr = strstr(Config_Buffer,"MOBALL"))
         {    
            memset(&Vehicle.Master_No[0][0],0,15);
            memset(&Vehicle.Master_No[1][0],0,15);
            memset(&Vehicle.Master_No[2][0],0,15);
            memset(&Vehicle.Master_No[3][0],0,15);
            memset(&Vehicle.Master_No[4][0],0,15);
            
            sprintf(&Vehicle.Master_No[0][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
            sprintf(&Vehicle.Master_No[1][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
            sprintf(&Vehicle.Master_No[2][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
            sprintf(&Vehicle.Master_No[3][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
            sprintf(&Vehicle.Master_No[4][0],"N.A0000000");                           // DEFAULT MASTER NUMBER SET TO NONE
        
            Flag_OK = SET;
            
         }
         else if(strstr((char*)Config_Buffer,"@CLR#"))
         {
           // CLEAR ODDO
           if(Ptr = strstr((char*)Config_Buffer,"ODDO"))
           {
              Ptr += 4;
              MTR = 0; KM = 0;
              FlashAddress = SPIFLASH_CONFIG_ODO;
              memset(loc_buffer,0,30);
              sprintf(loc_buffer,"KM-%u,MTR-%hu",KM,MTR);  
              if(Flag_SPIOK == SET)
              {
               SPI_FLASH_SectorErase(FlashAddress);
               SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
              } 
              Flag_OK = SET;
           }
         }      
         else if(Ptr = strstr((char*)Config_Buffer,"CLRSOS"))
         {
           if(Flag_STOPMSG == SET)
            {
              ED_Timer = 0;
              Flag_Emrgcy = RESET;
              
              Save_EMR();
              
              Flag_EA = RESET;
              Flag_EA_OFF = SET; 
              Flag_OK = SET;
              Flag_MAIN_PKT = SET;
              Flag_TER_PKT = SET;
              ServerHit_Time = 2; 
              TER_ServerHit_Time = TER_HitTime + 3;
              
              Flag_EM_LED = RESET; 
              Get_EMRGY_PKT(); 
              Flag_HitServ = SET;  
            }
            else 
            {
              Flag_OK = RESET;
              Flag_INVALID = RESET;
              Flag_SYNErr = SET;
              goto END_FUNC;
            }
         }
         else 
         {
             Flag_SYNErr = SET;
         }
     }        
     /*****************************************************************************/      
     /*---------- GET COMMANDS , ALARM, ALERTS ------------------------------------*/
     else if((strstr((char*)Config_Buffer,"GET")) || (strstr((char*)Config_Buffer,"@GET#")))
     {
       Flag_OK = RESET;
       Flag_INVALID = RESET;
       Flag_SYNErr = RESET;
       Flag_SKIP = SET;
       if(strstr((char*)Config_Buffer,"CONFIG"))Send_SMS(CONFIG,RESET);
       else if(strstr((char*)Config_Buffer,"CONF"))Send_SMS(CONF,RESET);
       else if(strstr((char*)Config_Buffer,"DEBUG"))Send_SMS(DEBUG,RESET);
       else if(Ptr = strstr((char*)Config_Buffer,"IOSTS"))Send_SMS(IOSTATUS,RESET);
       else if(strstr((char*)Config_Buffer,"STS"))Send_SMS(GETSTS,RESET);
       else if(strstr((char*)Config_Buffer,"LOC"))  Send_SMS(LOCATION,RESET);
       else if(strstr((char*)Config_Buffer,"GPS"))  Send_SMS(GPSDATA,RESET);
       else if(strstr((char*)Config_Buffer,"GPRS")) Send_SMS(GPRSDATA,RESET);
       else if(strstr((char*)Config_Buffer,"INFO")) Send_SMS(INFO,RESET);
       else if(strstr((char*)Config_Buffer,"DFU"))  {Send_SMS (DFU_DETAIL,RESET);}
       else if(Ptr = strstr((char*)Config_Buffer,"MOB "))
       {  
         Ptr += 4;
         if((*(Ptr + 1) == '\r') || (*(Ptr + 1) == '\n'))
         {
           //GetConfig_Server();
           Flag_OTACFG = SET;
           if(*Ptr == '1')      { Send_SMS(MOBNUM1,RESET);sprintf(GP_Buffer,",GET MOB 1,%s",Vehicle.Master_No[0]);}
           else if (*Ptr == '2'){ Send_SMS(MOBNUM2,RESET);sprintf(GP_Buffer,",GET MOB 2,%s",Vehicle.Master_No[1]);}
           else if (*Ptr == '3'){ Send_SMS(MOBNUM3,RESET);sprintf(GP_Buffer,",GET MOB 3,%s",Vehicle.Master_No[2]);}
           else if (*Ptr == '4'){ Send_SMS(MOBNUM4,RESET);sprintf(GP_Buffer,",GET MOB 4,%s",Vehicle.Master_No[3]);}
           else if (*Ptr == '5'){ Send_SMS(MOBNUM5,RESET);sprintf(GP_Buffer,",GET MOB 5,%s",Vehicle.Master_No[4]);}
           else 
           {
             sprintf(GP_Buffer,",UNKNOWN MOB,N.A");
             Send_SMS(SYN_ERR,RESET);
           }
         }
         else { sprintf(GP_Buffer,",UNKNOWN MOB,N.A"); Send_SMS(SYN_ERR,RESET);};
         strcat(CC_Source_TER,GP_Buffer);
      }
       else if(Ptr = strstr((char*)Config_Buffer,"VER"))Send_SMS(HW_VER,RESET);
       else if(Ptr = strstr((char*)Config_Buffer,"SMSPKT"))Send_SMS(SMSPKT,RESET);
       else if(Ptr = strstr((char*)Config_Buffer,"FENCE "))
       {
          Ptr += 6;
          if((*Ptr >= '0') && (*Ptr <= '9'))
          {
            R_ID = *Ptr - 48;
            Send_SMS(GEO_FENCE,RESET);
          }
          else Flag_INVALID = SET;
        }
       else if(Ptr = strstr((char*)Config_Buffer,"GEO "))
       {
           Ptr += 4;
           if((*Ptr >= '0') && (*Ptr <= '9'))
           {
             R_ID   = *Ptr - 48;
             Ptr++;
             if(*Ptr == ',')
             {
               Ptr++;
               if((*Ptr >= '0') && (*Ptr <= '9'))
               {
                  FNC_ID = *Ptr - 48;
                  Send_SMS(GEOFENCE,RESET);
               }
              else 
              {
                Send_SMS(INVALID,RESET);
              } 
             }
             else 
              {
                Send_SMS(SYN_ERR,RESET);
               
              }
           }
           else 
           {
             Send_SMS(INVALID,RESET);
           }
         }
       else if(Ptr = strstr((char*)Config_Buffer,"NUMBER:"))
       {
           Ptr += 7;
           if(*Ptr == '0')Ptr += 1;
           else if(*Ptr == '+')Ptr += 3;
           if((*Ptr > '0') && (*Ptr <= '9'))
           {
              SPtr = Ptr;
              if((Ptr = strstr(Ptr,"\r")) || (Ptr = strstr(Ptr,"\n")) || (Ptr = strstr(Ptr,",")))
                  {
                    memset(loc_buffer,0,20);           
                     if(((Ptr-SPtr) == 13) || ((Ptr-SPtr) == 10))
                      {
                       strncpy(loc_buffer,SPtr,Ptr-SPtr); 
                       Ptr= &loc_buffer[0];
                       while(*Ptr != '\0'){if(*Ptr < '0' || *Ptr > '9'){Flag_INVALID = SET;Flag_SYNErr = RESET;break;} Ptr++;} 
                       
                       if(Flag_INVALID == RESET)
                       {
                         Flag_SYNErr = RESET;
                          memcpy(&GPRSData.Mobile_No[0],loc_buffer,15);
                          Flag_GETMNO = SET;
                          Flag_OK = SET;
                       }
                      }
                    else 
                    {
                      Flag_INVALID = SET;
                      Flag_SYNErr = RESET;
                      Flag_OK = RESET;
                    }
                  }
           }
           
         }
       else if(Ptr = strstr((char*)Config_Buffer,"HPKT"))
       {
          Send_SMS(HEALTH,RESET);
        }
       else if(Ptr = strstr((char*)Config_Buffer,"PARAMS"))
       {
          Send_SMS(PARAMS,RESET);
        }
       else if(Ptr = strstr((char*)Config_Buffer,"VEHID"))
       {
         Send_SMS(GETVID,RESET);
         //GetConfig_Server();
          Flag_OTACFG = SET;
         sprintf(GP_Buffer,",GETVEHID,%s",&Vehicle.VRN[0]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"TD "))
       {
         GetConfig_Server();
         if(strstr((char*)Config_Buffer,"TD BL"))sprintf(GP_Buffer,",GET TD BL,%d",IBL);
         else if(strstr((char*)Config_Buffer,"TD HA"))sprintf(GP_Buffer,",GET TD HA,%0.2f",HA);
         else if(strstr((char*)Config_Buffer,"TD HB"))sprintf(GP_Buffer,",GET TD HB,%0.2f",HB);
         else if(strstr((char*)Config_Buffer,"TD RT"))sprintf(GP_Buffer,",GET TD RT,%d",RT);
         else if(strstr((char*)Config_Buffer,"TD SL"))sprintf(GP_Buffer,",GET TD SL,%d",Speed_Limit);
         else
         {
           Flag_OTACFG = RESET;
           sprintf(GP_Buffer,",UNKNOWN COMMAND,0");
         }
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"UI "))
       {
         GetConfig_Server();
         if(strstr((char*)Config_Buffer,"UI A"))sprintf(GP_Buffer,",GET UI A,%d",Active_Time);
         else if(strstr((char*)Config_Buffer,"UI O"))sprintf(GP_Buffer,",GET UI O,%d",StandBy_Time);
         else if(strstr((char*)Config_Buffer,"UI S"))sprintf(GP_Buffer,",GET UI S,%d",StopMode_Time);
         else if(strstr((char*)Config_Buffer,"UI H"))sprintf(GP_Buffer,",GET UI H,%d",HLTH_FRQ);
         else
         {
           Flag_OTACFG = RESET;
           sprintf(GP_Buffer,",UNKNOWN COMMAND,0");
         }
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"IP1"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP1,%s",SRVR_IP[MAIN]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"IP2"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP2,%s",SRVR_IP[EMR]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"IP3"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET IP3,%s",SRVR_IP[TER]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"PORT1"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT1,%d",SRVR_Port[MAIN]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"PORT2"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT2,%d",SRVR_Port[EMR]);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"PORT3"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET PORT3,%d",SRVR_Port[TER]);
         strcat(CC_Source_TER,GP_Buffer);
       }     
       else if(Ptr = strstr((char*)Config_Buffer,"ED "))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET ED,%d",ED_Time);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(Ptr = strstr((char*)Config_Buffer,"CP A"))
       {
         GetConfig_Server();
         sprintf(GP_Buffer,",GET CP A,%s",GPRSData.SET_APN);
         strcat(CC_Source_TER,GP_Buffer);
       }
       else 
       {
          Send_SMS(SYN_ERR,RESET);
       }
      
     }
     else if(Ptr = strstr((char*)Config_Buffer,"GPSD "))
     {
       Flag_SKIP = SET;
       Ptr += 5;
       if((*Ptr == '0') || (*Ptr == '1'))
       {
         Flag_OK = SET;
         if(*Ptr == '0')Flag_Debug_GNSS = RESET;
         else if(*Ptr == '1')
         {
           Flag_Debug_GNSS = SET;
           Flag_Debug_GPRS = RESET;
         }
       }
      else 
       {
         Flag_SYNErr = RESET;
         Flag_INVALID = SET;
         Flag_OK = RESET;
        
       }
     }
     else if(Ptr = strstr((char*)Config_Buffer,"GPSC "))
     {
       Flag_SKIP = SET;
       Ptr += 5;
       if((*Ptr == 'H') || (*Ptr == 'C') || (*Ptr == 'W'))
       {
         Flag_OK = SET;
         if(*Ptr == 'H')GNSS_PutString("$PSTMHOT");
         else if(*Ptr == 'C')GNSS_PutString("$PSTMCOLD");
         else if(*Ptr == 'W')GNSS_PutString("$PSTMWARM");
       }
      else 
       {
          Flag_SYNErr = RESET;
          Flag_INVALID = SET;
          Flag_OK = RESET;
         
       }
     }
     else if(Ptr = strstr((char*)Config_Buffer,"GSMD "))
     {
       Flag_SKIP = SET;
       Ptr += 5;
       if((*Ptr == '0') || (*Ptr == '1'))
       {
         Flag_OK = SET;
         if(*Ptr == '0')Flag_Debug_GPRS = RESET; 
         else if(*Ptr == '1')
         {
           Flag_Debug_GPRS = SET;
           Flag_Debug_GNSS = RESET;
         }
       }
      else 
       {
          Flag_SYNErr = RESET;
          Flag_INVALID = SET;
          Flag_OK = RESET;
        
       }
     }      
     else if(Ptr = strstr((char*)Config_Buffer,"RST "))
     {
       Flag_SKIP = SET;
       Flag_SYNErr  = RESET;
       Flag_INVALID = RESET;
       Flag_OK      = RESET;
       Ptr += 4;
       if(*Ptr == 'S') 
       {
         Flag_SysReset = SET; 
         Flag_OK = SET;
         Flag_TxENB = RESET;
         Flag_LogIn = RESET;
         Flag_Debug_GNSS = RESET;
         Flag_Debug_GPRS = RESET;
         GetConfig_Server();
         sprintf(GP_Buffer,",RST S");
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(*Ptr == 'M') 
       {
         Flag_MODULE_M66_RST = SET; 
         Flag_OK = SET;
         GetConfig_Server();
         sprintf(GP_Buffer,",RST M");
         strcat(CC_Source_TER,GP_Buffer);
       }
       else if(*Ptr == 'H') 
       {
         memset(GPRS_SendBuffer,0,strlen(GPRS_SendBuffer));
         Flag_Serial = SET;   
          if(Vehicle.Ignition[0] == 'F')
          {
            Serial_PutString_232("OK",3);
            Flag_Serial = RESET;
            TAKE_FUL_BKP(); 
            Write_Oddo(); 
            REG_LM2576(DISABLE); 
          }
          else 
          {
            Serial_PutString_232("NOT APPLICABLE",3);
            Flag_Serial = RESET;
          }
       }
       else
       {
           Flag_INVALID = SET;
       }

     }
     else if(strstr((char*)Config_Buffer,"GEOFCTRY"))
     {
       Flag_SYNErr  = RESET;
       Flag_INVALID = RESET;
       Flag_OK      = SET;
       for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)
       { 
          GF[GP_Counter].ID = 0;
          GF[GP_Counter].Last_FS = 0;
          for(FlashAddress = 0; FlashAddress < 10; FlashAddress++)
           {
             GF[GP_Counter].LAT[FlashAddress] = 0;
             GF[GP_Counter].LON[FlashAddress] = 0;
             
           }
       }
       
     }
     else if(strstr((char*)Config_Buffer,"FCTRY"))
     {
       Flag_SYNErr  = RESET;
       Flag_INVALID = RESET;
       Flag_OK      = RESET;
       
       Send_SMS(ACKN,RESET);
      
       Flag_FlashError = SET;
       SET_FACTORY();
       if(Flag_FlashError == SET)
       {
        Flag_Serial = SET;
        Serial_PutString_232("Flash Error",3);
        Flag_Serial = RESET;
       }
       memset(GPRS_SendBuffer,0,strlen(GPRS_SendBuffer));
     }
     else if(strstr((char*)Config_Buffer,"@POLL#")) 
     {
     Flag_OK = SET; 
     Flag_SKIP = SET; 
     
     Flag_MAIN_PKT = SET;
     Flag_TER_PKT = SET;
     ServerHit_Time = 2;
     TER_ServerHit_Time = TER_HitTime + 3;
   }         
  
END_FUNC:
  
/*------------ CHECK FOR VALID DATA ------------------------------------------*/
   if((Active_Time  > 3600)  || (Active_Time  < 5)) Active_Time  = 10;
   if((StandBy_Time > 3600)  || (StandBy_Time < 5)) StandBy_Time = 300;
   if((ES_Time      > 3600)  || (ES_Time      < 5)) ES_Time      = 30;
   if((Trans_Time   > 60)    || (Trans_Time  < 10)) Trans_Time = 20;
   if((StopMode_Time > 3600) || (StopMode_Time < 5))StopMode_Time = 1800;
   if(Time2Sleep   > 3600)Time2Sleep   = 60;
   if((OVS_DeadTime > 3600)  || (OVS_DeadTime  < 5))OVS_DeadTime = 5;
   
   if((TER_HitTime  > 3600)  || (TER_HitTime  < 5)) TER_HitTime  = 10;
   
   if(( SleepAgainTime  > 600)  || (SleepAgainTime    < 180))SleepAgainTime = 180;
   if((Speed_Limit  > 220)   || (Speed_Limit < 10)) Speed_Limit  = 80;
       
   if(HA > 3.5 || HA < 0.1)HA = 2.5;                                      // VALUE IN G-FORCE 
   if(HB > 3.5 || HB < 0.1)HB = 2.5;
   if(RT > 360 || RT < 10) RT = 35;
   if(IBL > 50 || IBL < 5)IBL = 35;
   if(HLTH_FRQ < 5 || HLTH_FRQ > 3600)HLTH_FRQ = 600;
   if(EMG_BP_FREQ    > 60000 || EMG_BP_FREQ    < 50)EMG_BP_FREQ = 3000;
   if(EMG_WC_Timeout > 60000 || EMG_WC_Timeout < 50)EMG_WC_Timeout = 15000;
   if(IDL_Time > 60  || IDL_Time < 1)IDL_Time = 10;
    
   if(ED_Time > 7200 || ED_Time < 60)ED_Time = 300;
   if(DeepSleepFreq > 86400 || DeepSleepFreq < 60) DeepSleepFreq = 86400;
    
   
   if((Flag_ACTV == SET) || (Flag_HCHK == SET))
   {    
     Send_ACTV_HCHK_Resp();
     if(Flag_SerialConfig == SET)
     {
       Flag_SerialConfig = RESET;
       Serial_PutString_232("OK",3);
     }
     Flag_H_A_SMS = SET;//Send_SMS(HLTH_ACTV,SET);
   }    
   else if(Flag_OK == SET)
   {
      if(Flag_SKIP == SET)
      {
        Flag_SKIP = RESET;
        Send_SMS(ACKN,RESET);
      }
      else 
      {
        Flag_FlashError = SET;
        Write_Config();                                                     // WRITE CONFIGURATION VALUE AS DEFAULT & RESET
        if(Flag_FlashError == SET)Send_SMS(FLASH_ERR,RESET);
        else Send_SMS(ACKN,RESET);
        Flag_OK = RESET; 
      }
    }
    else if(Flag_INVALID == SET){Flag_INVALID = RESET; Send_SMS(INVALID,RESET);}
    else if(Flag_SYNErr == SET){Flag_SYNErr = RESET; Send_SMS(SYN_ERR,RESET);}
    if(Flag_SerialConfig == SET)
    {
     // Close connection for IP Change
     if(Flag_TCPCloseAfterResp[MAIN] == SET){Flag_TCPCloseAfterResp[MAIN] = RESET; Flag_TCPConnect[MAIN] = RESET; Modem_PutString("AT+QICLOSE=0");WAIT_MODEM_RESP(2);  }
     if(Flag_TCPCloseAfterResp[TER] == SET){Flag_TCPCloseAfterResp[TER] = RESET;   Flag_TCPConnect[TER] = RESET;  Modem_PutString("AT+QICLOSE=1");WAIT_MODEM_RESP(2); }
     if(Flag_TCPCloseAfterResp[EMR] == SET){Flag_TCPCloseAfterResp[EMR] = RESET;   Flag_TCPConnect[EMR] = RESET;  Modem_PutString("AT+QICLOSE=2");WAIT_MODEM_RESP(2); }

    }
    
    Flag_SerialConfig = RESET;
    memset((char*)Config_Buffer,0,strlen(Config_Buffer));                                         // CLEAR MESSAGE BUFFER

    if(Flag_DFU == SET){Flag_DFU = RESET;Run_IAP();}
 
    if(Flag_OTACFG == SET)
    {
      memset(CC_Source_TER,0,sizeof(CC_Source_TER));
      sprintf(CC_Source_TER,"%s%s",CC_Source,GP_Buffer);
      if(Protocol == BSNL_MH)ServerHit_Time = 2;
      TER_ServerHit_Time = 3;    
      ChkSRVR_Time = 10;
    }
   
}
/*******************************************************************************
* Function Name : Get_Config
* Description   : Get Configuration to server
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void GetConfig_Server(void)
{
  memset(CC_Source_TER,0,sizeof(CC_Source_TER));
  memset(CC_Source,0,sizeof(CC_Source));
  Flag_OTACFG = SET;
  if(Flag_SerialConfig == RESET)  
  {
    if(Flag_ServerConf[MAIN] == SET)strcat(CC_Source_TER,&SRVR_IP[MAIN][0]);
    else if(Flag_ServerConf[TER] == SET)strcat(CC_Source_TER,&SRVR_IP[TER][0]);
    else strcat(CC_Source,GPRSData.Mobile_No);
  }
  else sprintf(CC_Source,"%s","SERIAL");
  
  
}

/*******************************************************************************
* Function Name  : Take_Backup
* Description    : Create a system reset & take backup
* Input          : SERVER 
* Output         : None
* Return         : None
*******************************************************************************/
void Take_Backup(uint8_t SRVR)                                                          // THIS FUNCTION SAVE ALL LOCAL BACKUP ON FLASH MEMORY BEFORE RESTART
{
  char *Ptr,*DPtr;
  Ptr = NULL; DPtr = NULL;
  
  while(RAM_BakCnt[SRVR])
  {
    memset(TCP_BACKUP,0,256);
    DPtr = &TCP_BACKUP[0];                                                      // SET POINTER TO DESTINATION 
    Ptr = &RAM_BACKUP[SRVR][0][0];                                              // SET POINTER TO SOURCE
    memcpy(DPtr,Ptr,256);Get_Backup(SRVR);                                      // SAVE TCP SERVER DATA INTO LOCAL BACKUP
    Ptr = &RAM_BACKUP[SRVR][1][0];DPtr = &RAM_BACKUP[SRVR][0][0];               // DECREMENT SHIFT OF BACKUP REGISTER 
    memmove(DPtr,Ptr,4864);             
    memset(&RAM_BACKUP[SRVR][19],0,256);
    if(RAM_BakCnt[SRVR])RAM_BakCnt[SRVR]--;                                                               
  };

  Ptr = &RAM_BACKUP[SRVR][0][0];  
  memset(Ptr,0,5120);                                                           // CLEAR RAM BACKUP all 7 buffer 

  Flag_HitServ = RESET;
}
/*******************************************************************************
* Function Name  : TAKE_FUL_BKP
* Description    : SAVE ALL FILE BEFORE RESET/RESTART/STANDBY MODE
* Input          : SERVER 
* Output         : None
* Return         : None
*******************************************************************************/
void TAKE_FUL_BKP(void)
{
  Take_Backup(MAIN); 
  Take_Backup(TER); 
  Take_EMRGY_BKP();
}

/*******************************************************************************
* Function Name  : Get_NBRData
* Description    : RECEIVE NEARBY CELL INFO 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_NBRData(void)
{
  
  char *Ptr;
  char loc_buffer[10];
  Cell_CheckTime = 60; 
  memset(loc_buffer,0,10);
  Ptr = NULL;
  DPtr = NULL;
  GP_Counter = 4;
  
  Ptr = &GPRSData.LAC[0];
//  int Lac = (CONVERTHEX(*Ptr) * 64) + (CONVERTHEX(*(Ptr+1)) * 32) + (CONVERTHEX(*(Ptr+2)) * 16) + CONVERTHEX(*(Ptr+3)) ;
  
  // For testing Only
//  sprintf(NBRCell[0].CELLID,"%d",GP_Counter + RTC_Time.RTC_Hours + 203); 
//  sprintf(NBRCell[1].CELLID,"%d",GP_Counter + RTC_Time.RTC_Minutes + 102); 
//  sprintf(NBRCell[2].CELLID,"%d",GP_Counter + RTC_Time.RTC_Seconds + 143); 
//  
//  sprintf(NBRCell[0].LAC,"%04X",Lac + 3); 
//  sprintf(NBRCell[1].LAC,"%04X",Lac + 4); 
//  sprintf(NBRCell[2].LAC,"%04X",Lac + 2); 
//  
//  NBRCell[0].SIG = -103;
//  NBRCell[1].SIG = -112;
//  NBRCell[2].SIG = -121;
  
//  while(GP_Counter)
//  {
//    GP_Counter--;
//    sprintf(NBRCell[GP_Counter].CELLID,"%d",GP_Counter + ((RTC_MM+1) * 3)); 
//    int Lac;
//    sscanf(Lac,"%x",GPRSData.LAC);
//    sprintf(NBRCell[GP_Counter].LAC,"%d",); 
//    NBRCell[GP_Counter].SIG = 0;
//  }
    
  Modem_PutString("AT+QENG=\"servingcell\"");WAIT_MODEM_RESP(3);

  if(Ptr = strstr((char*)GPRS_Buffer,"LTE"))
  {
    char *SC;
    SC = strtok(Ptr, ",");
    SC = strtok( NULL, ",");
    SC = strtok( NULL, ",");
    GPRSData.MCC = atoi(SC);
    SC = strtok( NULL, ",");
    GPRSData.MNC = atoi(SC);
  }
  else if(Ptr = strstr((char*)GPRS_Buffer,"GSM"))
  {
    char *SC;
    SC = strtok(Ptr, ",");
    SC = strtok( NULL, ",");
    GPRSData.MCC = atoi(SC);
    SC = strtok( NULL, ",");
    GPRSData.MNC = atoi(SC);
  }
  
   
  Modem_PutString("AT+QENG=\"neighbourcell\"");WAIT_MODEM_RESP(5);
  if(Ptr = strstr((char*)GPRS_Buffer,"LTE"))
  {
    for(int CC = 0; CC < 3; CC++)
    {
      char *NC;
      NC = strtok(Ptr, ",");
      NC = strtok( NULL, ",");
      sprintf((char*)NBRCell[CC].CELLID,"%s",NC);
      NC = strtok( NULL, ",");
      NC = strtok( NULL, ",");
      NBRCell[CC].SIG = atoi(NC);
      Ptr += 20;
      Ptr = strstr(Ptr,"LTE");
      
      NBRCell[CC].CELLID[4] = 0; // to restrict Cell ID to 4 digits as it overflows in TCP packet for TER
      NBRCell[CC].LAC[4] = 0;
    }
  }
  else if(Ptr = strstr((char*)GPRS_Buffer,"GSM"))
  {
    for(int CC = 0; CC < 3; CC++)
    {
      char *NC;
      if(Ptr > 0)
      {
        NC = strtok(Ptr, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        sprintf((char*)NBRCell[CC].LAC,"%s",NC);
        NC = strtok( NULL, ",");
        sprintf((char*)NBRCell[CC].CELLID,"%s",NC);   
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NC = strtok( NULL, ",");
        NBRCell[CC].SIG = atoi(NC);
        NC += 10;
        Ptr = strstr(NC,"GSM");
        
        NBRCell[CC].CELLID[4] = 0; // to restrict Cell ID to 4 digits as it overflows in TCP packet for TER
        NBRCell[CC].LAC[4] = 0;
      }
    }
  }

}

/*******************************************************************************
* Function Name  : Waiting_Reply
* Description    : Wait for the command received by SIM action  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Waiting_Reply(void)
{
  uint8_t UpLoad_Time;
  UpLoad_Time = 0;
  GPRS_Buffer[30] = 0;
  GPRS_Buffer[60] = 0;
  GPRS_Buffer[498] = 0;
  GPRS_Buffer[499] = 0;
  GPRS_Count = 0;
   
  UpLoad_Time = 10;
  
  while(Flag_SWAPCMD_OK == RESET && UpLoad_Time > 0 && Flag_Error == RESET)
   {
     if(Flag_Second == SET)
       {
         Flag_Second = RESET; UpLoad_Time--;IWDG_ReloadCounter();               // DECREMENT UPLOAD TIME 
         if(GNSS_Time <= 1 ){GNSS_Time = GNSS_INTERVAL;if(Flag_IRNSSFix){Get_Location();}};
       
         if(Flag_SerialConfig == SET)
         {
           Check_Config();
           Flag_SerialConfig = RESET;
         }
       };
     if(Flag_KeyScan == SET){Flag_KeyScan = RESET;Key_InputScan(); }            // KEY SCAN EVERY 200 MILI SECONDS  
                
   }
}


/*******************************************************************************
* Function Name  : Get_EMGY_SMS
* Description    : GENERATE HEALTH PACKET @ dedicated frequency or on demand 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_EMGY_SMS(void)
{
   char buffer[100];
   char BUF[10];
   memset(buffer,0,100);
   memset(BUF,0,10);
   memset(EM_BUFFER,0,256);
  
   if(Flag_IRNSSFix)
   {
     sprintf(buffer,"%s,%s%s1,%s%s,%s,",GPRSData.IMEI_No,IRNSSData.LAT,IRNSSData.LON,IRNSSData.SPEED,GPRSData.Cell_ID,GPRSData.LAC);
   }
   else 
   {
     sprintf(buffer,"%s,000.000000,0,000.000000,0,0,0,%s,%s,",GPRSData.IMEI_No,GPRSData.Cell_ID,GPRSData.LAC); 
   }
   strcpy(EM_BUFFER,buffer);     
   memset(BUF,0,10);
   sprintf(BUF,"%02d%02d20%02d",RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year);
   strcat(EM_BUFFER,BUF);
   sprintf(BUF,"%02d%02d%02d",RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
   strcat(EM_BUFFER,BUF);
}
/*******************************************************************************
* Function Name  : Get_HLTH_PKT
* Description    : GENERATE HEALTH PACKET @ dedicated frequency or on demand 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_HLTH_PKT(void)
{
    char buffer[100];
    unsigned char ibv_level = 0;
    float ibv = 0.0,used_memory = 0.0;
    memset(buffer,0,10);
    memset(HP_BUFFER,0,100);

    char *Ptr;
    Ptr = NULL;

    if(Protocol == BSNL_MH)
    {
      sprintf(HP_BUFFER,"$HLP,%s,%s,%s,",VENDOR_ID,SW_Ver,GPRSData.IMEI_No);
    }
    else if(Protocol == NIC_PC || Protocol == NIC_OD)
    {
      sprintf(HP_BUFFER,"$HEL,%s,%s,%s,",VENDOR_ID,SW_Ver,GPRSData.IMEI_No);
    }
    else 
    {
      sprintf(HP_BUFFER,"$,%s,%s,%s,%s,",HEADER_ID,VENDOR_ID,SW_Ver,GPRSData.IMEI_No);
    }                                   
    
    /*-----------------------------------
    INTERNAL BATTERY % CALCULATION 
    BATTERY 100% = 4.1 VOLT
    BATTERY 0% = 3.5 VOLT
    ----------------------------------------*/
    ibv = Vehicle.INT_VOLT;
    if(ibv > 4.1)ibv = 4.1;
    else if(ibv < 3.5)ibv = 3.5;
    
    ibv_level = (unsigned char)(((ibv-3.5)*100)/0.6);
    memset(buffer,0,10);
    sprintf(buffer,"%hu,%hu,",ibv_level,IBL);
    strcat(HP_BUFFER,buffer);
    
   if(Protocol == NIC_PC || Protocol == NIC_OD || Protocol == BSNL_MH)
   {
      used_memory = (TCP_BackCnt[MAIN]+TCP_BackCnt[TER])/600.0;
      memset(buffer,0,10);
      sprintf(buffer,"%d",(char)used_memory);
      strcat(HP_BUFFER,buffer);
   }
   else 
   {
      memset(buffer,0,10);
      used_memory = (TCP_BackCnt[MAIN]+TCP_BackCnt[TER])/600.0;
      if(used_memory > 99.0)ftoa(used_memory,buffer,3,1);
      else ftoa(used_memory,buffer,2,1);
      strcat(HP_BUFFER,buffer);
   }

    memset(buffer,0,10);
    sprintf(buffer,",%hu,%hu,",Active_Time,StandBy_Time);
    strcat(HP_BUFFER,buffer);
    
     /*------- DIGITAL INPUT -------------------------------*/  
     /* BITWISE DIGITAL INPUT (MOMENTARY)STATUS */                                 // DIGITAL INPUT STATUS 1- ON WHILE 0-OFF  
      
    if((Vehicle.DIN & 0x01) == 1)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
       
    if((Vehicle.DIN & 0x02) == 2)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
       
    if((Vehicle.DIN & 0x04) == 4)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
   
    if((Vehicle.DIN & 0x08) == 8)strcat(HP_BUFFER,"1");                         // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat(HP_BUFFER,"0");                                                  // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
    
    if(Protocol != BSNL_MH)strcat(HP_BUFFER,",");   
    
  if(Protocol != NIC_PC && Protocol != NIC_OD) // Digital Output removed in NIC
  {
     /* BITWISE DIGITAL OUTPUT STATUS */
    if((Vehicle.DOUT & 0x01) == 0x01)strcat(HP_BUFFER,"1");                      // DIGITAL OUTPUT 1-ON        
    else strcat(HP_BUFFER,"0");                                                  // DIGITAL OUTPUT 1-OFF
     
    if((Vehicle.DOUT & 0x02) == 0x02)strcat(HP_BUFFER,"1,");                     // DIGITAL OIUTPUT 2-ON
    else strcat(HP_BUFFER,"0,");                                                 // DIGITAL OUTPUT 2-OFF
  }
     
  if(Protocol == NIC_PC || Protocol == NIC_OD || Protocol == BSNL_MH)
  {
    memset(buffer,0,10);
    sprintf(buffer,"%02d",(char)Vehicle.AIN1);
    strcat(HP_BUFFER,buffer);
  }
  else
  {
    memset(buffer,0,10); 
    ftoa(Vehicle.AIN1,buffer,2,1);
    strcat(HP_BUFFER,buffer);
    strcat(HP_BUFFER,",");
    
    memset(buffer,0,10); 
    ftoa(Vehicle.AIN2,buffer,2,1);
    strcat(HP_BUFFER,buffer);
    
  }
    
  if(Protocol == NIC_PC || Protocol == NIC_OD || Protocol == BSNL_MH)
  { 
     strcat(HP_BUFFER,"*");
  }
  else
  {
     strcat(HP_BUFFER,",*");  
  }
    
  if(Protocol != BSNL_PC && Protocol != HP_PC)
  {
    Ptr = &RAM_BACKUP[MAIN][RAM_BakCnt[MAIN]][0];                                 // TAKE ADDRESS OF LOCAL BACK UP REGISTER
    memcpy(Ptr,HP_BUFFER,256); if(RAM_BakCnt[MAIN] < 19)RAM_BakCnt[MAIN]++;                          // INCREMENT REGISTER 
  }
  
  
   // GET SECONDARY PACKET
    memset(HP_BUFFER,0,sizeof(HP_BUFFER));
    sprintf(HP_BUFFER,"$,HP,%s,%s,%s,%s%s%hu,%hu,%d,%d,%d,",VENDOR_ID,SW_Ver,GPRSData.IMEI_No,IRNSSData.DATE,IRNSSData.TIME,ibv_level,IBL,(char)used_memory,Active_Time,StandBy_Time);
    
    if((Vehicle.DIN & 0x01) == 1)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
       
    if((Vehicle.DIN & 0x02) == 2)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
       
    if((Vehicle.DIN & 0x04) == 4)strcat(HP_BUFFER,"1");                           // DIGITAL INPUT CONNECTED TO GROUND 
    else strcat(HP_BUFFER,"0");                                                   // DIGITAL INPUT CONNECTED TO PWR OR FLOATING
   
    if((Vehicle.DIN & 0x08) == 8)strcat(HP_BUFFER,"1,");                         // DIGITAL INPUT CONNECTED TO GROUND       
    else strcat(HP_BUFFER,"0,");     
    
    if((Vehicle.DOUT & 0x01) == 0x01)strcat(HP_BUFFER,"1");                      // DIGITAL OUTPUT 1-ON        
    else strcat(HP_BUFFER,"0");                                                  // DIGITAL OUTPUT 1-OFF
       
    if((Vehicle.DOUT & 0x02) == 0x02)strcat(HP_BUFFER,"1,");                     // DIGITAL OIUTPUT 2-ON
    else strcat(HP_BUFFER,"0,");                                                 // DIGITAL OUTPUT 2-OFF
    
    sprintf(buffer,"%04.1f,%04.1f,%d,%d,%s,%d.%1d*",Vehicle.AIN1,Vehicle.AIN2,Flag_SPIOK, Flag_MEMSOK,GPRSData.ICCID,KM,MTR/100);
    strcat((char*)HP_BUFFER,buffer);

    Ptr = &RAM_BACKUP[TER][RAM_BakCnt[TER]][0];                                 // TAKE ADDRESS OF LOCAL BACK UP REGISTER
    memcpy(Ptr,HP_BUFFER,256); if(RAM_BakCnt[TER] < 19)RAM_BakCnt[TER]++;                          // INCREMENT REGISTER 
 
    if(Protocol == BSNL_PC || Protocol == HP_PC)
    {
      Ptr = &RAM_BACKUP[MAIN][RAM_BakCnt[MAIN]][0];                                 // TAKE ADDRESS OF LOCAL BACK UP REGISTER
      memcpy(Ptr,HP_BUFFER,256); if(RAM_BakCnt[MAIN] < 19)RAM_BakCnt[MAIN]++;  
    }// INCREMENT REGISTER 
    
    
  if(Flag_MAINServer || Flag_TERServer)
  {
    Flag_HitServ = SET;
    Flag_HealthCheck = SET; 
    
    Flag_MAIN_PKT = SET; 
    Flag_TER_PKT = SET; 
    ServerHit_Time = 2;  
    TER_ServerHit_Time = TER_HitTime + 3;
  }
  
  
}
/*******************************************************************************
* Function Name  : Get_EMRGY_PKT
* Description    : GENERATE EMERGENCY PACKET @ EMERGENCY SWITCH IS PRESSED 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_EMRGY_PKT(void)
{
 char BUF[20];
 char buffer[100];
 char EMR_PKT[256];
 float HDOP = 0.0;
 unsigned int Distance = 0;
 char *Ptr;
 uint32_t FlashAddress;
 
 
 FlashAddress = 0;
 Ptr = NULL;  
 memset(BUF,0,10);
   
 memset(buffer,0,100);
 if(Flag_Emrgcy == RESET)
 {
     ED_Timer = 0;
     Flag_EM_LED = RESET;
     RAM_BakCnt[EMR] = 0; 
     EMRGY_BKP_No = 0;
     memset(RAM_BACKUP[EMR],0,5120);
     FlashAddress = SPIFLASH_CONFIG;
     sprintf(buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
     if(Flag_SPIOK == SET)
      {
        SPI_FLASH_SectorErase(FlashAddress);
        SPI_FLASH_PageWrite(buffer,FlashAddress,strlen(buffer));
      }
   }

   memset(EMR_PKT,0,256);
 
   Ptr = &EMR_PKT[0]; 
   if(Protocol == NIC_PC || Protocol == NIC_OD || Protocol == BSNL_MH)sprintf(Ptr,"$EPB,");
   else sprintf(Ptr,"$,EPB,");                                                       // EMERGENCY HEADER                                                    
  
   if(Flag_Emrgcy)strcat(Ptr,"EMR,");                                           // EMERGENCY MESSAGE
   else strcat(Ptr,"SEM,");                                                     // STOP MESSAGE 
  
   strcat(Ptr,GPRSData.IMEI_No);                                                // IMEI NUMBER 
   strcat(Ptr,",NM,");                                                          // NORMAL PACKET 
  
   memset(BUF,0,20);
   if(Flag_IRNSSFix){ memcpy(BUF,IRNSSData.DATE,8); strcat(BUF,IRNSSData.TIME); }
   else                                                                          // IF GPS DATA NOT READY SET TIME FROM RTC
   {
     sprintf(BUF,"%02d%02d20%02d%02d%02d%02d,",RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year,RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
   }  
   strcat(Ptr,BUF);
   
  
   if(Flag_IRNSSFix)strcat(Ptr,"A,");
   else strcat(Ptr,"V,");
   
  if(Flag_IRNSSFix)                                                              // CHECK GPS GNSS STATUS 
  {
     HDOP = atof(IRNSSData.HDOP);
     if(HDOP < 10.0)
     {
        strcat(Ptr,IRNSSData.LAT);                                    // ADD LAT  IN SERVER SENDING DATA 
        strcat(Ptr,IRNSSData.LON);
        
        strcat(Ptr,IRNSSData.Altitude);
        strcat(Ptr,",");
        
        strcat(Ptr,IRNSSData.SPEED);                        // ADD SPEED IN SERVER SENDING DATA (VEHICE IGINITION ON)
        
        if(Prev_lat == 0.0)Distance = 0;
        else Distance = Calc_Distance(Prev_lat,Prev_lon,Geo_Lat,Geo_Long);
        Prev_lat = Geo_Lat; Prev_lon = Geo_Long;
        
        memset(buffer,0,100);
        if(Protocol == BSNL_MH)
          sprintf(buffer,"G,%u.%u,",Distance/1000,Distance%1000);
        else
          sprintf(buffer,"%u.%u,G,",Distance/1000,Distance%1000);
        strcat(Ptr,buffer);
      }
      else goto CELL_DATA;                                                       // GPRS FIX
  }  
  else
  {
  CELL_DATA:  
    /* CELL DATA AS FOLLOWS WHEN GPS & GNSS FAIL */
     
    if(IRNSSData.LAT[0] != 0)
    {
       strcat(Ptr,IRNSSData.LAT);
       strcat(Ptr,IRNSSData.LON);                                       // ADD LONG CO-ORDINATE
    }
    else strcat(Ptr,"00.000000,N,000.000000,E,");  
    if(Protocol == BSNL_MH)
      strcat(Ptr,"0.0,0.0,N,N.A,");                                               // ADD ALTITUDE, SPEED     
    else 
      strcat(Ptr,"0.0,0.0,N.A,N,");                                               // ADD ALTITUDE, SPEED     
  }
  
  memset(buffer,0,100);
  if(Protocol == NIC_PC || Protocol == NIC_OD)sprintf(buffer,"%s,+91-%sXX,*",Vehicle.VRN,&Vehicle.Master_No[0][0]);
  else if(Protocol == BSNL_MH)sprintf(buffer,"%s,%sXX,*",Vehicle.VRN,&Vehicle.Master_No[0][0]);
  else sprintf(buffer,"%s,%s,XX,*",Vehicle.VRN,&Vehicle.Master_No[0][0]);
  strcat(Ptr,buffer);
  
  if(Flag_EMG_UK_SMS == RESET)
  {
     if(RAM_BakCnt[EMR] > 19)Take_EMRGY_BKP();
     memcpy(&RAM_BACKUP[EMR][RAM_BakCnt[EMR]][0],Ptr,256);
     RAM_BakCnt[EMR]++;
    
     if(Flag_EMRServer){Flag_Hit_EMRServ = SET;}
   }
  
}

void Take_EMRGY_BKP(void)
{
  char loc_buffer[50];
  char *Ptr;
  unsigned char counter = 0;
  uint32_t FlashAddress;
  
  FlashAddress = 0;
  Ptr = NULL;
    
  memset(loc_buffer,0,50);
   while(RAM_BakCnt[EMR])
   {
     RAM_BakCnt[EMR]--;
     Ptr = RAM_BACKUP[EMR][counter];
     if(Ptr = strstr(Ptr,",NM,"))
     {
      Ptr += 1;
      *Ptr = 'S'; *(Ptr + 1) = 'P';
      if(Ptr = strstr(Ptr,",*,"))
      {
        Ptr += 3;
        *Ptr = 0;
         Ptr++;
        *Ptr = 0;
      }
      
      FlashAddress = SPIFLASH_BAKUP3 + (TCP_BackCnt[EMR] * 0x100);              // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
      TCP_BackCnt[EMR]++;
      if(TCP_BackCnt[EMR] > 1023) TCP_BackCnt[EMR] = 1024;                      // LIMIT TO MAX 1024 PACKETS 
      
      if(Flag_SPIOK == SET)                                                     // CHECK SERIAL FLASH STATUS
        {
         if(FlashAddress % 4096 == 0)SPI_FLASH_SectorErase(FlashAddress);       // IF ADDRESS IS ENDING ADDRESS ERASE ALL PAGES
         SPI_FLASH_PageWrite(&RAM_BACKUP[EMR][counter][0],FlashAddress,256);    // WRITE BACK UP ON FLASH MEMORY
         counter++;
         
         if(RAM_BakCnt[EMR] == 0)
          {
           FlashAddress = SPIFLASH_CONFIG;                                      // SET FLASH ADDRESS TO 0
           memset(loc_buffer,0,50);
           sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);                
           
           SPI_FLASH_SectorErase(FlashAddress);                                     // ERASE SECTION OF FLASH TO WRITE NEW DATA
           SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));// WRITE GPRS SEND BUFFER DATA TO FLASH PAGES 

           memset(&RAM_BACKUP[EMR][0][0],0,5120);
          }
        }
      }
   }
}


/*******************************************************************************
* Function Name : Get_Operator
* Description   : Get operator from modem
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Get_Operator(void)
{
  char *Ptr;
  uint16_t GP_Counter;
  
  Ptr = NULL;
  GP_Counter = 0;

   
    GPRS_Count = 0;
    memset(GPRS_Buffer,0,50);
    
    Modem_PutString("AT+QSPN");WAIT_MODEM_RESP(2);
   
  

  if(Flag_ModemOK == SET)
    {
      if(Ptr = strstr((char*)GPRS_Buffer,"PN:")) 
      {
         Ptr += 4;
         if(*Ptr == 0x22)
         {
           Ptr += 1;
         for(GP_Counter = 0;GP_Counter < 10; GP_Counter ++,Ptr++)
            {
              if (*Ptr == 0x22){GPRSData.Operator[GP_Counter] = 0; break;}
              else 
                {
                 GPRSData.Operator[GP_Counter] = *Ptr;
                 if(islower(GPRSData.Operator[GP_Counter]))GPRSData.Operator[GP_Counter] = toupper(GPRSData.Operator[GP_Counter]);
               };
            };
         }
       };     
    };  
  
}

/*******************************************************************************
* Function Name  : OPERATOR SWAPPING 
* Description    : SWAP THE SURVICE PROVIDER PROFILE INTO SIM 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Swap_OPS(void)
{
   char *Ptr;
   Get_Operator();
   Get_Operator();
   signed char TimeOut;
   
   
   memset(GPRS_Buffer,0,100);
   GPRS_Buffer[498] = 0;
   GPRS_Buffer[499] = 0;
   GPRS_Count = 0; 
   
   if(strstr (GPRSData.Operator,"BSNL") || strstr (GPRSData.Operator,"N.A"))
   {
     // Auto Switch to 4G2G Network for AIRTEL / VODAFONE
     Modem_PutString("AT+QCFG=\"nwscanmode\",0,1");WAIT_MODEM_RESP(2); 
   }
   else
   {
     // Manual Switch to 2G Network for BSNL
     Modem_PutString("AT+QCFG=\"nwscanmode\",1,1");WAIT_MODEM_RESP(2); 
   }
   // RESET Menu
   Modem_PutString("AT+CFUN=1");WAIT_MODEM_RESP(2); mSec_Delay(3000);
   // Enable STK Tool set
   Modem_PutString("AT+QSTK=1,0,60");WAIT_MODEM_RESP(2); mSec_Delay(3000);
   // Enable STK Tool set
   Modem_PutString("AT+QURCCFG=\"urcport\"");WAIT_MODEM_RESP(2); mSec_Delay(3000);
   // Setup Menu
   Modem_PutString("AT+QSTKGI=37");WAIT_MODEM_RESP(2); mSec_Delay(3000);
   // Select Sub Menu
   Modem_PutString("AT+QSTKRSP=37,1");WAIT_MODEM_RESP(2); 
   Ptr = 0x00; TimeOut = 3;
   while(Ptr == 0x00)
   {
     Ptr = strstr(GPRS_Buffer,"URC: 253");
     if(Ptr)break;
     mSec_Delay(1000);
     IWDG_ReloadCounter();
     TimeOut--;
     if(TimeOut < 0) break;
   }
   // Select menu item
   Modem_PutString("AT+QSTKRSP=253,0,1");WAIT_MODEM_RESP(2); 
   Ptr = 0x00; TimeOut = 3;
   while(Ptr == 0x00)
   {
     Ptr = strstr(GPRS_Buffer,"URC: 36");
     if(Ptr)break;
     mSec_Delay(1000);
     IWDG_ReloadCounter();
     TimeOut--;
     if(TimeOut < 0) break;
   }
   // Select item  
   Modem_PutString("AT+QSTKGI=36");WAIT_MODEM_RESP(2);mSec_Delay(3000);
 
   //Modem_PutString("AT+QSTKRSP=36,0,1");WAIT_MODEM_RESP(2);mSec_Delay(3000);
   
   if(Flag_ModemOK)
   {
     if(SP_REQ == 0)
     {
      SwapCnt++;
      // Profile Selection
      if(strstr (GPRSData.Operator,"BSNL") || strstr (GPRSData.Operator,"N.A"))
      {
      #if defined(TAISYS_VB)
         Modem_PutString("AT+QSTKRSP=36,0,3"); // To Vodafone
      #else
         Modem_PutString("AT+QSTKRSP=36,0,1"); // To Airtel
      #endif
       }
       else 
       {
         Modem_PutString("AT+QSTKRSP=36,0,2");  // to BSNL
       }
     }
     else 
     {
        sprintf(GPRS_SendBuffer,"AT+QSTKRSP=36,0,%d",SP_REQ); // to requested Profile
        Modem_PutString(GPRS_SendBuffer);
     }
     
     WAIT_MODEM_RESP(20);
     while(Ptr == 0x00 || TimeOut > 0)
     {
       Ptr = strstr(GPRS_Buffer,"B DONE");
       if(Ptr)break;
       mSec_Delay(1000);
       if(TimeOut)TimeOut--;
       else break;
     }
     SP_REQ = 0; 
     Modem_PutString("AT+CFUN=1,1");WAIT_MODEM_RESP(10);
     EC200_ONTime = 15;
   }
}

/*************************************/
// note change in simswap os
// change in set apn
// change in interrput
void Send_ACTV_HCHK_Resp(void)
{
  char buffer[100];
  char date[10];
  char time[10];
  memset(date,0,10);
  memset(time,0,10);
  memset(RESP_BUFFER,0,256);
  memset(buffer,0,100);
  sprintf(buffer,"%s,%s,%s,%s,",Random_Code,VENDOR_ID,SW_Ver,GPRSData.IMEI_No);
  
  if(Protocol == BSNL_MH)
  {
      if(Flag_ACTV)
      {
        Flag_ACTV = RESET;sprintf(RESP_BUFFER,"$ACTVR,");
        strcat(buffer,"23,");
      }
      else if(Flag_HCHK)
      {
        Flag_HCHK = RESET;sprintf(RESP_BUFFER,"$HCHKR,");
        strcat(buffer,"24,");
      }

  }
  else
  {
    if(Flag_ACTV)
    {
      Flag_ACTV = RESET;sprintf(RESP_BUFFER,"ACTVR,");
      strcat(buffer,"23,");
    }
    else if(Flag_HCHK)
    {
      Flag_HCHK = RESET;sprintf(RESP_BUFFER,"HCHKR,");
      strcat(buffer,"24,");
    }
  }
 
  
  strcat(RESP_BUFFER,buffer);memset(buffer,0,100);
  if(Flag_IRNSSFix)
  {
   memset(date,0,10);memcpy(date,IRNSSData.DATE,8);
   memset(time,0,10);memcpy(time,IRNSSData.TIME,6);
   sprintf(buffer,"%s%s1,%s %s,%s%s%d,%hu,%hu,%s,",IRNSSData.LAT,IRNSSData.LON,date,time,IRNSSData.DIRECTION,IRNSSData.SPEED,GPRSData.Signal_Strength,GPRSData.MCC,GPRSData.MNC,GPRSData.LAC);
  }
  else
  {
    memset(time,0,10);sprintf(time,"%02d%02d%02d",RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
   sprintf(buffer,"00.000000,N,00.000000,E,0,%s %s,0.0,0.0,%d,%hu,%hu,%s,",date,time,GPRSData.Signal_Strength,GPRSData.MCC,GPRSData.MNC,GPRSData.LAC);     
  }
  strcat(RESP_BUFFER,buffer);memset(buffer,0,100);
  if(Vehicle.Battery[0] == 'C')
  {
    strcat(RESP_BUFFER,"1,");
  }
  else strcat(RESP_BUFFER,"0,");
   
  if(Vehicle.Ignition[0] == 'I') 
  {
    strcat(RESP_BUFFER,"1,");
  }
  else strcat(RESP_BUFFER,"0,"); 
    
  ftoa(Vehicle.EXT_VOLT,buffer,2,1);
  strcat(RESP_BUFFER,buffer);
  strcat(RESP_BUFFER,",");
  memset(buffer,0,100);
  sprintf(buffer,"%06u,",Frame_Num);
  strcat(RESP_BUFFER,buffer);
  memset(buffer,0,100);
  if(Vehicle.Ignition[0] == 'I') 
  {
    if(Protocol == BSNL_MH)
      strcat(RESP_BUFFER,"NR*");
    if(Protocol == BSNL_PC || Protocol == HP_PC)
      strcat(RESP_BUFFER,"NR");
    else
      strcat(RESP_BUFFER,"01");
  }
  else 
    strcat(RESP_BUFFER,"00"); 
  
}


void Refresh_Data(void)
{
  char loc_buffer[50];
  uint32_t FlashAddress = 0;
  
  memset(loc_buffer,0,50);
  TCP_BackCnt[MAIN] = 0;
  TCP_BackCnt[TER] = 0;
  EMRGY_BKP_No = 0;

  RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
          
  FlashAddress = SPIFLASH_CONFIG;
  memset(loc_buffer,0,50);
  sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
   
  if(Flag_SPIOK == SET)
    {
     SPI_FLASH_SectorErase(FlashAddress);
     SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
    }
   
   RAM_BakCnt[MAIN]    = 0;
   RAM_BakCnt[TER]     = 0;
   RAM_BakCnt[EMR]     = 0;
   
   if(Flag_TCPConnect[MAIN] == SET){Flag_TCPConnect[MAIN] = RESET; Modem_PutString("AT+QICLOSE=0"); WAIT_MODEM_RESP(2);}
   if(Flag_TCPConnect[TER]  == SET){Flag_TCPConnect[TER]  = RESET; Modem_PutString("AT+QICLOSE=1"); WAIT_MODEM_RESP(2);}
   if(Flag_TCPConnect[EMR]  == SET){Flag_TCPConnect[EMR]  = RESET; Modem_PutString("AT+QICLOSE=2"); WAIT_MODEM_RESP(2);}
   
   Frame_Num = 0;
   Frame_Num_TER = 0;  
   Save_EMR(); //write to register in function
   RTC_WriteBackupRegister(RTC_BKP_DR1,((Frame_Num_TER << 8) | (Flag_PowerOff))); 
   
   memset(&RAM_BACKUP[MAIN][0][0],0,5120);
   memset(&RAM_BACKUP[TER][0][0],0,5120);
   memset(&RAM_BACKUP[TER][0][0],0,5120);    
}
/*******************************************************************************
* Function Name : SET_RTC
* Description   : SET RTC CLOCK DATE & TIME   
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void SET_RTC(void)
{  
    int HH,MM,SS,DD,MH,YY;
   
    if(Flag_IRNSSFix)
    {
       DD = (IRNSSData.DATE[0] - '0')*10 + (IRNSSData.DATE[1] - '0');      // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
       MH = (IRNSSData.DATE[2] - '0')*10 + (IRNSSData.DATE[3] - '0');      // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
       YY = (IRNSSData.DATE[6] - '0')*10 + (IRNSSData.DATE[7] - '0');      // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2
          
       HH = (IRNSSData.TIME[0] - '0')*10 + (IRNSSData.TIME[1] - '0');      // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
       MM = (IRNSSData.TIME[2] - '0')*10 + (IRNSSData.TIME[3] - '0');      // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
       SS = (IRNSSData.TIME[4] - '0')*10 + (IRNSSData.TIME[5] - '0');      // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2
      
    }
    else if(GPRSData.Cell_Time[0] == 0 || GPRSData.Cell_Date[0] == 0)return;
    else
    {
      DD = (GPRSData.Cell_Date[0] - '0')*10 + (GPRSData.Cell_Date[1] - '0');      // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
      MH = (GPRSData.Cell_Date[2] - '0')*10 + (GPRSData.Cell_Date[3] - '0');      // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
      YY = (GPRSData.Cell_Date[6] - '0')*10 + (GPRSData.Cell_Date[7] - '0');      // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2
          
      HH = (GPRSData.Cell_Time[0] - '0')*10 + (GPRSData.Cell_Time[1] - '0');      // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
      MM = (GPRSData.Cell_Time[2] - '0')*10 + (GPRSData.Cell_Time[3] - '0');      // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
      SS = (GPRSData.Cell_Time[4] - '0')*10 + (GPRSData.Cell_Time[5] - '0');      // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2
    
      HH -= 5;
      MM -= 30;
      if(MM < 0) { HH--; MM += 59;}
      if(HH < 0){ DD--;HH += 23;}
    }
    

        
    RTC_Time.RTC_H12 = RTC_HourFormat_24;
    RTC_Time.RTC_Hours = HH;
    RTC_Time.RTC_Minutes = MM;
    RTC_Time.RTC_Seconds = SS;
    RTC_SetTime(RTC_Format_BIN, &RTC_Time); 
    
    RTC_Date.RTC_WeekDay = 0;
    RTC_Date.RTC_Date    = DD;
    RTC_Date.RTC_Month   = MH;
    RTC_Date.RTC_Year    = YY;

    
    RTC_SetDate(RTC_Format_BIN, &RTC_Date);
    
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
      
}
/*******************************************************************************
* Function Name : Save_EMR
* Description   : Save the emergency status on BKP Reg 3 with frame NUM Primary server   
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void Save_EMR(void)
{
   uint32_t var;
   var = 0;
   
   var = (Frame_Num << 8) | Flag_Emrgcy;
   RTC_WriteBackupRegister(RTC_BKP_DR3,var);
}
