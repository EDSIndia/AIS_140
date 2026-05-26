/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : GPRS.h					       	       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This File Defines variables & function used in GPRS          * 
* Revision	: Rev0 						      	       *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPRS_H
#define __GPRS_H


/* FUNCTIONS -----------------------------------------------------------------*/
void Serial_SendData_GPRS(char);                                         // SEND DATA ON SERIAL (USART)=> SINGLE DATA
void Serial_PutData_GPRS(char *, char);                                  // SEND STRING OF GIVEN LENGTH 
void Serial_PutString_GPRS(char *);                                      // SEND STRING UNTILL STRING END 
void Get_Backup(uint8_t );
FlagStatus Verify_UploadData(char *);
void Ack_Upload(uint8_t);
//static void Send_Config(uint8_t);
void Get_NBRCell_CSQ(void);
void Waiting_Reply(void);
void Send_ACTV_HCHK_Resp(void);
void Get_EMGY_SMS(void);
void delay(uint8_t nTime);
void Refresh_Data(void);

void Serial_SendData_GPRS(char);                                         // SEND DATA ON SERIAL (USART)=> SINGLE DATA
void Serial_PutData_GPRS(char *, char);                                  // SEND STRING OF GIVEN LENGTH 
void Serial_PutString_GPRS(char *);                                      // SEND STRING UNTILL STRING END 

void Modem_PutString(char *);
void Gen_PKT(void);
void WAIT_MODEM_RESP(uint8_t);
void Take_Backup(uint8_t);
void Take_EMRGY_BKP(void);
void Get_EMRGY_PKT(void);
void Get_Operator(void);
void Write_Oddo(void);
void MODULE_EC200_RST(void);
void Set_TCPData(uint8_t);
void Get_RAMBackup(uint8_t);
void SET_RTC(void);
void Save_EMR(void);

void Get_Backup(uint8_t );
FlagStatus Verify_UploadData(char *);
void Ack_Upload(uint8_t);
void Get_NBRCell_CSQ(void);
void Waiting_Reply(void);
void Send_ACTV_HCHK_Resp(void);
void Get_EMGY_SMS(void);
void delay(uint8_t nTime);



void SendResp(void);
void INIT_USART_GPRS(void);                                                     // INITIALIZE GPRS USART 
void INIT_GPRS_Modem(void);
void Hit_TCPServer(uint8_t);
void Read_SMS(void);
void Send_SMS(uint8_t,FlagStatus);
void Get_CellLocation(void);
void Check_Signal_Nw_GPRS(void);
void Check_Config(void);
void GetConfig_Server(void);
void Get_NBRData(void);
void Swap_OPS(void);
void Get_HLTH_PKT(void);
void TAKE_FUL_BKP(void);
void GPRS_Managment(void);

#if defined(DEBUG_ENB)
void GET_SYS_TIME(void);
#endif


/* DEFINATIONS----------------------------------------------------------------*/
// GPRS BAUD RATE
#define BAUD_RATE_GPRS          115200                                          // GPRS BAUD RATE

// USART 1 used for GPRS/GSM -  GPRS Modem Auto Buad Rate
#define GPRS_USART              USART1     
#define GPRS_Tx                 GPIO_Pin_9                                      // USART1(GPRS) TRANSMITTER PIN
#define GPRS_Rx                 GPIO_Pin_10                                    // USART1(GPRS) RECEIVER PIN
#define PORT_GPRSUSART          GPIOA

// GPRS PIN & PORT 
#define PORT_GPRS               GPIOC
#define PIN_GPRS                GPIO_Pin_7                                     // TO ENABLE/DISABLE GPRS POWER SUPPLY

// GPRS ON/OFF
#define GPRS_ON                 PORT_GPRS->BRR   = PIN_GPRS                     // INVERSE LOGIC TRANSISTOR
#define GPRS_OFF                PORT_GPRS->BSRR  = PIN_GPRS                     // INVERSE LOGIC TRANSISTOR

//Defination for sending SMS 
#define OP1_STARTED             0
#define OP1_STOPPED             1
#define OP2_STARTED             2
#define OP2_STOPPED             3
#define CONF                    5
#define ACKN                    6
#define DEBUG                   7
#define UNATHU                  8
#define UPGRADE                 9
#define DFU                     10
#define LOCATION                11
#define GPSDATA                 12
#define GPRSDATA                14
#define INFO                    15
#define EMG                     16
#define DFU_DETAIL              17
#define DFU_DNF                 18
#define HEALTH                  19
#define HLTH_ACTV               20
#define GEO_FENCE               21
#define INVALID                 22

#define GETSTS                  31
#define GETVID                  32
#define MOBNUM1                 33
#define MOBNUM2                 34
#define MOBNUM3                 35
#define MOBNUM4                 36
#define MOBNUM5                 37
#define IOSTATUS                38
#define HW_VER                  39
#define SMSPKT                  40
#define CONFIG                  41
#define GEOFENCE                42
#define PARAMS                  43

#define EMRN                    44
#define EMRF                    45
#define OVS                     46
#define TLT                     47
#define GEN                     48
#define GEX                     49
#define MBR                     60
#define RNC                     61
#define DT                      62

#define WARNING                 50
#define FLASH_ERR               51
#define NA                      52
#define ATC                     53

#define UI                      54
#define TD                      55
#define IP1                     56
#define IP2                     57
#define IP3                     58

#define SYN_ERR                 99

// SERVER 
#define MAIN                    0
#define TER                     1
#define EMR                     2



/* VARIABLES -----------------------------------------------------------------*/

typedef struct									// DATA STRUCTURE FOR GPRS 
  {
    char IMEI_No[16];                                                           // IMEI NUMBER        
    char SIM_Present[2];                                                        // SIM PRESENT 
    char ICCID[25];
    char Operator[11];								// OPERATOR 
    char APN_Name[33];                                                          // ACCESS POINT NAME 
    char SET_APN[33];                                                           // SET APN BY USER 
    char Cell_ID[10];                                                            // CELL TOWER ID                    
    char Cell_Lat[15];                                                          // CELL TOWER LATITUDE 
    char Cell_Long[15];                                                         // CELL TOWER LONGITUDE 
    char Cell_Time[8];                                                          // CELL TOWER TIME 
    char Cell_Date[10];                                                         // CELL TOWER DATE 
    unsigned char Signal_Strength;							// SIGNAL STRENGTH 
    char Network_Status;							// NETWORK STATUS 
    FlagStatus Home_Network;							// HOME NETWORK 
    FlagStatus Roaming_Network;							// ROAMING NETWORK 
    char Mobile_No[16];								// SENDER MOBILE NUMBER
    char LAC[5];                                                                // LOCATION AREA CODE
    unsigned short MCC;                                                                    // MOBILE COUNTRY COCDE 
    unsigned short MNC;                                                                    // MOBILE NETWORK CODE
  }GPRS_InitTypeDef;

extern GPRS_InitTypeDef    GPRSData;

typedef struct                                                                  // DATA STRUCTURE FOR DFU 
{ 
  char IP[4];                                                                   // DFU/HTTP IP ADDRESS 
  char PATH[50];                                                                // DFU/HTTP Path
  char FILENAME[15];                                                            // DFU/HTTP File Name
  char URL[100];                                                                // DFU/HTTP URL 
}HTTP_InitTypeDef;

extern HTTP_InitTypeDef DFU_HTTP;

typedef struct                                                                  // DATA STRUCTURE FOR DFU 
{ 
  char CELLID[6];                                                               // DFU/FTP USERNAME 
  char LAC[5];                                                                  // DFU/FTP PASSWORD 
  int SIG;
 }NBR_InitTypeDef;

extern NBR_InitTypeDef NBRCell[4];

#endif