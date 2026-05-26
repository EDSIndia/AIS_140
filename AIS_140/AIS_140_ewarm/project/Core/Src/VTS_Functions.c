/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : VTS_Functions.c					       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file contains all misc functions of VTS                 *
* Revision	: Rev0    						       *
********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Main.h"
    
extern void TAKE_FUL_BKP(void);
extern void Send_SMS(uint8_t,FlagStatus);
extern void CONFIG_OB(void);


//static uint32_t Extract_Value(char * TempPtr, uint8_t Place);

/*******************************************************************************
* Function Name  : Serial_PutString
* Description    : Print a string on the GPRS Modem
* Input          : A String till null character
* Output         : None
* Return         : None
*******************************************************************************/
#if defined (RS485_ENABLE)
void Serial_PutString_485(char *s)                                             // THIS FUNCTION SEND STRING'S CHARACTER TILL NULL CHARACTER
{ 
  GPIOA->BSRR = GPIO_Pin_12;
  while (*s != '\0')                                                            // PUT STRING OVER USART TILL NULL CHARACTER 
  {
    while (USART_GetFlagStatus(COM2_USART, USART_FLAG_TXE) == RESET);               // WAIT UNTILL LAST TRANSMISSION COMPLETES
    COM2_USART->TDR = (*s & (uint16_t)0x01FF);                                  // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
    s++; 
    mSec_Delay(2);// INCREMENT POINTER
  };
  GPIOA->BRR = GPIO_Pin_12;
}
#endif


/*******************************************************************************
* Function Name  : Second_Timer
* Description    : Get The Configuration Values
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void Second_Timer(void)
{ 
   Flag_Second = RESET;
   GS_Time++;
   IWDG_ReloadCounter();                                                        // RELOAD IWDT
   
#if defined (RS485_ENABLE)
   // Testing RS485
   sprintf(GP_Buffer,"%02d:%02d:%02d\r\n",RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
   Serial_PutString_485(GP_Buffer);
#endif
   
   if(Flag_SetIMOB == SET && Current_Speed < 10){Flag_SetIMOB = RESET; OP1_START;}
   
   if(Flag_EMG_SMS){Flag_EMG_SMS = RESET; Send_SMS(EMG,SET);}                   // Send SMS
   if(Modem_NoResp > 10){Modem_NoResp = 0; Flag_MODULE_M66_RST = SET;  EC200_ONTime = 15; }      // No Response from Modem
   if(Flag_SWAP)                                                                // Swap SIM Operator
   {
     Flag_SWAP = RESET; Flag_CSQErr = RESET; 
     SIM_Err = 0; SIM_SWAPTimer = 0;
     if(Flag_SWAP_ENB == SET)Swap_OPS();                                  // Swap Next
   }                                                                      
     
   if(Flag_MODULE_M66_RST){Flag_MODULE_M66_RST = RESET;  EC200_ONTime = 15;}; // RESET MODEM
   // Inital Turn On Delay for Modem
   
   if(EC200_ONTime)EC200_ONTime--;
   if(EC200_ONTime == 14){  Flag_InitLED = RESET;  MODULE_EC200_RST();}
   if(EC200_ONTime == 10)GPRS_ON;
   if(EC200_ONTime > 0 && EC200_ONTime < 6)
   {
      if(EC200_ONTime == 5)Modem_PutString("AT"); 
      if(strstr((char*)GPRS_Buffer,"PB DONE") || EC200_ONTime == 1){Flag_ModemRdy = SET; EC200_ONTime = 0; INIT_GPRS_Modem();};
   }
         
   // GNSS Time Activity 
   if(GNSS_Time)GNSS_Time--;
   if(GNSS_Time <= 1 ){GNSS_Time = GNSS_INTERVAL; Get_Location();};        
   
   if(IRNSS_FailTime > 30 && IRNSS_FailTime < 40)
   {
      mSec_Delay(3000);
      RST_GNSS;
      mSec_Delay(5000);
      RUN_GNSS;
      mSec_Delay(2000);
   }
   
   if(IRNSS_FailTime > 400)
   {
      IRNSS_FailTime = 0;
      MODULE_L89(DISABLE);         
      mSec_Delay(2000); IWDG_ReloadCounter();
      mSec_Delay(2000); IWDG_ReloadCounter();
      MODULE_L89(ENABLE);
      Flag_IRNSSFix = RESET;
   }
  
   if((Flag_GetStatus == SET) || (GS_Time > 30))
   {
     Flag_GetStatus = RESET;
     GS_Time = 0;
     if(Vehicle.Ignition[0] == 'I') 
     {
         DS = 1;                                                            // Device Status = Stationary
         Avg_Speed = 0.0;
     }
     else if(Vehicle.Battery[0] == 'C') DS = 0;                              // Device Status = Power_On Status   // change on 17082021
     else DS = 3;                                                             // Device Status = Internal Battery 

     Avg_Speed = 0;
   }

   if(Protocol == HP_PC)
   {
     if(Flag_OverSpeed == SET){ OVS_Buzz_Timer++; if(OVS_Buzz_Timer > 600)OVS_Buzz_Timer = 601; }
     if(Flag_TILT == SET) { TLT_Buzz_Timer++; if(TLT_Buzz_Timer > 600)TLT_Buzz_Timer = 601; }
     else TLT_Buzz_Timer = 0;
       
     if(GEO_Buzz_Time){GEO_Buzz_Time--; if(GEO_Buzz_Time == 0)OP2_STOP;}
             
     if(Flag_Emrgcy == SET)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 199){if(ED_Timer)OP2_TGL; else OP2_STOP; Buzz_Tim = 0;};
     }
     else if(Flag_OverSpeed == SET)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 599){if(OVS_Buzz_Timer < 600)OP2_TGL; else OP2_STOP; Buzz_Tim = 0;};
     }
     else if(Flag_TILT == SET)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 999){ if(TLT_Buzz_Timer < 600)OP2_TGL;  else OP2_STOP;  Buzz_Tim = 0; };
     }
     else if(GEO_Buzz_Time)OP2_START;
     else OP2_STOP;
       
     if(Flag_SMS_EMRN == SET){SMS_ID = 1; Send_SMS(EMRN,SET); }
     if(Flag_SMS_EMRF == SET){SMS_ID = 2; Send_SMS(EMRF,SET); }
     if(Flag_SMS_OVS  == SET){SMS_ID = 3; Send_SMS(OVS,SET);  }
     if(Flag_SMS_TL   == SET){SMS_ID = 4; Send_SMS(TLT,SET);  }
     if(Flag_SMS_GE   == SET){SMS_ID = 5; Send_SMS(GEN,SET);  }
     if(Flag_SMS_GX   == SET){SMS_ID = 6; Send_SMS(GEX,SET);  }
     if(Flag_SMS_MBR  == SET){SMS_ID = 7; Send_SMS(MBR,SET);  }
     if(Flag_SMS_RNC  == SET){SMS_ID = 8; Send_SMS(RNC,SET);  }
     if(Flag_SMS_DT   == SET){SMS_ID = 9; Send_SMS(DT,SET);  }
   }
   else 
   {
      if(Flag_SMS_EMRN == SET){Flag_SMS_EMRN = RESET;}
      if(Flag_SMS_EMRF == SET){Flag_SMS_EMRF = RESET;}
      if(Flag_SMS_OVS  == SET){Flag_SMS_OVS  = RESET;}
      if(Flag_SMS_TL   == SET){Flag_SMS_TL   = RESET;}
      if(Flag_SMS_GE   == SET){Flag_SMS_GE   = RESET;}
      if(Flag_SMS_GX   == SET){Flag_SMS_GX   = RESET;}
      if(Flag_SMS_MBR   == SET){Flag_SMS_MBR   = RESET;}
      if(Flag_SMS_RNC   == SET){Flag_SMS_RNC   = RESET;}
      if(Flag_SMS_DT   == SET){Flag_SMS_DT   = RESET;}
    }
    
   

   // Over speed
   if(Current_Speed < Speed_Limit || Flag_IRNSSFix == RESET)
   {
     Flag_OverSpeed = RESET;
     OVS_Buzz_Timer = 0;
   }

   
   // Emergency Auto Disable Time
   if(ED_Timer > 0 && Flag_Emrgcy == SET && Flag_STOPMSG == SET)
   {
     ED_Timer--;
     if(ED_Timer == 0)
     {
       if(Protocol != HP_PC)
       {
         Flag_Emrgcy = RESET;
         Flag_EA = RESET;
         Flag_EA_OFF = SET; 
         
         Save_EMR();
         
         Flag_MAIN_PKT = SET;
         Flag_TER_PKT = SET;
         ServerHit_Time = 2; 
         TER_ServerHit_Time = TER_HitTime + 3;

         
         Flag_EM_LED = RESET; 
         Get_EMRGY_PKT(); 
         Flag_HitServ = SET;
        }
     }
   }
   
   
   // HEALTH PACKET INTERVAL 
   if(HLTH_TIMER > HLTH_FRQ)HLTH_TIMER = HLTH_FRQ;
   if(HLTH_TIMER)HLTH_TIMER--;
   else
   {
     if(HLTH_FRQ)
     {
      HLTH_TIMER = HLTH_FRQ-1;                                             // RELOAD SERVER HIT TIME 
      Get_HLTH_PKT();
     }
   };

   // Vehicle Ideal
   if(Vehicle.Ignition[0] == 'I')
   {
     if(Flag_IRNSSFix == SET && Current_Speed < 3)Idle_Time++;
     else Idle_Time = 0;
     
     if(Idle_Time == (IDL_Time * 60))
     {
//       if(Protocol == DIMTS_PC)
//       {
//         Flag_ID = SET;                         // VEHICLE IDLE DETECTED 
//         
//         Flag_MAIN_PKT = SET;
//         Flag_TER_PKT = SET;
//         ServerHit_Time = 2; 
//         TER_ServerHit_Time = TER_HitTime + 3;
//       }
     }                                
     else if(Idle_Time > (IDL_Time * 60))Idle_Time = (IDL_Time * 60) + 1;                           
   }
   
   // Emergency SMS Fallback
   if(SMS_FAllBack_Check > ES_Time * 2){SMS_FAllBack_Check = 0; Flag_EMG_SMS = SET;}
   
   // Cell Tower Check time
   if(Cell_CheckTime)Cell_CheckTime--;
        
}// End of Second Flag





/*******************************************************************************
* Function Name  : Get_Config
* Description    : Get The Configuration Values
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_Config(void)
{
  FlagStatus 
  Flash_Error;
  
  char temp[5];
  uint8_t 
  FP,
  FA;
  unsigned short FlashData;
  uint16_t GP_Counter;
  uint32_t FlashAddress;
  uint64_t loc;
  
  FlashAddress = 0;
  GP_Counter = 0;
  FP = 0;
  FA = 0;
  loc = 0;
  FlashData = 0;
  memset(temp,0,5);
  
  Flash_Error = RESET;
  
  /****** FLASH READING *****************************************************/
  FlashAddress = EndAddr - FLASH_PAGE_SIZE;

  FlashData = (*(vu16*) FlashAddress);FlashAddress += 2;                      // VERIFY FLASH DATA 
  if(FlashData == 0x00ED)                                                     // Signature Byte
  { 
     // skip for sos number 
    for(GP_Counter = 0;GP_Counter < 14; GP_Counter++)                           // Get SOS Mobile No, at start set Mobile No aslo same 
    {
       FlashAddress += 2;GP_Counter++;
    };
    
    /*----- MASTER NUMBER ---------------------------------------------------*/       
    for(loc = 0; loc < 5; loc++)
    {
      for(GP_Counter = 0;GP_Counter < 14; GP_Counter++)                            // At offset of 15 get Master Number 
      {
         FlashData = (*(vu16*) FlashAddress);
         Vehicle.Master_No[loc][GP_Counter+1] = FlashData >> 8;
         Vehicle.Master_No[loc][GP_Counter] = (char)FlashData;  
         FlashAddress += 2;GP_Counter++;
      };
    }
         
    /*----- PASSWORD STRING --------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                            // GET SAVED PASSWORD 
    {
       FlashData = (*(vu16*) FlashAddress);  
       PWD_IN[GP_Counter+1] = FlashData >> 8;PWD_IN[GP_Counter] = (char)FlashData; 
       FlashAddress += 2;GP_Counter++;
    };
        
    FlashAddress += 36;                                                         // Next Blok start at 50 leaving some space for future
    /*----- GET TCP SERVER IP ADDRESS & TCP PORT -----------------------------*/
    FlashData = (*(vu16*) FlashAddress);
    //TCP_IP[0][0] = (char)FlashData; TCP_IP[0][1] = FlashData >> 8; 
    FlashAddress += 2;       
  
    FlashData = (*(vu16*) FlashAddress);  
    //TCP_IP[0][2] = (char)FlashData; TCP_IP[0][3] = FlashData >> 8; 
    FlashAddress += 2;                             
    
    SRVR_Port[MAIN] = 0x0000FFFF & (*(vu16*) FlashAddress);            FlashAddress += 2;          
    
    // Main IP Address - will be sec IP for dual Server
    FlashData = (*(vu16*) FlashAddress);
    //TCP_IP[1][0] = (char)FlashData; TCP_IP[1][1] = FlashData >> 8; 
    FlashAddress += 2;                              // IP Tab 0 & IP Tab 1
    
    FlashData = (*(vu16*) FlashAddress);
    //TCP_IP[1][2] = (char)FlashData; TCP_IP[1][3] = FlashData >> 8; 
    FlashAddress += 2;                              // IP Tab 2 & IP Tab 3
    
    SRVR_Port[EMR] = 0x0000FFFF & (*(vu16*) FlashAddress);            
    FlashAddress += 2;          // IP PORT

     /*----- GET TCP SERVER IP ADDRESS & TCP PORT OF TERTIARY SERVER ------------*/
    FlashData = (*(vu16*) FlashAddress);
    //TCP_IP[2][0] = (char)FlashData; TCP_IP[2][1] = FlashData >> 8; 
    FlashAddress += 2;                              // IP Tab 0 & IP Tab 1
  
    FlashData = (*(vu16*) FlashAddress);
    //TCP_IP[2][2] = (char)FlashData; TCP_IP[2][3] = FlashData >> 8; 
    FlashAddress += 2;                              // IP Tab 2 & IP Tab 3
    
    SRVR_Port[TER] = 0x0000FFFF & (*(vu16*) FlashAddress);            FlashAddress += 2;          // IP PORT
  
    /*----- SERVER HIT TIMES--------------------------------------------------*/ 
    Active_Time  = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;      // Active Angle
    StandBy_Time = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;      // Standby Time
    Trans_Time   = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;      // ACTIVE TO STANDBY TRANSITION TIME
    ES_Time      = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;      //
    
     /*----- SERVER INFORMAION ------------------------------------------------*/
    GP_Counter = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    if((GP_Counter & 0x01) == 0x01)Flag_MAINServer = SET; else Flag_MAINServer = RESET;
    if((GP_Counter & 0x02) == 0x02)Flag_EMRServer  = SET; else Flag_EMRServer  = RESET;
    if((GP_Counter & 0x04) == 0x04)Flag_TERServer  = SET; else Flag_TERServer  = RESET;
    
    /******************* SERVER INFORMATION **************************************/
    TER_HitTime = 0x0000FFFF & (*(vu16*) FlashAddress);
    FlashAddress += 8;                                                          // Leaving Additional 26  Bytes for future use

    /*----- INTERNL BATTERY THRESHOLD ----------------------------------------*/
    IBL = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;  
    
    /*--------- HELATH PACKET TIMER ------------------------------------------*/
    HLTH_FRQ = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 4;
    
    /*----- EMG BUTTON PRESS TIMER ----------------------------------------*/
    EMG_BP_FREQ = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;  
 
  
    /*----- SPEED LIMIT ------------------------------------------------------*/
    Speed_Limit = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;  
    
    /*---------- MEMS DATA ------------------------------------------------------*/
    
    memset(temp,0,5);
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                            // HARSH BREAKING                                         
    {
      FlashData = (*(vu16*) FlashAddress);  
      temp[GP_Counter+1] = FlashData >> 8;temp[GP_Counter] = (char)FlashData; 
      FlashAddress += 2;GP_Counter++;
    };
    HB = atof(temp);
 
    memset(temp,0,5);
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                            // HARSH ACCELERATION                                        
    {
      FlashData = (*(vu16*) FlashAddress);  
      temp[GP_Counter+1] = FlashData >> 8;temp[GP_Counter] = (char)FlashData; 
      FlashAddress += 2;GP_Counter++;
    };
    HA = atof(temp);
    RT = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 4;  

    memset(temp,0,5);
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                            // RASH TURNING                                      
    {
      FlashData = (*(vu16*) FlashAddress);  
      temp[GP_Counter+1] = FlashData >> 8;temp[GP_Counter] = (char)FlashData; 
      FlashAddress += 2;GP_Counter++;
    };
    Accel_WK_Thrs = atof(temp);
 
     /*---------- VEHICLE ID & REGISTRATION -----------------------------------*/  
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                           // Get SOS Mobile No, at start set Mobile No aslo same
    {
       FlashData = (u16)((*(vu16*) FlashAddress));
       //Vehicle.VID[GP_Counter+1]  = FlashData >> 8;Vehicle.VID[GP_Counter] = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
  
    for(GP_Counter = 0;GP_Counter < 16; GP_Counter++)                           // Get SOS Mobile No, at start set Mobile No aslo same
    {
       FlashData = (u16)((*(vu16*) FlashAddress));
       Vehicle.VRN[GP_Counter+1]  = FlashData >> 8;Vehicle.VRN[GP_Counter] = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };

    /*---------- VEHICLE HEADER ---------------------------------------------*/  
    for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)                           // Get SOS Mobile No, at start set Mobile No aslo same
    {
       FlashData = (u16)((*(vu16*) FlashAddress));
       //Vehicle.HDR[GP_Counter+1]  = FlashData >> 8;Vehicle.HDR[GP_Counter] = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
    
    FlashData = (u16)((*(vu16*) FlashAddress));
    FlashAddress += 2;
    if(FlashData == 1)Flag_MEMS_ENB = SET;
    else Flag_MEMS_ENB = RESET;
   
    Flash_Error = RESET;
    /*-------------SRVR IP 1--------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)                          // GET SET APN                                        
    {
        if(Flash_Error == RESET)
        {
          FlashData = (u16)((*(vu16*) FlashAddress));  
          SRVR_IP[MAIN][GP_Counter+1] = FlashData >> 8;
          SRVR_IP[MAIN][GP_Counter]   = (char)FlashData; 
        }
        if(SRVR_IP[MAIN][0] == 255 || SRVR_IP[MAIN][0] == 0){Flash_Error = SET;}
        FlashAddress += 2;GP_Counter++;
     };
     
    Flash_Error = RESET;
    
    /*-------------SRVR IP 2--------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)                          // GET SET APN                                        
    {
      if(Flash_Error == RESET)
      {
        FlashData = (u16)((*(vu16*) FlashAddress));  
        SRVR_IP[EMR][GP_Counter+1] = FlashData >> 8;
        SRVR_IP[EMR][GP_Counter]   = (char)FlashData; 
      }
      if(SRVR_IP[EMR][0] == 255 || SRVR_IP[EMR][0] == 0){Flash_Error = SET;}
      FlashAddress += 2;GP_Counter++;
    };
   
    Flash_Error = RESET;
    /*-------------SRVR IP 3-------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)                          // GET SET APN                                        
    {
      if(Flash_Error == RESET)
      {
        FlashData = (u16)((*(vu16*) FlashAddress));  
        SRVR_IP[TER][GP_Counter+1] = FlashData >> 8;
        SRVR_IP[TER][GP_Counter]   = (char)FlashData; 
      }
      if(SRVR_IP[TER][0] == 255 || SRVR_IP[TER][0] == 0){Flash_Error = SET;}
      FlashAddress += 2;GP_Counter++;
    };
    Flash_Error = RESET;
    /*-------------FOTA IP -------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)                          // GET SET APN                                        
    {
      FlashAddress += 2;GP_Counter++;
    };
    Flash_Error = RESET;
   
    FlashAddress += 2;
     
    ED_Time  = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    IDL_Time = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    EMG_WC_Timeout = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    DO_STATUS      = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    FOTA_IMAGE     = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    StopMode_Time  = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    Time2Sleep     = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    OVS_DeadTime   = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    GP_Counter = 0;
    GP_Counter   = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    if(GP_Counter == 1)Flag_STOPMSG = SET; else Flag_STOPMSG = RESET;
  
    for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)                          // GET SET APN                                        
    {
      FlashData = (*(vu16*) FlashAddress);  
      SER_USN[GP_Counter+1] = FlashData >> 8;
      SER_USN[GP_Counter] = (char)FlashData; 
      FlashAddress += 2;GP_Counter++;
    };
    for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)                          // GET SET APN                                        
    {
      FlashData = (*(vu16*) FlashAddress);  
      SER_PWD[GP_Counter+1] = FlashData >> 8;
      SER_PWD[GP_Counter] = (char)FlashData; 
      FlashAddress += 2;GP_Counter++;
    };
  
    Protocol = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    TA = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    OFFSET_X = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    OFFSET_Y = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    SleepAgainTime = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    GP_Counter = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    if(GP_Counter == 1)Flag_SWAP_ENB = SET;
    else Flag_SWAP_ENB = RESET;
    EBL = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2; 
    DeepSleepFreq = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    DeepSleepFreq |= ((0x0000FFFF & (*(vu16*) FlashAddress)) << 16);FlashAddress += 2;
    GP_Counter = 0x0000FFFF & (*(vu16*) FlashAddress);FlashAddress += 2;
    if(GP_Counter == 1)Flag_DSF_ENB = SET;
    else Flag_DSF_ENB = RESET;
  

   /*-----PARAMETERS' VALIDATION----------------------------------------------*/ 
   if((Active_Time  > 3600)  || (Active_Time  < 2)) Active_Time  = 10;
   if((StandBy_Time > 3600)  || (StandBy_Time < 5)) StandBy_Time = 300;
   if((ES_Time      > 3600)  || (ES_Time      < 5)) ES_Time      = 30;
   if((Trans_Time   > 60)    || (Trans_Time  < 10)) Trans_Time = 20;
   if((StopMode_Time > 3600) || (StopMode_Time < 5))StopMode_Time = 1800;
   if(Time2Sleep  > 3600)Time2Sleep = 120;
   if(( SleepAgainTime  > 600)  || (SleepAgainTime    < 180))SleepAgainTime = 180;
   if((TER_HitTime  > 3600)  || (TER_HitTime  < 5)) TER_HitTime  = 10;
   
 
 
   if((Speed_Limit  > 220)   || (Speed_Limit < 10)) Speed_Limit  = 80;
   if((OVS_DeadTime > 3600)  || (OVS_DeadTime  < 5))OVS_DeadTime = 5;
   if(HA > 3.5 || HA < 0.1)HA = 2.5;                                      // VALUE IN G-FORCE 
   if(HB > 3.5 || HB < 0.1)HB = 2.5;
   if(RT > 360 || RT < 10) RT = 35;
   if((Accel_WK_Thrs < 0.2) || (Accel_WK_Thrs > 7.9))Accel_WK_Thrs = 5.0;
   if(IBL > 50 || IBL < 5)IBL = 35;
   if(EBL > 30 || EBL < 5)EBL = 15;
   if(HLTH_FRQ < 5 || HLTH_FRQ > 3600)HLTH_FRQ = 600;
   if(EMG_BP_FREQ    > 60000 || EMG_BP_FREQ    < 50)EMG_BP_FREQ = 3000;        // MILISECOND 
   if(EMG_WC_Timeout > 60000 || EMG_WC_Timeout < 50)EMG_WC_Timeout = 15000;     // MILI SECOND
   if(IDL_Time > 60  || IDL_Time < 1)IDL_Time = 10;                            // MINUTES                       

   if(DO_STATUS > 3)DO_STATUS = 0;
   if(ED_Time > 7200 || ED_Time < 60)ED_Time = 300;
   if(DeepSleepFreq > 86400 || DeepSleepFreq < 60) DeepSleepFreq = 86400;

   if(OFFSET_X > 180)OFFSET_X = 0;
   if(OFFSET_Y > 180)OFFSET_Y = 0;
   if(TA > 180)TA = 55;
  
   /************** geo fence ********************/
   // GEOFENCE DATA IN PREVIOUS PAGE 
   FlashAddress = EndAddr-FLASH_PAGE_SIZE-FLASH_PAGE_SIZE;
   for(FA = 0; FA < 10; FA++)
   {    
      GF[FA].ID   = 0x0000FFFF & (*(vu16*) FlashAddress);                     FlashAddress += 2;
      
      for(FP = 0; FP < 10; FP++)
      {
        GF[FA].LAT[FP] = 0x0000FFFF & (*(vu16*) FlashAddress);                FlashAddress += 2;
        
        loc            = 0x0000FFFF & (*(vu16*) FlashAddress);                FlashAddress += 2;
        loc           |= ((0x0000FFFF & (*(vu16*) FlashAddress)) << 16);      FlashAddress += 2;
        GF[FA].LAT[FP] += (loc/1000000.0);      
        
        GF[FA].LON[FP] = 0x0000FFFF & (*(vu16*) FlashAddress);                FlashAddress += 2;
        
        loc            = 0x0000FFFF & (*(vu16*) FlashAddress);                FlashAddress += 2;
        loc           |= ((0x0000FFFF & (*(vu16*) FlashAddress)) << 16);      FlashAddress += 2;
        GF[FA].LON[FP] += (loc/1000000.0);    
       
      }
    }
  

   FlashAddress = (EndAddr-FLASH_PAGE_SIZE) + 1024 - 36;   
   /*----- APN NAME ---------------------------------------------------------*/
   for(GP_Counter = 0;GP_Counter < 32; GP_Counter++)                          // GET SET APN                                        
   {
      FlashData = (*(vu16*) FlashAddress);  
      GPRSData.SET_APN[GP_Counter+1] = FlashData >> 8;GPRSData.SET_APN[GP_Counter] = (char)FlashData; 
      if( GPRSData.SET_APN[0] == 255 || GPRSData.SET_APN[0] == 0){sprintf(GPRSData.SET_APN,"no-apn");}
      FlashAddress += 2;GP_Counter++;
   };
  
    /***************************************************************************/
    /*                    DFU BLOCK                                            */
    /***************************************************************************/
  
    FlashAddress = (EndAddr-FLASH_PAGE_SIZE) + 1024; 
    /*----- FTP IP ADDRESS --------------------------------------------------*/
    FlashData = (*(vu16*) FlashAddress);
    DFU_HTTP.IP[0] = (char)FlashData;DFU_HTTP.IP[1] = FlashData >> 8;FlashAddress += 2;                             // DFU IP TAB 1,2
    FlashData = (*(vu16*) FlashAddress);
    DFU_HTTP.IP[2] = (char)FlashData;DFU_HTTP.IP[3] = FlashData >> 8;FlashAddress += 2;                             // DFU IP TAB 3,4
  
    
    /*----- HTTP PATH ----------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter <= 50; GP_Counter++)                           // GET FTP USERNAME   
    {
       FlashData = (*(vu16*) FlashAddress);
       DFU_HTTP.PATH[GP_Counter+1] = FlashData >> 8;DFU_HTTP.PATH[GP_Counter]  = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
  
    /*----- HTTP FILE NAME -----------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter <= 14; GP_Counter++)                           // GET FTP USERNAME   
    {
       FlashData = (*(vu16*) FlashAddress);
       DFU_HTTP.FILENAME[GP_Counter+1] = FlashData >> 8;DFU_HTTP.FILENAME[GP_Counter]  = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
   
    /*----- HTTP URL -----------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter <= 100; GP_Counter++)                           // GET FTP USERNAME   
    {
       FlashData = (*(vu16*) FlashAddress);
       DFU_HTTP.URL[GP_Counter+1] = FlashData >> 8;DFU_HTTP.URL[GP_Counter]  = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
   
    /*----- DFU NUMBER -------------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 15; GP_Counter++)                           // GET DFU MOBILE NUMBER TO UPGRADE SOFTWARE VERSION  
    {
       FlashData = (*(vu16*) FlashAddress);
       DFU_Mob[GP_Counter+1] = FlashData >> 8;DFU_Mob[GP_Counter]  = (char)FlashData;  
       FlashAddress += 2;GP_Counter++;
    };
  
  }
  else
  {
    SET_FACTORY();
  }
}



void SET_FACTORY(void)
{
  uint32_t FlashAddress;
  uint8_t GP_Counter;
  char loc_buffer[30];
  
  GP_Counter = 0;
  FlashAddress = 0;
  
  
  memset(loc_buffer,0,30);
  //@PASS#EDS-UPGRADE,IP-13.234.46.63,PATH-4GAIS_MMI,FILE-V-1.0.00,<CR><LF>
  /*---------- CLEAR DFU DATA --------------------------------------------*/
  DFU_HTTP.IP[0] =  13;                                                          // DFU SERVER IP ADDRESS
  DFU_HTTP.IP[1] = 234;
  DFU_HTTP.IP[2] =  46;
  DFU_HTTP.IP[3] =  63;
  sprintf(DFU_HTTP.PATH,"NVT_4G");                                              // DFU HTTP PATH
  sprintf(DFU_HTTP.FILENAME,"V-1.0.00");                                        // DFU HTTP FILE NAME 
  
  /*---------- SET DEFAULT TO ALL CONFIGURABLE VALUE ---------------------*/
  sprintf((char*)GPRSData.Mobile_No,"N.A0000000");
  
#if defined(VODA_BSNL) || defined(TAISYS_VB)
  sprintf(&Vehicle.Master_No[0][0],"N.A0000000");                               // DEFAULT MASTER NUMBER SET TO NONE
  sprintf(&Vehicle.Master_No[1][0],"N.A0000000");                               // BSNL number added as per mail on 26/03/2021
  sprintf(&Vehicle.Master_No[2][0],"N.A0000000");                               // DEFAULT MASTER NUMBER SET TO NONE AIRETL ADDED ON 13012021
  sprintf(&Vehicle.Master_No[3][0],"N.A0000000");                               // DEFAULT MASTER NUMBER SET TO NONE AIRTEL
  sprintf(&Vehicle.Master_No[4][0],"N.A0000000");                               // DEFAULT MASTER NUMBER SET TO NONE IDEA 
#elif defined(AIRTEL_BSNL) || defined(TAISYS_AB)
  sprintf(&Vehicle.Master_No[0][0],"N.A0000000");                               // DEFAULT MASTER NUMBER SET TO NONE
  sprintf(&Vehicle.Master_No[1][0],"8929921179");                               // BSNL number added as per mail on 26/03/2021
  sprintf(&Vehicle.Master_No[2][0],"N.A0000000");                               // DEFAULT MASTER NUMBERSET TO NONE AIRTEL ADDED ON 13012021
  sprintf(&Vehicle.Master_No[3][0],"7428933677");                               // DEFAULT MASTER NUMBER SET TO NONE IDEA
  sprintf(&Vehicle.Master_No[4][0],"7428933688");                               // DEFAULT MASTER NUMBER SET TO NONE AIRTEL 
#endif      

  
  sprintf((char*)PWD_IN,"RMDE");                                                 // Defauult Password

 /*-----------------DEFAULT = DIMTS -------------------------------------------*/    
 Protocol = BSNL_PC;                                                            //DIMTS_PC
 /*------------------MAIN IP --------------------------------------------------*/
 memset(&SRVR_IP[MAIN][0],0,50);
 sprintf(&SRVR_IP[MAIN][0],"35.154.23.247");
 SRVR_Port[MAIN] = 20005;
  
 Flag_MAINServer  = SET;  
 /*------------------EMR IP ---------------------------------------------------*/
 memset(&SRVR_IP[EMR][0],0,50);
 sprintf(&SRVR_IP[EMR][0],"vltsemg.delhi.gov.in");
 SRVR_Port[EMR] = 9032;
 Flag_EMRServer   = SET;
  
 /*------------------TER IP -----------------------------------------------*/
  memset(&SRVR_IP[TER][0],0,50);
  sprintf(&SRVR_IP[TER][0],"track.rdmautomotive.net"); // sprintf(&SRVR_IP[TER][0],"103.233.79.13"); 103.233.79.13
  SRVR_Port[TER] = 11664;           
  Flag_TERServer   =  SET;

 /*----------------------------UI----------------------------------------------*/ 
 //UI :- A:10,O:300,H:600,E:30,ET:50,ST:15000,TS:120,S:1800,C:10,OI:5
 Active_Time    = 10;                                                           // ACTIVE TIME / IGNITION ON UI
 StandBy_Time   = 30;                                                           // STANDBY TIME / IGNITION OFF UI
 HLTH_FRQ       = 600;
 ES_Time        = 30;                                                           // EMERGENCY STATE TIME
 ED_Time        = 1800;                                                         // TIME IN SECONDS
  
 Time2Sleep     = 0;
 StopMode_Time  = 600;
 TER_HitTime    = 10;
 OVS_DeadTime   = 5;
  
 //TD :- LB:035,HA:000.40,HB:000.60,RT:035.00,SL:80,WK:0.15,ID:10
 IBL = 35;
 EBL = 15;
 HA = 2.5;                                                                     // G-FORCE 
 HB = 2.5;                                                                     // G-FORCE 
 RT = 35;                                                                  
 Speed_Limit = 80;                                                              // SPEED LIMIT
 Accel_WK_Thrs = 5.0;
 IDL_Time = 10;                                                                 // MAXIMUM THRESHOLD IN SECOND  
  

 Trans_Time = 20;
 EMG_BP_FREQ = 1000;
 EMG_WC_Timeout = 50000;
 DeepSleepFreq = 60;
    
 SleepAgainTime = 180;//300;
 Flag_SWAP_ENB  = SET; 
  
 Flag_STOPMSG   = SET;
 Flag_MEMS_ENB  = SET;
 Flag_DSF_ENB   = SET;
  
 FOTA_IMAGE = 0;
  
 OFFSET_X = 0;
 OFFSET_Y = 0;
 TA = 55;

  
#if defined(SENSORISE)       
  #if defined(VODA_BSNL)
    sprintf(GPRSData.SET_APN,"sensem2m");
  #elif defined(AIRTEL_BSNL)
    sprintf(GPRSData.SET_APN,"sensem2m2");
  #else 
    sprintf(GPRSData.SET_APN,"no-apn"); 
  #endif
#elif defined (TAISYS) || defined (TAISYS_AB) || defined (TAISYS_VB)
  sprintf(GPRSData.SET_APN,"taisysnet"); 
#endif
  
 sprintf((char*)Vehicle.VRN,"NA00000000");
  
 sprintf(SER_USN,"rdme 123");
 sprintf(SER_PWD,"RDME@123");
  
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

 OP1_STOP;Vehicle.DOUT &=0xFE; 
 OP2_STOP;Vehicle.DOUT &=0xFD; 

 DO_STATUS = Vehicle.DOUT;
 TCP_BackCnt[MAIN] = 0;
 TCP_BackCnt[TER] = 0;
 EMRGY_BKP_No = 0;
 RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
 EMRGY_BKP_No  = 0;        
 FlashAddress = SPIFLASH_CONFIG;
 memset(loc_buffer,0,30);
 sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
 if(Flag_SPIOK == SET)
 {
   SPI_FLASH_SectorErase(FlashAddress);
   SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
 }
 
 KM = 0;
 MTR = 0;
 Write_Oddo();
  
 Write_Config();                                                           // WRITE GOT CONFIGURATION IN FLASH MEMORY FOR FURTHER USE 
   
}
/*******************************************************************************
* Function Name  : mSec_Delay
* Description    : Insert Delay in mili Seconds
* Input          : mili seconds 
* Output         : None
* Return         : None
*******************************************************************************/
void mSec_Delay(unsigned int nTime)
{
 while(nTime != 0)                                                              // MILI SECOND DELAY 
  { 
    if(Flag_mSec == SET){Flag_mSec = RESET;nTime--;};                           // MILI SECOND FLAG IS SET BY SYSTICK TIMER 
  };
}


/*******************************************************************************
* Function Name  : Run_IAP
* Description    : Call Inapplication Programming
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Run_IAP(void)
{
 uint16_t GP_Counter;
 
 GP_Counter = 0;
 
  //Flag_ENGMode = RESET;Modem_PutString("AT+QENG=0,0");WAIT_MODEM_RESP(2);
  Vehicle.INT_VOLT = 4.0;
  if((Vehicle.Battery[0] == 'C') && (Vehicle.INT_VOLT > 3.3))
  {
    if (((*(__IO uint32_t*)0x08000000) & 0x2FFE0000 ) == 0x20000000)
       { 
         if(Flag_Serial_IAP == SET)
         {
          if(Flag_TCPConnect[MAIN]){Flag_TCPConnect[MAIN] = RESET; Modem_PutString("AT+QICLOSE=0");WAIT_MODEM_RESP(2);};
          if(Flag_TCPConnect[TER]){Flag_TCPConnect[TER] = RESET; Modem_PutString("AT+QICLOSE=1");WAIT_MODEM_RESP(2);};
          if(Flag_TCPConnect[EMR]){Flag_TCPConnect[EMR] = RESET; Modem_PutString("AT+QICLOSE=2");WAIT_MODEM_RESP(2);};
         }
    
        GP_Counter = 0;
        if(Flag_Serial_IAP){GP_Counter = 0x00 << 8 | 'S';}
        else{GP_Counter = 0x00 << 8 | 'I';}
        
        Flag_Serial_IAP = RESET;      
        RTC_WriteBackupRegister(RTC_BKP_DR4,GP_Counter);
        TAKE_FUL_BKP();
        SYSTEM_SW_RESET;
       }
     }
  else 
  {
    if(Vehicle.Battery[0] != 'C')Serial_PutString_232("CONNECT EXTERNAL POWER...",3);
    else if(Vehicle.INT_VOLT < 3.5)Serial_PutString_232("INTERNAL BATTERY VOLTAGE TOO LOW...",3);
  }
}


/*******************************************************************************
* Function Name  : Write_Default
* Description    : Write Default Data in Flash Memory
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Write_Config(void)
{
  char temp[5];
  uint8_t 
  FP,
  FA;
  uint16_t 
  FlashData,
  GP_Counter;
  uint32_t FlashAddress;
  uint64_t loc;
  
  FP = 0;
  FA = 0;
  FlashData = 0;
  GP_Counter = 0;
  FlashAddress = 0;
  loc = 0;
  memset(temp,0,5);
 /*---- UNLOCK THE FLASH PROGRAM ERASE CONTROLLER ----------------------------*/
 FLASH_Unlock();								// UNLOCK FLASH

 /*----- CLEAR ALL PENDING FLAGS ---------------------------------------------*/
 FLASH_ClearFlag(FLASH_FLAG_BSY);
 FLASH_ClearFlag(FLASH_FLAG_PGERR);
 FLASH_ClearFlag(FLASH_FLAG_WRPERR);
 FLASH_ClearFlag(FLASH_FLAG_EOP);
 
 FlashAddress = EndAddr-FLASH_PAGE_SIZE;					//  SET FLASH ADDRESS
 
/*----- ERASE THE FLASH PAGES -----------------------------------------------*/
 if(FLASH_ErasePage(FlashAddress) == FLASH_COMPLETE)                            // ERASE FLASH PAGES 
 {
 /*----- FLASH HALFWORD PROGRAM ----------------------------------------------*/
  if(FLASH_ProgramHalfWord(FlashAddress,0xED) == FLASH_COMPLETE)
  {
    FlashAddress += 2;  // SIGNATURE  BYTE
 
 /*----- WRITE SOS NUMBER ----------------------------------------------------*/ 
   for(GP_Counter = 0;GP_Counter < 14; GP_Counter++)				
   {
    FlashAddress += 2;GP_Counter++;
   }
   
   for(loc = 0; loc < 5; loc++)
   {
     for(GP_Counter = 0;GP_Counter < 14; GP_Counter++)				
     {
      FlashData = (Vehicle.Master_No[loc][GP_Counter+1] <<8) | Vehicle.Master_No[loc][GP_Counter];// GET MASTER NUMBER 
      FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH 
      FlashAddress += 2;GP_Counter++;
     }
   }
 
 /*----- WRITE PASSWORD ALPHANUMERIC -----------------------------------------*/ 
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				// GET 5 BYTES FOR PASSWORD STRING 
   {
     FlashData = (PWD_IN[GP_Counter+1]<<8) | PWD_IN[GP_Counter] ;		// GET PASSWORD DATA
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
 FlashAddress += 36;                                                            // Next Blok start at 50 leaving some space for future
 
 /*---- WRITE TCP SERVER IP ADDRESS & TCP PORT -------------------------------*/
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[0][1]<<8 | TCP_IP[0][0]));
 FlashAddress += 2; // IP TAB 0 & 1
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[0][3]<<8 | TCP_IP[0][2]));
 FlashAddress += 2; // IP TAB 2 & 3
 FLASH_ProgramHalfWord(FlashAddress,SRVR_Port[MAIN]);FlashAddress += 2;                   // TCP Port
 
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[1][1]<<8 | TCP_IP[1][0]));
 FlashAddress += 2;     // IP TAB 0 & 1 to maintian same table sime IP is reprinted
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[1][3]<<8 | TCP_IP[1][2]));
 FlashAddress += 2;     // IP TAB 2 & 3
 FLASH_ProgramHalfWord(FlashAddress,SRVR_Port[EMR]);FlashAddress += 2;                       // TCP Port

   /*---- WRITE TCP SERVER IP ADDRESS & TCP PORT TERTIARY SERVER -------------*/
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[2][1]<<8 | TCP_IP[2][0]));
 FlashAddress += 2;     // IP TAB 0 & 1 to maintian same table sime IP is reprinted
 //FLASH_ProgramHalfWord(FlashAddress,(TCP_IP[2][3]<<8 | TCP_IP[2][2]));
 FlashAddress += 2;     // IP TAB 2 & 3
 FLASH_ProgramHalfWord(FlashAddress,SRVR_Port[TER]);FlashAddress += 2;                       // TCP Port

/*----- WRITE SERVER HIT TIMES ----------------------------------------------*/
 FLASH_ProgramHalfWord(FlashAddress,Active_Time); FlashAddress += 2;             // Active Time
 FLASH_ProgramHalfWord(FlashAddress,StandBy_Time);FlashAddress += 2;            // StandBy Time
 FLASH_ProgramHalfWord(FlashAddress,Trans_Time);FlashAddress += 2;            // StandBy Time
 FLASH_ProgramHalfWord(FlashAddress,ES_Time);     FlashAddress += 2;
  
 /*----- SERVER INFORMATION --------------------------------------------------*/
 GP_Counter = 0;
 if(Flag_MAINServer == SET) GP_Counter |= 0x01;
 if(Flag_EMRServer  == SET) GP_Counter |= 0x02;
 if(Flag_TERServer  == SET) GP_Counter |= 0x04;
 FLASH_ProgramHalfWord(FlashAddress,GP_Counter);FlashAddress += 2;              // Active Angle


 
 FLASH_ProgramHalfWord(FlashAddress,TER_HitTime);             // Active Time
 FlashAddress += 8;                                                             // Leaving Additional 8  Bytes for future use
 
 
 /*---------- INTERNAL BATTERY THRESHOLD -------------------------------------*/
 FLASH_ProgramHalfWord(FlashAddress,IBL);             FlashAddress += 2;
 
 /*---------- HEALTH FREQUENCY -----------------------------------------------*/
 FLASH_ProgramHalfWord(FlashAddress,HLTH_FRQ);        FlashAddress += 4;
 
  /*---------- EMG BUTTON PRESS TIMER -------------------------------------*/
 FLASH_ProgramHalfWord(FlashAddress,EMG_BP_FREQ);     FlashAddress += 2;
 
 
/*----------SPEED LIMIT -----------------------------------------------------*/
 FLASH_ProgramHalfWord(FlashAddress,Speed_Limit);     FlashAddress += 2;
 
 /*---------- MEMS DATA ------------------------------------------------------*/
 memset(temp,0,5);
 if(HB > 10)ftoa(HB,temp,2,1);
 else ftoa(HB,temp,1,1);
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				//  
   {
     FlashData = (temp[GP_Counter+1]<<8) |temp[GP_Counter];                     // 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
 memset(temp,0,5);
 if(HA > 10)ftoa(HA,temp,2,1);
 else ftoa(HA,temp,1,1);
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				//  
   {
     FlashData = (temp[GP_Counter+1]<<8) |temp[GP_Counter];                     // 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
 FLASH_ProgramHalfWord(FlashAddress,RT); FlashAddress += 4;	
 
 
 memset(temp,0,5);
 ftoa(Accel_WK_Thrs,temp,1,1);
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				//  
   {
     FlashData = (temp[GP_Counter+1]<<8) |temp[GP_Counter];                     // 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
 /*---------- VEHICLE ID & REGISTRATION NUMBER -------------------------------*/
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				// GET 10 BYTES FOR VEHICLE ID
   {
     //FlashData = (Vehicle.VID[GP_Counter+1]<<8) | Vehicle.VID[GP_Counter] ;	// GET PASSWORD DATA
     //FLASH_ProgramHalfWord(FlashAddress, FlashData);				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
  for(GP_Counter = 0;GP_Counter < 16; GP_Counter++)				// GET 10 BYTES FOR VEHICLE ID
   {
     FlashData = (Vehicle.VRN[GP_Counter+1]<<8) | Vehicle.VRN[GP_Counter] ;	// GET PASSWORD DATA
     FLASH_ProgramHalfWord(FlashAddress, FlashData);				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
  
  /*---------- VEHICLE HEADER -------------------------------------------------*/
 for(GP_Counter = 0;GP_Counter < 4; GP_Counter++)				// GET 10 BYTES FOR VEHICLE ID
   {
     //FlashData = (Vehicle.HDR[GP_Counter+1]<<8) | Vehicle.HDR[GP_Counter] ;	// GET PASSWORD DATA
     //FLASH_ProgramHalfWord(FlashAddress, FlashData);				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
   if(Flag_MEMS_ENB == SET)
   {
     FLASH_ProgramHalfWord(FlashAddress,1);
   }
   else
   {
     FLASH_ProgramHalfWord(FlashAddress,0);
   }
   FlashAddress += 2;
   
   
    /*----- WRITE SRVR IP 1------------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)				
   {
     FlashData = (SRVR_IP[MAIN][GP_Counter+1]<<8) | SRVR_IP[MAIN][GP_Counter];                     
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
   
     /*----- WRITE URL2------------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)				
   {
     FlashData = (SRVR_IP[EMR][GP_Counter+1]<<8) | SRVR_IP[EMR][GP_Counter];                     
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
   
     /*----- WRITE URL3------------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)				 
   {
     FlashData = (SRVR_IP[TER][GP_Counter+1]<<8) | SRVR_IP[TER][GP_Counter];                     
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
   
     /*----- FOTA IP------------------------------------------------------*/
    for(GP_Counter = 0;GP_Counter < 50; GP_Counter++)				 
   {
     		// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
   
   
 
  //FLASH_ProgramHalfWord(FlashAddress,FOTA_Port);  
  FlashAddress += 2;              // EMERGENCY TIMEOUT 
   
  FLASH_ProgramHalfWord(FlashAddress,ED_Time);  FlashAddress += 2;              // EMERGENCY TIMEOUT 
  FLASH_ProgramHalfWord(FlashAddress,IDL_Time); FlashAddress += 2;              // IDLE TIME 
  FLASH_ProgramHalfWord(FlashAddress,EMG_WC_Timeout); FlashAddress += 2;        // IDLE TIME
  FLASH_ProgramHalfWord(FlashAddress,DO_STATUS);    FlashAddress += 2;          // IDLE TIME
  FLASH_ProgramHalfWord(FlashAddress,FOTA_IMAGE);   FlashAddress += 2;          // IDLE TIME
  FLASH_ProgramHalfWord(FlashAddress,StopMode_Time);FlashAddress += 2;          // STANDBY MODE UPDATE INTERVAL 
  FLASH_ProgramHalfWord(FlashAddress,Time2Sleep);   FlashAddress += 2;          // TIMER VALUE TO JUMP INTO STANDBY MODE AFTER IGNITION OFF 
  FLASH_ProgramHalfWord(FlashAddress,OVS_DeadTime); FlashAddress += 2;          // OVERSPEED DEAD TIME TO DETECT IT AGAIN 
  FLASH_ProgramHalfWord(FlashAddress,Flag_STOPMSG); FlashAddress += 2;          // ENABLE/DISABLE CLR SOS MESSAGE 
   for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)				// GET 20 BYTES FOR SET APN NAME 
   {
     FlashData = (SER_USN[GP_Counter+1]<<8) | SER_USN[GP_Counter];// GET PASSWORD DATA 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
    for(GP_Counter = 0;GP_Counter < 10; GP_Counter++)				// GET 20 BYTES FOR SET APN NAME 
   {
     FlashData = (SER_PWD[GP_Counter+1]<<8) | SER_PWD[GP_Counter];// GET PASSWORD DATA 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
 
   FLASH_ProgramHalfWord(FlashAddress,Protocol);  FlashAddress += 2;            // EMERGENCY TIMEOUT 
   FLASH_ProgramHalfWord(FlashAddress,TA);  FlashAddress += 2;                  // EMERGENCY TIMEOUT 
   FLASH_ProgramHalfWord(FlashAddress,OFFSET_X);  FlashAddress += 2;            // EMERGENCY TIMEOUT 
   FLASH_ProgramHalfWord(FlashAddress,OFFSET_Y);  FlashAddress += 2;            // EMERGENCY TIMEOUT 
   FLASH_ProgramHalfWord(FlashAddress,SleepAgainTime);  FlashAddress += 2;      // TIMEOUT VALUE FOR GOING INTO SLEEP MODE IF NOT MEET ANY IDEAL CONDITION 
   if(Flag_SWAP_ENB == SET)FLASH_ProgramHalfWord(FlashAddress,1);
   else FLASH_ProgramHalfWord(FlashAddress,0);
   FlashAddress += 2;
   FLASH_ProgramHalfWord(FlashAddress,EBL);  FlashAddress += 2;
   FLASH_ProgramHalfWord(FlashAddress,(unsigned short)DeepSleepFreq); FlashAddress += 2;        // DEEP SLEEP FREQ 
   FLASH_ProgramHalfWord(FlashAddress,(unsigned short)(DeepSleepFreq >> 16)); FlashAddress += 2;        // DEEP SLEEP FREQ
   if(Flag_DSF_ENB == SET)FLASH_ProgramHalfWord(FlashAddress,1);                // Flag_DSF_ENB
   else FLASH_ProgramHalfWord(FlashAddress,0);
   FlashAddress += 2;
   
 
   
   
     // GEOFENCE DATA IN PREVIOUS PAGE 
    FlashAddress = EndAddr-FLASH_PAGE_SIZE-FLASH_PAGE_SIZE;
    /*----- ERASE THE FLASH PAGES -----------------------------------------------*/
    if(FLASH_ErasePage(FlashAddress) == FLASH_COMPLETE)				        // ERASE FLASH PAGES 
    {
      for(FA = 0; FA < 10; FA++)
      {
        FLASH_ProgramHalfWord(FlashAddress,(unsigned short)GF[FA].ID);            FlashAddress += 2;
        
       
        //FLASH_ProgramHalfWord(FlashAddress,GF[FA].Type);                          FlashAddress += 2;
        for(FP = 0; FP < 10; FP++)
        {
          loc = (int)(GF[FA].LAT[FP]);
          FLASH_ProgramHalfWord(FlashAddress,loc);                                FlashAddress += 2;
          loc = (int)((GF[FA].LAT[FP])*1000000);
          loc = (loc%1000000);
          FLASH_ProgramHalfWord(FlashAddress,(unsigned short)loc);                FlashAddress += 2;
          FLASH_ProgramHalfWord(FlashAddress,(unsigned short)(loc >> 16));        FlashAddress += 2;
          
          loc = (int)(GF[FA].LON[FP]);
          FLASH_ProgramHalfWord(FlashAddress,loc);                                FlashAddress += 2;
          loc = (int)((GF[FA].LON[FP])*1000000);
          loc = (loc%1000000);
          FLASH_ProgramHalfWord(FlashAddress,(unsigned short)loc);                FlashAddress += 2;
          FLASH_ProgramHalfWord(FlashAddress,(unsigned short)(loc >> 16));        FlashAddress += 2;
        }
       }
     }

  FlashAddress = (EndAddr-FLASH_PAGE_SIZE) + 1024 - 72; 
 /*----- WRITE CURRENT USED APN NAME -----------------------------------------*/
 for(GP_Counter = 0;GP_Counter < 32; GP_Counter++)				// GET 20 BYTES FOR SET APN NAME 
   {
     FlashData = (GPRSData.APN_Name[GP_Counter+1]<<8) | GPRSData.APN_Name[GP_Counter];// GET PASSWORD DATA 
     FLASH_ProgramHalfWord(FlashAddress,FlashData);				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };  
    
  FlashAddress = (EndAddr-FLASH_PAGE_SIZE) + 1024 - 36;     
/*----- WRITE APN NAME ------------------------------------------------------*/
 for(GP_Counter = 0;GP_Counter < 32; GP_Counter++)				// GET 20 BYTES FOR SET APN NAME 
   {
     FlashData = (GPRSData.SET_APN[GP_Counter+1]<<8) | GPRSData.SET_APN[GP_Counter];// GET PASSWORD DATA 
     FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH
     FlashAddress += 2;	GP_Counter++;
   };
   
 /***************************************************************************/
  /*                    DFU BLOCK                                            */
  /***************************************************************************/
 FlashAddress = (EndAddr-FLASH_PAGE_SIZE) + 1024; 
 // FlashAddress = 0x08019000 + 0x400;
 
 /*----- HTTP SERVER IP -------------------------------------------------------*/ 
 FLASH_ProgramHalfWord(FlashAddress,(DFU_HTTP.IP[1] << 8 | DFU_HTTP.IP[0]));FlashAddress += 2; // DFU IP TAB 0,1
 FLASH_ProgramHalfWord(FlashAddress,(DFU_HTTP.IP[3] << 8 | DFU_HTTP.IP[2]));FlashAddress += 2; // DFU IP TAB 2,3
 
 /*----- HTTP PATH -------------------------------------------------------*/
 for(GP_Counter = 0;GP_Counter <= 49; GP_Counter++)				
 {
    FlashData = (DFU_HTTP.PATH[GP_Counter+1] <<8) | DFU_HTTP.PATH[GP_Counter];// FTP SERVER USERNAME
    FLASH_ProgramHalfWord(FlashAddress, FlashData);				     // WRITE IN FLASH 
    FlashAddress += 2;GP_Counter++;
 };
 
 /*----- HTTP FILE --------------------------------------------------------*/
 for(GP_Counter = 0;GP_Counter <= 14; GP_Counter++)				
 {
    FlashData = (DFU_HTTP.FILENAME[GP_Counter+1] <<8) | DFU_HTTP.FILENAME[GP_Counter];// FTP SERVER PASSWORD 
    FLASH_ProgramHalfWord(FlashAddress, FlashData);				     // WRITE IN FLASH 
    FlashAddress += 2;GP_Counter++;
 };
 
 /*----- IAP REQUEST NUMBER DFU NUMBER ---------------------------------------*/
 for(GP_Counter = 0;GP_Counter < 15; GP_Counter++)				// GET 10 BYTES FOR REQUEST MOBILE NUMBER 
   {
    FlashData = (GPRSData.Mobile_No[GP_Counter+1] <<8) | GPRSData.Mobile_No[GP_Counter];// GET MOBILE NUMBER 
    FLASH_ProgramHalfWord(FlashAddress, FlashData );				// WRITE IN FLASH 
    FlashAddress += 2;GP_Counter++;
   };
 /*----- NEXT SOFTWARE VERSION -----------------------------------------------*/
 FlashData = ('-' <<8) | 'V';                                                   // NEW SOFTWARE VERSION 
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 		// WRITE IN FLASH
 FlashData = (SW_VER[3]<<8) | SW_VER[2];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 	
 FlashData = (SW_VER[5]<<8) | SW_VER[4];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2;  	
 FlashData = (SW_VER[7]<<8) | SW_VER[6];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2;
 FlashData = (SW_VER[9]<<8) | SW_VER[8];
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2;

 
 /*----- CURRENT SOFTWARE VERSION --------------------------------------------*/
 FlashData = ('-' <<8) | 'V';                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 		
 FlashData = (SW_Ver[1]<<8) | SW_Ver[0];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 	
 FlashData = (SW_Ver[3]<<8) | SW_Ver[2];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 	
 FlashData = (SW_Ver[5]<<8) | SW_Ver[4];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2;
 FlashData = (SW_Ver[7]<<8) | SW_Ver[6];                                                   
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2;
 FlashData = (SW_Ver[9]<<8) | SW_Ver[8];
 FLASH_ProgramHalfWord(FlashAddress, FlashData );FlashAddress += 2; 
 

 /*----- LOCK FLASH MEMORY ---------------------------------------------------*/
 FLASH_Lock();									// LOCK FLASH MEMORY 
 mSec_Delay(500);
 
 
 /* CHECK FLASH STATUS */
  FlashAddress = EndAddr-FLASH_PAGE_SIZE;
  FlashData = (*(vu16*) FlashAddress);                      
  if(FlashData == 0x00ED)Flag_FlashError = RESET;                               // Signature Byte
 
  }
 }
}

/*******************************************************************************
* Function Name  : Get_HitTime
* Description    : Extract real HIT time as per time zone 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_HitTime(unsigned char Server)
{
  unsigned char TEMP_HH,TEMP_MM,TEMP_SS;
  
  TEMP_HH = RTC_Time.RTC_Hours + 5;
  TEMP_MM = RTC_Time.RTC_Minutes + 30;
    
  if(TEMP_MM > 59){TEMP_MM -= 60;TEMP_HH++;};                                   
  if(TEMP_HH > 23){TEMP_HH -= 24;};
  if(RTC_Time.RTC_Seconds)TEMP_SS = RTC_Time.RTC_Seconds-1;
  else TEMP_SS = RTC_Time.RTC_Seconds;
  
  Last_HitTime[Server][0] = TEMP_HH;                                            // HOURS
  Last_HitTime[Server][1] = TEMP_MM;                                            // MINUTES
  Last_HitTime[Server][2] = TEMP_SS;                                            // SECONDS   
}
/*******************************************************************************
* Function Name  : Key_InputScan
* Description    : Scan the inputs
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Key_InputScan(void)
{
  static FlagStatus
  Flag_IgnON_Chk  = RESET,
  Flag_IgnOFF_Chk = RESET,
  Flag_BD_Chk     = RESET,
  Flag_BR_Chk     = RESET,
  Flag_BTNPressed = RESET;
  
  
  static uint8_t
  Check_IgnONTime  = 0,
  Check_IgnOFFTime = 0,
  Check_BD_Time    = 0,
  Check_BR_Time    = 0;
  
  static uint16_t
  Conn_CheckTime = 0;
  
  uint32_t FlashAddress;
  FlashAddress = 0;
  
  char loc_buffer[50];
  memset(loc_buffer,0,50);
  /**************************************************************************/
  /*              Battery Source sensing                                    */
  /**************************************************************************/
  if(GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET)                        // EXTERNAL 12V FROM VEHICLE BATTERY
  {
    Flag_BD_Chk = RESET;
    Check_BD_Time = 0;
    
    if(Flag_BR_Chk == RESET)
    {
      Check_BR_Time++;
      if(Check_BR_Time > 7)
      {
        Check_BR_Time = 0;
        Flag_BR_Chk = SET;
      }
    }
    else if((GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET) && (Flag_BR_Chk == SET))
    {
      Flag_BR_Chk = RESET; 
      if(Vehicle.Battery[0] != 'C')
      {
        STNDBY_WAITTime = 0;
        Vehicle.Battery[0] = 'C';
        if(Vehicle.Ignition[0] == 'I')Current_Hit_Time = Active_Time;
        else Current_Hit_Time = StandBy_Time;
        if(Protocol == HP_PC)Flag_SMS_RNC = SET;
        Flag_Power_RNC = SET;
        Flag_MAIN_PKT = SET;
        Flag_TER_PKT = SET;
        Gen_PKT();
       }
      Vehicle.Battery[0] = 'C';

      if((GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET) && (Vehicle.EXT_VOLT > (EXT_BATT_END + 2*EXT_PWR_FCTR)))
      {
        REG_LM2576(ENABLE);
        Flag_PowerOff = RESET;
        RTC_WriteBackupRegister(RTC_BKP_DR1,((Frame_Num_TER << 8) | (Flag_PowerOff))); 
      }     
     }
  }
  else                                                                         
  {
    EMG_Button_Press = EMG_BP_FREQ;
    Flag_BR_Chk = RESET;
    Check_BR_Time = 0;
    
    if(Flag_BD_Chk == RESET)
    {
      Check_BD_Time++;
      if(Check_BD_Time > 7)
      {
        Check_BD_Time = 0;
        Flag_BD_Chk = SET;
      }
    }
    else if((GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == SET) && (Flag_BD_Chk == SET))
    {
      Flag_BD_Chk = RESET;
      if(Vehicle.Battery[0] == 'C')
      {
         Vehicle.Battery[0]  = 'B';
         Flag_Remove = SET;
         if(Protocol == HP_PC)Flag_SMS_MBR = SET;
         Flag_MAIN_PKT = SET;
         Flag_TER_PKT = SET;
         Gen_PKT();
         Current_Hit_Time = StopMode_Time;
         STNDBY_WAITTime = Trans_Time;       
      }
      Vehicle.Battery[0] = 'B'; 
     }  
   }
  if(Vehicle.Battery[0] == 'B')PL_OFF;

/******************************************************************************/
  /*               INPUT  SENSING                                               */
  /******************************************************************************/
  if(GPIO_ReadInputDataBit(PORT_DIN1,PIN_DIN1) == RESET)                        // DIN 1 ON ACTIVE LOW
  {
   if((Vehicle.DIN & 0x01) == 0)
   {
     Flag_DIN1 = SET;
     ServerHit_Time = 2;
     Flag_MAIN_PKT = SET;
     Flag_TER_PKT = SET;
     TER_ServerHit_Time = TER_HitTime + 3;
   }
   Vehicle.DIN |= 0x01;                                                           
  }
  else                                                                          // DIN 1 OFF
  {
   if((Vehicle.DIN & 0x01) == 1)
   {
     ServerHit_Time = 2; 
     Flag_TER_PKT = SET;
     Flag_MAIN_PKT = SET;
     TER_ServerHit_Time = TER_HitTime + 3;
   }
   Vehicle.DIN &= 0xFE;
  };
  
  if(GPIO_ReadInputDataBit(PORT_DIN2,PIN_DIN2) == RESET)                        // DIN 2 ON  
  {
   if((Vehicle.DIN & 0x02) == 0)
   {
     Flag_DIN2 = SET; 
     ServerHit_Time = 2;
     Flag_TER_PKT = SET;
     Flag_MAIN_PKT = SET;
     TER_ServerHit_Time = TER_HitTime + 3;
   }
   Vehicle.DIN |= 0x02;                                                    
  }
  else                                                                          // DIN 2 OFF
  {
    if((Vehicle.DIN & 0x02) == 2)
    {
      ServerHit_Time = 2;
      Flag_TER_PKT = SET;
      Flag_MAIN_PKT = SET;
      TER_ServerHit_Time = TER_HitTime + 3;
    }
    Vehicle.DIN &= 0xFD; 
  };
  
  if(GPIO_ReadInputDataBit(PORT_DIN3,PIN_DIN3) == RESET)                        // DIN 3 ON  
  {
     if((Vehicle.DIN & 0x04) == 0)
     {
       Flag_DIN3 = SET; 
       ServerHit_Time = 2;
       Flag_TER_PKT = SET;
       Flag_MAIN_PKT = SET;
       TER_ServerHit_Time = TER_HitTime + 3;
     }
     Vehicle.DIN |= 0x04;                                                    
  }
  else                                                                          // DIN 3 OFF
  {
     if((Vehicle.DIN & 0x04) == 4)
     {
       ServerHit_Time = 2;
       Flag_TER_PKT = SET;
       Flag_MAIN_PKT = SET;
       TER_ServerHit_Time = TER_HitTime + 3;
     }
     Vehicle.DIN &= 0xFB; 
  };

  
  if((Vehicle.Battery[0] == 'C') && (GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET))
  {
    if(PB_ADC > 3500)                                                                      
    {
      Vehicle.DIN &= 0xF7; 
      PL_OFF;
      if(EMG_Button_Press >= 250)
      {
         EMG_Button_Press -= 250;
         if(EMG_Button_Press < 250)
         {
           EMG_Button_Press = 0;
           if(Flag_BTNPressed == RESET)
           {
              Flag_BTNPressed = SET;
              Conn_CheckTime = EMG_WC_Timeout;  
           }      
         }
       }
    }
    else                                                                          // DIN 4 OFF
    {
      Vehicle.DIN |= 0x08; 
      if((EMG_Button_Press == 0) && (Flag_EMRGY_TMP == RESET) && (Flag_BTNPressed == SET))
      {
         EMG_Button_Press = EMG_BP_FREQ;
         if(Flag_Emrgcy == RESET)
         { 
           Flag_DIN4 = SET; 
           ED_Timer = ED_Time;
           Flag_EM_LED = SET;
           SMS_FAllBack_Check = 0;
           Prev_lat = 0.0; Prev_lon = 0.0;
           Flag_Emrgcy = SET;
           
           Save_EMR();
           
           Flag_EA = SET;   
           Flag_MAIN_PKT = SET;
           Flag_TER_PKT = SET;
           ServerHit_Time = 2; 
           TER_ServerHit_Time = TER_HitTime + 3;
           EMRGY_PKTHit_Time = ES_Time; 
           memset(&RAM_BACKUP[EMR][0],0,5120);
           FlashAddress = SPIFLASH_CONFIG;
           memset(loc_buffer,0,50);
           sprintf(loc_buffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],EMRGY_BKP_No);  
            if(Flag_SPIOK == SET)
            {
              SPI_FLASH_SectorErase(FlashAddress);
              SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
            }
            Get_EMRGY_PKT();
        
           }
          Conn_CheckTime = 0; 
      }  
      else
      {
       
       Flag_BTNPressed = RESET;
       
       Flag_EMRGY_TMP = RESET; Conn_CheckTime = 0;
       EMG_Button_Press = EMG_BP_FREQ;
       if(Flag_DT == SET)
       {
         Flag_DT = RESET; 
         Flag_DTC = SET; 
           
         Flag_TER_PKT = SET;
         TER_ServerHit_Time = 2;
        }
      }
    }
  }
  

  /**************************************************************************/
  /*              Ignition Button Sensing                                      */
  /**************************************************************************/
  if(GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == RESET)                              // CHECK IGNITION STATUS -IGNITION OFF 
  {
     Check_IgnONTime = 0;
     Flag_IgnON_Chk  = RESET;
     
     if(Flag_IgnOFF_Chk == RESET)
     {
        Check_IgnOFFTime++;
        if(Check_IgnOFFTime > 2)
        {
          Flag_IgnOFF_Chk = SET;
          Check_IgnOFFTime = 0;
        }
     }
     else if((GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == RESET) && (Flag_IgnOFF_Chk == SET))                          // RECHECK IGNITION STATUS
     {
        Flag_IgnOFF_Chk = RESET;
        if(Vehicle.Ignition[0] != 'F')
        {
          Vehicle.Ignition[0] = 'F';
          Flag_IgnOff = SET;
          Flag_MAIN_PKT = SET;
          Flag_TER_PKT = SET;
          
          sprintf(TCP_ServerData,"IMEI:%s,ICCID:%s,P:%d,%s:%d,S:%d,%s:%d,T:%d,%s:%d,",GPRSData.IMEI_No,GPRSData.ICCID,Flag_MAINServer, &SRVR_IP[MAIN][0],SRVR_Port[MAIN],Flag_EMRServer, &SRVR_IP[EMR][0],SRVR_Port[EMR], Flag_TERServer, &SRVR_IP[TER][0],SRVR_Port[TER]);
          sprintf(GPRS_SendBuffer,"P:%d,O:%s,SP:%d,UR:%d,P:%d,%02hu:%02hu:%02hu,B:%d,S:%d,%02hu:%02hu:%02hu,B:%d,T:%d,%02hu:%02hu:%02hu,B:%d,ODO:%d,",Protocol,GPRSData.Operator,Flag_SIM2,Current_Hit_Time,Flag_TCPConnect[MAIN],Last_HitTime[MAIN][0],Last_HitTime[MAIN][1],Last_HitTime[MAIN][2],TCP_BackCnt[MAIN],Flag_TCPConnect[EMR],Last_HitTime[EMR][0],Last_HitTime[EMR][1],Last_HitTime[EMR][2],EMRGY_BKP_No,Flag_TCPConnect[TER],Last_HitTime[TER][0],Last_HitTime[TER][1],Last_HitTime[TER][2],TCP_BackCnt[TER],KM);
          strcat(TCP_ServerData,GPRS_SendBuffer);
          Get_RAMBackup(TER);
          
          Gen_PKT();
          if(Vehicle.Battery[0] == 'B')Current_Hit_Time = StopMode_Time;
          else Current_Hit_Time = StandBy_Time;
          STNDBY_WAITTime = Trans_Time;
          if(Flag_DSF_ENB == SET){if(Flag_DeepSleep == RESET){DSC = DeepSleepFreq;}}

          if(Flag_IRNSSFix)
          {
            Get_Location();
            if((Flag_ValidGNSSString == SET) && (Flag_OFFGNSS == RESET))
            {
              Flag_OFFGNSS = SET; 
              if(Flag_IRNSSFix)
              {
                memcpy(OFF_LAT,IRNSSData.LAT,strlen(IRNSSData.LAT));
                memcpy(OFF_LON,IRNSSData.LON,strlen(IRNSSData.LON));
              }
            }
          }     
        }
      }
  }
  else                                                                          // IGNITION ON 
  {
     Check_IgnOFFTime = 0;
     Flag_IgnOFF_Chk  = RESET;

     if(Flag_IgnON_Chk == RESET)
     {
       Check_IgnONTime++;
       if(Check_IgnONTime > 2)
       {
         Check_IgnONTime = 0;
         Flag_IgnON_Chk  = SET;
       }
     }
     else if((GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == SET) && (Vehicle.Battery[0] == 'C') && (Flag_IgnON_Chk == SET))                       // RECHECK IGNITION
     {
       Flag_IgnON_Chk = RESET;
       if(Flag_OFFGNSS == SET)
       {
         Flag_OFFGNSS = RESET; 
         memset(OFF_LAT,0,15);
         memset(OFF_LON,0,15);
       }
       if(Vehicle.Ignition[0] != 'I')
        {
          
          Vehicle.Ignition[0] = 'I';
          Flag_IgnOn = SET;
          Flag_MAIN_PKT = SET;
          Flag_TER_PKT = SET;
          Gen_PKT();
          Idle_Time = 0;
          STNDBY_WAITTime = 0;
          REG_LM2576(ENABLE);
          MODULE_L89(ENABLE);
          
          Flag_DeepSleep = RESET;Flag_SleepMode = RESET;
          Flag_DS_RM = RESET;DSC = 0;
          RTC_WriteBackupRegister(RTC_BKP_DR2,((ServerHit_Time << 16) | 'N')); 
          
          /******** EXT BATT CUT OFF CHECH ***********/ 
          Flag_Check_EXT_PWR = SET;
          
          Flag_PowerOff = RESET;
          Flag_EXT_Batt_Cut = RESET;
          
          CHRG_Delay = 100;
          RTC_WriteBackupRegister(RTC_BKP_DR1,((Frame_Num_TER << 8) | (Flag_PowerOff))); 
        } 
      
       if(Flag_TOW == SET)Flag_TOW = RESET;
     }
   }
 
  /* VEHICLE STATUS CHECK */ 
 
  if(((RTC_ReadBackupRegister(RTC_BKP_DR2) & 0x0000FFFF) == 'S') || (Flag_DeepSleep == SET))
    Current_Hit_Time = StopMode_Time;
  else if(Vehicle.Ignition[0] == 'I')
    Current_Hit_Time = Active_Time;
    
  if(Conn_CheckTime >= 250)
  {
    Conn_CheckTime -= 250;
    if(Conn_CheckTime < 250)
    {
      Conn_CheckTime = 0;
      Vehicle.DIN &= 0xF7;
      Flag_EWT = SET; 
      if(Protocol == HP_PC)Flag_SMS_DT = SET;
      Flag_EMRGY_TMP = SET;        
      Flag_EM_LED = RESET; 
      PL_OFF;   
      
      Flag_MAIN_PKT = SET;
      Flag_TER_PKT = SET;
      ServerHit_Time = 2;
      TER_ServerHit_Time = TER_HitTime + 3;
      
    }
  }
  
  
if(Vehicle.Ignition[0] == 'I') CHRG_ON;
else CHRG_OFF;
  
  

}
/*******************************************************************************
* Function Name  : Serial_PutString_232
* Description    : Print a string on the GPRS Modem
* Input          : A String till null character
* Output         : None
* Return         : None
*******************************************************************************/
void Serial_PutString_232(char *s, uint8_t usart_no)                                              // THIS FUNCTION SEND STRING'S CHARACTER TILL NULL CHARACTER
{  
  mSec_Delay(10);
  if(usart_no == 3) 
  {
     while (*s != '\0')                                                            // PUT STRING OVER USART TILL NULL CHARACTER 
      {
        while (USART_GetFlagStatus(COM_USART, USART_FLAG_TXE) == RESET);               // WAIT UNTILL LAST TRANSMISSION COMPLETES
        COM_USART->TDR = (*s & (uint16_t)0x01FF);                                    // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
        s++;                                                                        // INCREMENT POINTER
        mSec_Delay(2);
      };
       mSec_Delay(5);
   
      while (USART_GetFlagStatus(COM_USART, USART_FLAG_TXE) == RESET);	                // WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
      COM_USART->TDR = (10 & (uint16_t)0x01FF);					// SEND DATA BYTE 13 
  }
  else if(usart_no == 6)
  {
      while (*s != '\0')                                                            // PUT STRING OVER USART TILL NULL CHARACTER 
      {
        while (USART_GetFlagStatus(COM2_USART, USART_FLAG_TXE) == RESET);               // WAIT UNTILL LAST TRANSMISSION COMPLETES
        COM2_USART->TDR = (*s & (uint16_t)0x01FF);                                    // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
        s++;                                                                        // INCREMENT POINTER
        mSec_Delay(2);
      };
       mSec_Delay(5);
   
      while (USART_GetFlagStatus(COM2_USART, USART_FLAG_TXE) == RESET);	                // WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
      COM2_USART->TDR = (10 & (uint16_t)0x01FF);					// SEND DATA BYTE 13 
  }
  else __NOP();
 
}
/*******************************************************************************
* Function Name : Serial_SendData_232
* Description   : Send Data to 232
* Input         : one hex data only
* Output        : None
* Return        : None
*******************************************************************************/
void Serial_SendData_232(char SerialData)					// THIS FUNCTION SEND ONLY ONE HEX VALUE AT A TIME	
 {
   while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);		// WAIT FOR COMPLETE TRANMISSION TILL FLAG IS 0 
     
   COM_USART->TDR = (SerialData & (uint16_t)0x01FF);                             // WRITE IN DATA REGISTER OF GPRS FOR SENDING NEXT DATA
 }
/*******************************************************************************
* Function Name  : ftoa
* Description    : float to char array string
* Input          : float, string, number of char after & before decimal
* Output         : None
* Return         : None
*******************************************************************************/
void ftoa(float num, char *buffer, int befor_point, int after_point)
{
   int buffer_size,GP_Counter,value;
   long int_part,temp;
   buffer_size = befor_point+after_point;
   buffer[buffer_size+1] = '\0';
   int_part = (int)num;
   num = num - int_part;
   num = num*pow(10,after_point);
   for(GP_Counter=1;GP_Counter<=after_point;GP_Counter++)
   {
   	temp = (int)pow(10,GP_Counter);
	value = (int)num%temp;
        temp = temp/10;
        value = value/temp;
        value = value + 48;
   	buffer[buffer_size] = value;
   	buffer_size--;
   }
   
   buffer[buffer_size] = '.';
   
   for(GP_Counter=1;GP_Counter<=befor_point;GP_Counter++)
   {
   	buffer_size--;
   	temp = (int)pow(10,GP_Counter);
	value = int_part%temp;
        temp = temp/10;
        value = value/temp;
        value = value + 48;
   	buffer[buffer_size] = value;
   }
}



/*******************************************************************************
* Function Name  : REG_LM2576
* Description    : ENABLE/DISABLE REG_LM2576 TO POWERED ON DEVICE FROM EXTERNAL VEHICLE BATTERY  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void REG_LM2576(uint8_t state)
{
//  GPIO_InitTypeDef        GPIO_InitStructure;
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
//  
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_Pin = PIN_DEVICE_ON; 
//  GPIO_Init(PORT_DEVICE_ON, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  
  switch(state)
  {
    case ENABLE       :  PWR_ENB; EBL_Count = 0;  break;                                   /* ENABLE LM2576 REGULATOR  */
    case DISABLE      :  // PWR_DSB;  break;       // CUT OFF DISABLED                                           /* DISABLE LM2576 REGULATOR */
    default           :  PWR_ENB; EBL_Count = 0;  break;
  }
}
/*******************************************************************************
* Function Name  : Write_Oddo
* Description    : Save ODOMETER value into SPI Flash
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Write_Oddo(void)
{
  uint32_t FlashAddress;
  FlashAddress = 0;
  char loc_buffer[70];
    
  memset(loc_buffer,0,70);
  FlashAddress = SPIFLASH_CONFIG_ODO;
  sprintf(loc_buffer,"KM-%u,MTR-%hu,Lat-%s,Lon-%s",KM,MTR,Last_Lat,Last_Lon);  
  if(Flag_SPIOK == SET)
    {
     SPI_FLASH_SectorErase(FlashAddress);
     SPI_FLASH_PageWrite(loc_buffer,FlashAddress,strlen(loc_buffer));
    }
}
/*******************************************************************************
* Function Name : Extract Value
* Description   : Extract Parameter Value form String with Place
* Input         : Parameter name & place
* Output        : Parameter new value
* Return        : Paremeter
*******************************************************************************/
//uint32_t Extract_Value(char * TempPtr, uint8_t Place)
//{
//unsigned int Param;
//char Temp_Val;
//  
//TempPtr = TempPtr + Place; Temp_Val=*TempPtr; *TempPtr = 0; TempPtr-=Place;
//
//Param = atol(TempPtr);                                                          //CONVERT STRING INTO LONG DATA TYPE
//
//TempPtr += Place;*TempPtr = Temp_Val;
//
//return (Param);
//}

/*******************************************************************************
* Function Name  : Test_Mode
* Description    : Test_Mode of Hardware
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Test_Mode(void)
{
  uint8_t Recheck_Time = 0;

  //char Time_AT = 60;
  uint8_t TO_LDRC = 0;
  char Test_Buffer[300];
  FlagStatus
  Flag_AIN = RESET,
  Flag_PWR = RESET,
  Flag_GPRS = RESET;
  char *Ptr,buffer[20],*DPtr;
//  unsigned char Sim_Wait = 0;
  unsigned char SIM_REG_Err = 0,TO = 0; //250
  unsigned short TimeOut = 300;
  uint16_t GP_Counter;
  uint32_t FlashAddress;
  
  FlashAddress = 0;
  GP_Counter = 0;
  Vehicle.DIN = 0;
  Vehicle.Battery[0] = 0;
  Vehicle.Ignition[0] = 0;
  Vehicle.AIN1 = 0.0;
  Vehicle.AIN2 = 0.0;
  Vehicle.EXT_VOLT = 0.0;
  Vehicle.INT_VOLT = 0.0;
  
  DPtr = Ptr = NULL;
  MODULE_L89(DISABLE);
  memset(Test_Buffer,0,300);
  Vehicle.Ignition[0] = 'F';
  
  WLED_ON;
  CHRG_ON;
    
  Flag_ModemRdy = SET;
  Flag_ModemOK = RESET;
  Modem_PutString("AT"); WAIT_MODEM_RESP(2);
  Modem_PutString("AT"); WAIT_MODEM_RESP(2);
  if(Flag_ModemOK == RESET){Modem_PutString("AT"); WAIT_MODEM_RESP(2);}
  if(Flag_ModemOK == RESET){Modem_PutString("AT"); WAIT_MODEM_RESP(2);}
  if(Flag_ModemOK == RESET){Modem_PutString("AT"); WAIT_MODEM_RESP(2);}
  
  
  MODULE_L89(ENABLE);
  
//  Flag_ReadData = SET;
//  Modem_PutString("AT+QSTK?");WAIT_MODEM_RESP(10);  
//  if(strstr(GPRS_Buffer,"STK: 0"))
//  {
//   Modem_PutString("AT+QSTK=1");WAIT_MODEM_RESP(20); 
//   MODULE_M66_RST();
//   
//   Modem_PutString("AT"); WAIT_MODEM_RESP(5);
//   Modem_PutString("AT"); WAIT_MODEM_RESP(5);
//  }
//  Flag_ReadData = RESET;
//   
  
//
//  Sim_Wait = 10;
//  while(Sim_Wait) 
//  {
//    if(Flag_Second)
//    {
//      Flag_Second = RESET;
//      PL_TGL;
//      Sim_Wait--;
//      IWDG_ReloadCounter();
//    }
//  }
//  
  /*************************** CHECK PROFILE ********************************/
      
  GPRS_Buffer[15] = 0;
  GPRS_Buffer[16] = 0;
  GPRS_Buffer[498] = 0;
  GPRS_Buffer[499] = 0;
  GPRS_Count = 0;
//  Modem_PutString("AT+QSPN?");WAIT_MODEM_RESP(5);
//  if((Flag_ModemOK == SET) && (strstr((char*)GPRS_Buffer,"BSNL")) || (strstr((char*)GPRS_Buffer," F")))                     
//  {
//     Swap_OPS(); Sim_Wait = 60;
//  };
 // Modem_PutString("AT+CFUN=1,1");WAIT_MODEM_RESP(10);    
  Flag_ModemRdy = SET;
//  Modem_PutString("AT"); WAIT_MODEM_RESP(5);
//  Modem_PutString("AT"); WAIT_MODEM_RESP(5);
//      
//  while(Sim_Wait) 
//  {
//    if(Flag_Second)
//    {
//      Flag_Second = RESET;
//      PL_TGL;
//      Sim_Wait--;
//      IWDG_ReloadCounter();
//    }
//  }

  
  Flag_DIN = SET; // TO CHECK DIGITAL INPUT @ FIRST
  TimeOut = 360;
  while(TimeOut)
  {
    
    if(Flag_Second)
    {
      Flag_Second = RESET;
      PL_TGL;
      if(Recheck_Time < 20){Recheck_Time++; if(Recheck_Time > 15){Recheck_Time = 0;Flag_DIN = SET;}} 
      if(Flag_CheckLDR1)
      {
        TO_LDRC++;
        if(TO_LDRC > 10)
        {
          TO_LDRC = 0; 
          Flag_CheckLDR1 = RESET; 
          Flag_CheckLDR2 = RESET; 
          Flag_GPRS = SET; 
          strcat(Test_Buffer,"TAMPER-FAIL,");
        }
      }
      if(Flag_CheckLDR2)
      {
        TO++;
        if(TO > 10){Flag_CheckLDR2 = RESET; Flag_GPRS = SET; strcat(Test_Buffer,"TAMPER-FAIL,");}
      }
      
      TimeOut--; 
      IWDG_ReloadCounter();
    }    
    /*---------- CHECK DIN ---------------------------------------------*/
    if(Flag_DIN)
    {
      Flag_DIN = RESET;
      Serial_PutString_232("TEST_DIN",3);
    }
    /*---------- CHECK DOUT ---------------------------------------------*/
    else if(Flag_DO)
    {
     Flag_DO = RESET;
     
     OP1_START;  OP2_START; mSec_Delay(1000);  Serial_PutString_232("DOUT_12_ON",3); mSec_Delay(500); 
     OP1_STOP;   OP2_STOP;  mSec_Delay(1000);  Serial_PutString_232("DOUT_12_OFF",3);mSec_Delay(500);
     
     Flag_AIN = SET;
    }
    /*---------- CHECK AIN ---------------------------------------------*/
    else if(Flag_AIN)
    {
      CHRG_ON;
      
      LDR_Volt         = ADCConvertedValue[1];
      Vehicle.INT_VOLT = (ADCConvertedValue[5])*0.001596+0.04;//0.001611; // INTERNAL BATTERY VOLATAGE (5 CYCLES AVERAGE VALUE)
      Vehicle.AIN1     = (ADCConvertedValue[2])*0.0088+0.0513;
      Vehicle.AIN2     = (ADCConvertedValue[0])*0.0088+0.1513;
      Vehicle.EXT_VOLT = (ADCConvertedValue[4])*0.01352;//y = 0.025x  EXT. BATTERY VOLTAGE (5 CYCLES AVERAGE VALUE)

        
      Flag_AIN = RESET;
      if(Vehicle.AIN1 > 4.5 && Vehicle.AIN1 < 7.5)strcat(Test_Buffer,"AIN1-OK,"); else strcat(Test_Buffer,"AIN1-FAIL,"); 
      if(Vehicle.AIN2 > 4.5 && Vehicle.AIN2 < 7.5)strcat(Test_Buffer,"AIN2-OK,"); else strcat(Test_Buffer,"AIN2-FAIL,"); 
      if(Vehicle.EXT_VOLT > 8.5 && Vehicle.EXT_VOLT < 14.5)strcat(Test_Buffer,"EBV-OK,"); else strcat(Test_Buffer,"EBV-FAIL,"); 
      if(Vehicle.INT_VOLT > 2.3 && Vehicle.INT_VOLT < 4.3)strcat(Test_Buffer,"CHARGING IC-OK,"); else strcat(Test_Buffer,"CHARGING IC-FAIL,"); 
    
      mSec_Delay(500);
      Flag_PWR = SET;
    }
    else if(Flag_PWR)
    {
      Flag_PWR = RESET;
      Serial_PutString_232("EP_TEST",3);
    }
    else if(Flag_LDR)
    {
      Flag_LDR = RESET;
      Serial_PutString_232("TEST_LDR",3);
    }
    else if(Flag_CheckLDR1)
    {
      mSec_Delay(250);
      if(ADCConvertedValue[0] < 2000) {Flag_CheckLDR1 = RESET; Flag_CheckLDR2 = SET; Serial_PutString_232("LDR COVERED",3);}
    }
    else if(Flag_CheckLDR2)
    {
      mSec_Delay(250);
      if(ADCConvertedValue[0] > 3000)
      {
        Flag_CheckLDR2 = RESET; Flag_GPRS = SET; strcat(Test_Buffer,"TAMPER-OK,");
      
        // Turn on Ignition Manually for Indication
         GPIO_InitTypeDef    GPIO_InitStructure;
         RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
         GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
         GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
         GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
         GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
         GPIO_InitStructure.GPIO_Pin = PIN_IGN;
         GPIO_Init(PORT_IGN, &GPIO_InitStructure);
         PORT_IGN->BRR = PIN_IGN;
      }   
    }
    else if(Flag_GPRS)
    {
      //mSec_Delay(500);mSec_Delay(500);mSec_Delay(500);mSec_Delay(500);mSec_Delay(500);mSec_Delay(500);
      Serial_PutString_232("SIM TESTING",3);
      Flag_GPRS = RESET;
      GPRS_Buffer[25] = 0;
      GPRS_Buffer[26] = 0;
      GPRS_Buffer[498] = 0;
      GPRS_Buffer[499] = 0;
      GPRS_Count = 0;
      sprintf(GPRSData.IMEI_No,"N.A.00000000000");                                   // CLEAR IMEI NUMBER 
      Flag_WrongIMEI = SET;
      Modem_PutString("AT+GSN");WAIT_MODEM_RESP(10);                                 // REQUEST FOR IMEI NUMBER   
      if(Flag_ModemOK == SET && strstr((char*)GPRS_Buffer,"GSN"))                           
      {
       Ptr = strstr((char*)GPRS_Buffer,"GSN") + 5;                             // SET POINTER TO READ IMEI NUMBER 
       for(GP_Counter = 0;GP_Counter <=14; GP_Counter++,Ptr++)                 // EXTRACT 15 DIGIT IMEI NUMBER 
         {
          if(*Ptr < '0' || *Ptr > '9')Flag_WrongIMEI = SET;                 // IF CHAR OTHER THAN NUMERIC IT MEANS WRONG IMEI NUMBER 
          GPRSData.IMEI_No[GP_Counter] = *Ptr;                                 // SAVE IMEI NUMBER                               
         };
       GPRSData.IMEI_No[GP_Counter] = 0;                                          // SET LAST LOCATION AS 0
       Flag_WrongIMEI = RESET;
       }
      else Flag_WrongIMEI = SET;                                                     // IF NOT GETTING CORRECT IMEI NUMBER  

     Modem_PutString("AT+CCID");WAIT_MODEM_RESP(2); 
     if(Flag_ModemOK)
     {
       Modem_PutString("AT+CCID");WAIT_MODEM_RESP(2); 
       if(Flag_ModemOK)
       {
         if(DPtr = strstr((char*)GPRS_Buffer,"ID: "))
         {
           DPtr += 4;
           GPRSData.SIM_Present[0] = '1';                                             // MODEM RESPONDING i.e. SIM IS PRESENT
           for(GP_Counter = 0;GP_Counter <=19; GP_Counter++,DPtr++)                    // EXTRACT 15 DIGIT IMEI NUMBER 
             GPRSData.ICCID[GP_Counter] = *DPtr;                                       // SAVE IMEI NUMBER                               
          }
         Flag_ModemOK = RESET; 
       }
      CHECK_SIGNAL :
     
      GPRS_Buffer[15] = 0;
      GPRS_Buffer[16] = 0;
      GPRS_Buffer[498] = 0;
      GPRS_Buffer[499] = 0;
      GPRS_Count = 0;
      Modem_PutString("AT+CSQ");WAIT_MODEM_RESP(2);                            // CHECK SIGNAL STRENGTH 

      if((Ptr = strstr((char*)GPRS_Buffer,"Q:")) && (Flag_ModemOK == SET))
      {
         Flag_ModemOK = RESET;
         Ptr += 3;GPRSData.Signal_Strength = atoi(Ptr);                     // EXTRACT SIGNAL STRENGTH
         if(GPRSData.Signal_Strength > 31 )GPRSData.Signal_Strength = 0;          // SET SIGNAL STRENGTH TO 0 IF INVALID/GARBAGE
         if(GPRSData.Signal_Strength >= VALID_SIGNAL )                            // CHECK AUTHENTICATION OF SIGNAL STRENGTH
         {
            GPRS_Buffer[15] = 0;
            GPRS_Buffer[16] = 0;
            GPRS_Buffer[498] = 0;
            GPRS_Buffer[499] = 0;
            GPRS_Count = 0;
           Modem_PutString("AT+CGREG?");WAIT_MODEM_RESP(2);
         } 
         
       }
      else Flag_ModemOK = RESET;
      
    
     if(Ptr =  strstr((char*)GPRS_Buffer,"Q:")){*Ptr = ' ';Ptr++;*Ptr = ' ';};// INSERT BLANK 
     if(Flag_ModemOK)
     {
       Flag_ModemOK = RESET;
       Ptr = strstr((char*)GPRS_Buffer,"EG: ");
       if(Ptr)
       {
         Ptr += 6;
         if((*Ptr == '1') || (*Ptr == '5'))Flag_ValidNetwork = SET;
       }
       else Flag_ValidNetwork = RESET;
     }
     
     if(GPRSData.Signal_Strength > 10)
     {
     SIM_REG_Err = 0;      
     }
   else {mSec_Delay(500); SIM_REG_Err++; if(SIM_REG_Err < 60)goto CHECK_SIGNAL;}
     
     // Network will not register
//     if((GPRSData.Signal_Strength > 10) && (Flag_ValidNetwork == SET))
//     {
//       SIM_REG_Err = 0;      
//     }
//     else {mSec_Delay(500); SIM_REG_Err++; if(SIM_REG_Err < 60)goto CHECK_SIGNAL;}
   
     strcat(Test_Buffer,"ICCID-");strcat(Test_Buffer,GPRSData.ICCID);strcat(Test_Buffer,",");
     if(Flag_ValidNetwork)strcat(Test_Buffer,"SIM-OK,"); else strcat(Test_Buffer,"SIM-FAIL,");
     if(Flag_WrongIMEI)strcat(Test_Buffer,"GPRS-FAIL,"); else strcat(Test_Buffer,"GPRS-OK,");
     memset(buffer,0,20);
     sprintf(buffer,"SIG-%hu,",GPRSData.Signal_Strength);
     
     strcat(Test_Buffer,buffer);
     
     MEMS_INIT();
     mSec_Delay(1000);
     
     //Serial_PutString_232("EP_TEST",3);
     int TTFF_Time = 120;
     while(TTFF_Time > 0)
     {
       mSec_Delay(1000);
       IWDG_ReloadCounter();	
       TTFF_Time--;
       if(Rcv_IR_RMC[18] == 'A' || Rcv_IR_RMC[18] == 'D'){ if(TTFF_Time > 5)TTFF_Time = 5; PPS_ON; }
     }
     if((Rcv_IR_RMC[18] == 'A') ||  (Rcv_IR_RMC[18] == 'D'))strcat(Test_Buffer,"GNSS-OK,");else strcat(Test_Buffer,"GNSS-FAIL,");
     
     
     
     
     if(Flag_MEMSOK)strcat(Test_Buffer,"MEMS-OK,");else strcat(Test_Buffer,"MEMS-FAIL,");
   
     FlashAddress = SPI_FLASH_ReadID();                                            // GET FLASH STATUS WORKING 
     if(FlashAddress == 0xEF4018 || FlashAddress == 0xEF4017 || FlashAddress == 0x852018 || FlashAddress == 0xB4018)Flag_SPIOK = SET;
     else Flag_SPIOK = RESET;
     
     if(Flag_SPIOK == SET)strcat(Test_Buffer,"MMRY-OK,");else strcat(Test_Buffer,"MMRY-FAIL,");
     if((Vehicle.DIN & 0x01) == 0x01)strcat(Test_Buffer,"DIN1-OK,");else strcat(Test_Buffer,"DIN1-FAIL,");
     if((Vehicle.DIN & 0x02) == 0x02)strcat(Test_Buffer,"DIN2-OK,");else strcat(Test_Buffer,"DIN2-FAIL,");
     if((Vehicle.DIN & 0x04) == 0x04)strcat(Test_Buffer,"DIN3-OK,");else strcat(Test_Buffer,"DIN3-FAIL,");
     if((Vehicle.DIN & 0x08) == 0x08)strcat(Test_Buffer,"DIN4-OK,");else strcat(Test_Buffer,"DIN4-FAIL,");
      
     if(Vehicle.Ignition[0] == 'I')strcat(Test_Buffer,"IGN-OK,");else strcat(Test_Buffer,"IGN-FAIL,");
     if(Flag_EP)strcat(Test_Buffer,"EP-OK,");else strcat(Test_Buffer,"EP-FAIL,");
     //Serial_PutString_232("EP_TEST",3);  
     strcat(Test_Buffer,"IMEI-");
     strcat(Test_Buffer,GPRSData.IMEI_No);strcat(Test_Buffer,",SW-");
     strcat(Test_Buffer,CLIENT_ID);
     strcat(Test_Buffer,FW_VER);
     char state[10] = {0};
      for(int n = 0; n < 10; n++)
      {
        if(SRVR_IP[MAIN][n] == '.')break;
        state[n] = SRVR_IP[MAIN][n];
      }
     strcat(Test_Buffer,":");
     strcat(Test_Buffer,state);
     strcat(Test_Buffer,",Dt-");
      
     GPRS_Buffer[15] = 0;
     GPRS_Buffer[16] = 0;
     GPRS_Buffer[498] = 0;
     GPRS_Buffer[499] = 0;
     GPRS_Count = 0;
     Modem_PutString("AT+QSTK=1");WAIT_MODEM_RESP(2);                      // TO ENABLE STK BY DEFAULT 
//     Modem_PutString("AT+CCLK?"); WAIT_MODEM_RESP(5);
//     // Modem_PutString("AT+QLTS"); WAIT_MODEM_RESP(5);
//     //Modem_PutString("AT+QGSMLOC=2"); WAIT_MODEM_RESP(50);
//     if(Flag_ModemOK == SET)
//     { 
//         if(Ptr = strstr((char*)GPRS_Buffer,"CLK:"))                           // SET POINTER TO LOC FOR ITS ADDRESS
//        // if(Ptr = strstr((char*)GPRS_Buffer,"LTS:"))  
//          {
//         Ptr+=6;                                                                 // SET POINTER TO IDENTITY SUCCESS 
//         
//         /*----- EXTRACT CELL DATE -----------------------------------------------*/
//          GPRSData.Cell_Date[6] = *Ptr;Ptr++;                                 /********************************/
//          GPRSData.Cell_Date[7] = *Ptr;Ptr += 2;                                 /*      GPRS CELL DATE FORMATE  */
//          GPRSData.Cell_Date[2] = *Ptr;Ptr++;                                 /*      YYYY/MM/DD              */
//          GPRSData.Cell_Date[3] = *Ptr;Ptr += 2;                                // HERE POINTER +1 JUMP FOR '/' CHARACTER  
//          GPRSData.Cell_Date[0] = *Ptr;Ptr++;                                 
//          GPRSData.Cell_Date[1] = *Ptr;Ptr+=2;                                // HERE POINTER +1 JUMP FOR '/' CHARACTER 
//          GPRSData.Cell_Date[4] = '2';                                 
//          GPRSData.Cell_Date[5] = '0';  
//
//           };                                                                          
//       };  
     
      strcat(Test_Buffer,Date); //strcat(Test_Buffer,",");
      
      strcat(Test_Buffer,state);
      Serial_PutString_232("RESULT-",3);
      Serial_PutString_232(Test_Buffer,3);
      TimeOut = 0;
      mSec_Delay(500);
     }   
  }
  
  }
  CONFIG_OB();
  SYSTEM_SW_RESET;
}
/*****************************************************************************/
void Send_IMEI(void)
{
  
  GPIO_InitTypeDef    GPIO_InitStructure;
  
  FlagStatus Flag_GPRS = RESET;
  char *Ptr,timer = 60,Timeout;
  char IMEI_Buffer[25];
  uint16_t GP_Counter;
  
  GP_Counter = 0;  
  Ptr = NULL;
//  GNSS_DSB;
//  GPRS_DSB;
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = PIN_IGN;
  GPIO_Init(PORT_IGN, &GPIO_InitStructure);
  PORT_IGN -> BRR = PIN_IGN;
  
  
  CHRG_ON; 
  mSec_Delay(1000);
  IWDG_ReloadCounter(); 
  Vehicle.Ignition[0] = 'I';                                                    // TO GLOW IGN LED 
  PL_ON;                                                                        // TO GLOW PANIC LED
  GNSS_ENB;                                                                     // TO BLINK GPS LED
  GPRS_ON;

  mSec_Delay(1000);
  IWDG_ReloadCounter(); 
  
  
  
 if(GPRSData.IMEI_No[0] != 'N' && GPRSData.IMEI_No[0] != 0)
 {
    memset(IMEI_Buffer,0,25);
    sprintf(IMEI_Buffer,"IMEI-%s,V-%4.2f",GPRSData.IMEI_No,Vehicle.INT_VOLT);
    Serial_PutString_232(IMEI_Buffer,3);
 }
 else 
 {
    //GPRS_OFF;
    GPRS_Buffer[15] = 0;
    GPRS_Buffer[16] = 0;
    GPRS_Buffer[498] = 0;
    GPRS_Buffer[499] = 0;
    GPRS_Count = 0;
    mSec_Delay(200);
    
    GPRS_ON;
    mSec_Delay(1000);
    IWDG_ReloadCounter(); 
    
    timer = 60;
    while(Flag_GPRS == RESET)
    {
    Modem_PutString("AT"); WAIT_MODEM_RESP(2);
    if(Flag_ModemOK == SET)Flag_GPRS = SET;
    
    if(Flag_Second)
      {
        Flag_Second = RESET;
        IWDG_ReloadCounter(); 
        if(timer)timer--;
        else break;
      }
    }
    
    mSec_Delay(1000);

    
    if(Flag_GPRS)
      {
        Flag_GPRS = RESET;
        GPRS_Buffer[15] = 0;
        GPRS_Buffer[16] = 0;
        GPRS_Buffer[498] = 0;
        GPRS_Buffer[499] = 0;
        GPRS_Count = 0;
        sprintf(GPRSData.IMEI_No,"N.A.00000000000");                                   // CLEAR IMEI NUMBER 
        Flag_ModemRdy = SET;
        GPRSData.SIM_Present[0] = '1';                                                 // MODEM RESPONDING i.e. SIM IS PRESENT
         Timeout = 5;
         Flag_WrongIMEI = SET; 
         GPRS_Buffer[15] = 0;
         GPRS_Buffer[16] = 0;
         GPRS_Buffer[498] = 0;
         GPRS_Buffer[499] = 0;
         GPRS_Count = 0;
         Modem_PutString("AT+GSN");//WAIT_MODEM_RESP(5);                                 // REQUEST FOR IMEI NUMBER   
         while(Timeout)
         {
            if(strstr((char*)GPRS_Buffer,"OK"))                           
              {
               Ptr = strstr((char*)GPRS_Buffer,"GSN") + 5;                             // SET POINTER TO READ IMEI NUMBER 
               for(GP_Counter = 0;GP_Counter <=14; GP_Counter++,Ptr++)                 // EXTRACT 15 DIGIT IMEI NUMBER 
                 {
                   if(*Ptr < '0' || *Ptr > '9'){Flag_WrongIMEI = SET; break;}                // IF CHAR OTHER THAN NUMERIC IT MEANS WRONG IMEI NUMBER 
                  GPRSData.IMEI_No[GP_Counter] = *Ptr;                                 // SAVE IMEI NUMBER                               
                 };
               GPRSData.IMEI_No[GP_Counter] = 0;                                          // SET LAST LOCATION AS 0
               Flag_WrongIMEI = RESET; 
               break;
              }
            if(Flag_Second == SET)
             {
               Flag_Second = RESET; Timeout--;IWDG_ReloadCounter();               // DECREMENT UPLOAD TIME 
               if(GNSS_Time <= 1 )GNSS_Time = GNSS_INTERVAL;
             };
            
         }
        Flag_WrongIMEI = RESET;
        memset(IMEI_Buffer,0,25);
        sprintf(IMEI_Buffer,"IMEI-%s,V-%4.2f",GPRSData.IMEI_No,Vehicle.INT_VOLT);
        Serial_PutString_232(IMEI_Buffer,3);
        mSec_Delay(100);
        Serial_PutString_232(IMEI_Buffer,3);
        mSec_Delay(100);
        Serial_PutString_232(IMEI_Buffer,3);
        mSec_Delay(100);

      } 
      else 
      {
        Serial_PutString_232("IMEI NOT FOUND",3);
      }
       
 }
 IWDG_ReloadCounter(); 
 mSec_Delay(1000);
 IWDG_ReloadCounter(); 
 mSec_Delay(1000);
 
 SYSTEM_SW_RESET;
}

