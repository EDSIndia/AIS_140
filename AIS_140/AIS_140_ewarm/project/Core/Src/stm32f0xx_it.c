/**
  ******************************************************************************
  * @file    stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    16-January-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
    
uint32_t IC1ReadValue1 = 0, IC1ReadValue2 =0;

/** @addtogroup STM32F0xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup DMA_FLASHRAMTransfer
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    RCC_ClearITPendingBit(RCC_IT_CSS);
    if(Flag_SPIOK == SET)
    {
      TAKE_FUL_BKP();
    }
    RTC_WriteBackupRegister(RTC_BKP_DR4,(((RTC_Time.RTC_Hours*3600)+(RTC_Time.RTC_Minutes*60)+RTC_Time.RTC_Seconds) << 8) | 'T');
    SYSTEM_SW_RESET; 
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
    if(Flag_SPIOK == SET)
    {
       TAKE_FUL_BKP();
    }
    RTC_WriteBackupRegister(RTC_BKP_DR4,(((RTC_Time.RTC_Hours*3600)+(RTC_Time.RTC_Minutes*60)+RTC_Time.RTC_Seconds) << 8) | 'T');
    SYSTEM_SW_RESET; 
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
    
void SysTick_Handler(void)
{
 
 
 Flag_mSec = SET;                                                               // SET MILI SECOND FLAG

  if(Flag_MEMS_ENB == SET)
  {
    if((Flag_SleepMode == RESET) && (Flag_MEMSOK == SET) && (Flag_SelfTest == RESET) && (HA_HB == 0))
    {
      GET_MEMS++;
      if(GET_MEMS > 9) 
      { 
        GET_MEMS = 0;
        GET_MEMSData();
        ACC++; X_AVG += x_g;
        Y_AVG += y_g; Z_AVG += z_g;
         
        if(ACC > 9)
        {
          ACC = 0; X_avg = (X_AVG/10.0);
          Y_avg = (Y_AVG/10.0);  Z_avg = (Z_AVG/10.0);
          X_AVG = 0; Y_AVG = 0; Z_AVG = 0;
        }
      }
    }
  }

  if((Sec_Tick % 250)==0)Flag_KeyScan = SET;
  if((Sec_Tick % 2)==0)Flag_ReadADC = SET; 
  
  if(Flag_InitLED == RESET)
  {
     // LED Indication
     if(Vehicle.Battery[0] == 'C')
     {
       // GPS LED
       if(Flag_IRNSSFix)
       {
          if((RTC_Time.RTC_Seconds & 0x01) && Sec_Tick ==  99)PPS_ON;
          else if(Sec_Tick == 999)PPS_OFF;
       }
       else PPS_OFF;
       
       // GSM LED
       if(GPRSData.IMEI_No[0] != '8') // Boot Up
       {
         if ((RTC_Time.RTC_Seconds % 4) == 0 && Sec_Tick == 999)NL_TGL;
       }
       else
       {
         if(Flag_TCPConnect[0] == SET)
         {
           if ((Sec_Tick % 100) == 0)NL_TGL; // Connected to server
         }
         else if(Flag_APNSet) // SIM OK
         {
           if(Sec_Tick == 999)NL_TGL;
         }
         else if((RTC_Time.RTC_Seconds % 2) == 0 && Sec_Tick == 999)NL_TGL; // GSM Working      
       }
     
       // Diagnostic LED
       if(Flag_MEMSOK == RESET || Flag_SPIOK == RESET)WLED_ON;
       else
       {
          if(Sec_Tick == 99 && Flag_TCPConnect[1] == SET)WLED_ON;
          else if(Sec_Tick == 999)WLED_OFF;
       }
     }
     else
     {
       // GPS LED
       if(Flag_IRNSSFix)
       {
          if(Sec_Tick ==  99 && (RTC_Time.RTC_Seconds % 4 == 0))PPS_ON;
          else if(Sec_Tick == 999)PPS_OFF; 
       }
       else PPS_OFF;
       
       // GSM LED
       if(GPRSData.IMEI_No[0] != '8') // Boot Up
       {
         if ((RTC_Time.RTC_Seconds % 4) == 0 && Sec_Tick == 99)NL_ON;
         if (Sec_Tick == 999) NL_OFF;
       }
       else
       {
         if(Flag_TCPConnect[0] == SET)
         {
            if ((RTC_Time.RTC_Seconds % 8) == 0 && Sec_Tick == 99)NL_ON;
            if (Sec_Tick == 999) NL_OFF;
         }
         else if(Flag_APNSet) // SIM OK
         {
            if ((RTC_Time.RTC_Seconds % 4) == 0 && Sec_Tick == 99)NL_ON;
            if (Sec_Tick == 999) NL_OFF;
         }
          
       }
          
       // Diagnostic LED
       if(Flag_MEMSOK == RESET || Flag_SPIOK == RESET)WLED_ON;
       else
       {
          if(Sec_Tick == 99 && (RTC_Time.RTC_Seconds % 8 == 0) && Flag_TCPConnect[1] == SET)WLED_ON;
          else if(Sec_Tick == 999)WLED_OFF;
       }
     
     }
  };
   
  
  if((Protocol == HP_PC || Protocol == NIC_OD) && Flag_TestMode == RESET)
  {
     if(GEO_Buzz_Time && Protocol == HP_PC)
     {
       GEO_Buzz_Time--;
       if(GEO_Buzz_Time == 0)OP2_STOP;
     }    
     if(Flag_Emrgcy == SET)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 199)
       {
         if(ED_Timer)OP2_TGL;
         else OP2_STOP;
         Buzz_Tim = 0;
       }
     }
     else if(Flag_OverSpeed == SET)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 599)
       {
         if(OVS_Buzz_Timer < 600)OP2_TGL;
         else OP2_STOP;
         Buzz_Tim = 0;
       }
     }
     else if(Flag_TILT == SET && Protocol == HP_PC)
     {
       Buzz_Tim++;
       if(Buzz_Tim > 999)
       {
         if(TLT_Buzz_Timer < 600)OP2_TGL;
         else OP2_STOP;
         Buzz_Tim = 0;
       }
     }
     else if(GEO_Buzz_Time && Protocol == HP_PC)OP2_START;
     else OP2_STOP;
  }
   
  if(Sec_Tick)Sec_Tick--;                                                         
  else 
  {
    Sec_Tick = 1000;
    Flag_Second = SET;
    
    if(Vehicle.Ignition[0] == 'F')PL_OFF;
    else PL_TGL;
    
    if(Flag_Emrgcy)PL_ON;
   
    RTC_GetTime(RTC_Format_BIN, &RTC_Time);
    
    if(Flag_IRNSSFix == RESET)IRNSS_FailTime++; else IRNSS_FailTime = 0;
    
    if(Reset_SystemTime)Reset_SystemTime--;                                     // Receive Confidata timeout
    if(Reset_SystemTime == 1)Flag_SysReset = SET; 
         
    // Stand By Time
    if(STNDBY_WAITTime)STNDBY_WAITTime--;
    if((Vehicle.Ignition[0] == 'F') && (Flag_Emrgcy == RESET))
    {
      if(Flag_DeepSleep == RESET)
      {
        AM_TimeOut++;
        if(AM_TimeOut > Time2Sleep  && Time2Sleep > 0)
        {
          Flag_STDBY = SET;                           // ACTIVE MODE TIMEOUT -> GOTO SLEEP MODE/ STANDBY MODE 
          Flag_SaveODO = SET;
          if(Current_Hit_Time !=  StopMode_Time)
          {
            Current_Hit_Time = StopMode_Time;
            ServerHit_Time = Current_Hit_Time-1;
            TER_ServerHit_Time = ServerHit_Time + 5; 
          }
        }
      }
    }
    else AM_TimeOut = 0;
    
    // Sleep managment
    if((Flag_DeepSleep == SET) && (Flag_Emrgcy == RESET) && (Flag_SelfTest == RESET)) 
    {
      if(Time2SleepAgain)
      {
        Time2SleepAgain--;
        if(Time2SleepAgain == 0)Flag_STDBY = SET;
      }
      
    }
    if(Flag_DS_RM == RESET) { if(DSC)DSC--; }
   
    if(RcvData_Time)RcvData_Time--;                                           // Receive Confidata timeout
    if(RcvData_Time == 1)
    {
      Flag_RecieveCfg = RESET;
      Flag_ServerResp[MAIN] = RESET;
      Flag_ServerResp[TER] = RESET;
      Flag_ServerResp[EMR] = RESET;
    }
    
    if(CHRG_Delay)CHRG_Delay--;                                               // Charger OFF when GPS Not FIX
    if(Response_Time[MAIN])Response_Time[MAIN]--;
    if(Response_Time[TER])Response_Time[TER]--;
    if(Response_Time[EMR])Response_Time[EMR]--;
    if(Current_Hit_Time > 0)
    {
       No_Hit_Time++;                                                         // SET SECOND FLAG
       if(Current_Hit_Time < 20)
        {
          if(No_Hit_Time == (Current_Hit_Time * 30))Flag_MODULE_M66_RST = SET;// if no hit in last 30x AT
          else if(No_Hit_Time == (Current_Hit_Time * 50)) { Flag_SWAP = SET; SP_REQ = 0;}    // Swap Profile if not hti for 50 x AT
          else if(No_Hit_Time > (Current_Hit_Time * 70))Flag_SysReset = SET;  // if no hit in last 60x hit time then system Reset   
        }
        else 
        {
          if(No_Hit_Time == (Current_Hit_Time * 5))Flag_MODULE_M66_RST = SET; // if no hit in last 50x hit time then system Reset
          else if(No_Hit_Time == (Current_Hit_Time * 10)){ Flag_SWAP = SET; SP_REQ = 0;}      // Swap Profile if not hti for 50 x AT
          else if(No_Hit_Time > (Current_Hit_Time * 15))Flag_SysReset = SET;   // if no hit in last 50x hit time then system Reset   
        }     
     }

    // Primary Server Hitting
    if((ServerHit_Time > (Current_Hit_Time + 2)) && (Vehicle.Ignition[0] == 'I'))ServerHit_Time = Current_Hit_Time-2;
    if(ServerHit_Time)ServerHit_Time--; 
    else
    {
       if(Current_Hit_Time)
       {
         Flag_MAIN_PKT = SET;
         Gen_PKT();
       }
       else 
       {
         Current_Hit_Time = Active_Time;
         ServerHit_Time = Current_Hit_Time - 1;
       }
       
     };
 
    // Tertirary Server Hitting
    if((TER_ServerHit_Time > (TER_HitTime + 5)) && (TER_HitTime > 4) && (Vehicle.Ignition[0] == 'I'))TER_ServerHit_Time = TER_HitTime;
    if(TER_ServerHit_Time)TER_ServerHit_Time--; 
    else
    {   
      Flag_TER_PKT = SET;
      Gen_PKT();
    };
    
    // Emergency Server Hitting
    if(Flag_Emrgcy)
    {
      if((Flag_EM_LED == RESET) && (PB_ADC < 1200))Flag_EM_LED = SET;
      if(EMRGY_PKTHit_Time > ES_Time)EMRGY_PKTHit_Time = ES_Time;
       
      if(EMRGY_PKTHit_Time)EMRGY_PKTHit_Time--;
      else 
      {
        if(ES_Time)
        {
          EMRGY_PKTHit_Time = ES_Time-1;
          Get_EMRGY_PKT(); 
        }
      }
      
      if(Flag_TCPConnect[EMR] == RESET)SMS_FAllBack_Check++;   
   
    }
        
    // Check for swap
    if((SIM_Err > 0) || (Flag_CSQErr == SET)){SIM_SWAPTimer++; if(SIM_SWAPTimer > 239){SIM_SWAPTimer = 0; SIM_Err = 0; Flag_SWAP = SET; SP_REQ = 0; }}
    else SIM_SWAPTimer = 0;
    
    // HA HB Detection
    if(HA_HB)HA_HB--;  
   } 

}


void USART1_IRQHandler(void)
{
 char *GPtr,*GPRSPtr; 
 unsigned char RcvIn;
 
 
 GPtr = NULL;
 DPtr = NULL;
 GPRSPtr = NULL;
 
  if(USART_GetITStatus(GPRS_USART, USART_IT_RXNE) != RESET)
  {
    Flag_GPRSUpdate = SET;
   
    RcvIn = (unsigned short)(GPRS_USART->RDR & (unsigned short)0x01FF);
    USART_ClearITPendingBit(GPRS_USART,USART_IT_RXNE);
  
    if(Flag_RecieveCfg == SET)
    {
        if((Recieve_Buffer[Data_RcvCnt] == '#' && strstr(Recieve_Buffer,"@")))
        {
           Recieve_Buffer[Data_RcvCnt+1] = 0;
           Flag_RecieveCfg = RESET;
           strcpy(Config_Buffer,Recieve_Buffer);                                // COPY RECIEVE BUFFER INTO GPRS MSG BUFFER
           memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
           Flag_NewConf = SET;
           RcvData_Time = 0;
           
           Data_RcvCnt = 0;Flag_RecieveCfg = RESET;RcvData_Time = 0; 
           memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
           
           if(((strstr((char*)Recieve_Buffer,"STOP_MSG")) || (strstr((char*)Recieve_Buffer,"CLRSOS")))&& (Flag_Emrgcy == SET) && (Flag_ServerConf[EMR] == SET))
           {
             
           CLR_EMR:
             ED_Timer = 0; Flag_Emrgcy = RESET;
             Save_EMR();Flag_EA = RESET;Flag_EA_OFF = SET;  
             
             Flag_MAIN_PKT = SET;
             Flag_TER_PKT = SET;
             ServerHit_Time = 2; 
             TER_ServerHit_Time = TER_HitTime + 3;
             
             Flag_EM_LED = RESET; 
             Get_EMRGY_PKT(); 
             Flag_HitServ = SET;
           }
          
         }
        else if(RcvIn == 10)
        {        
           if(strstr(Recieve_Buffer,"+ACC "))
           {
               if((strstr(Recieve_Buffer," CLRSOS")) && (Flag_Emrgcy == SET) && (Flag_STOPMSG == SET))
               {
                 Flag_NewConf = SET;
                 if(Flag_ServerConf[TER] == SET || Flag_ServerConf[MAIN] == SET || Flag_ServerConf[EMR] == SET)goto CLR_EMR;
               }
               else
               { 
                 strcat(Recieve_Buffer,"\r\n");
                 Recieve_Buffer[Data_RcvCnt+3] = 0;
                 Flag_RecieveCfg = RESET;
                 strcpy(Config_Buffer,Recieve_Buffer);                                // COPY RECIEVE BUFFER INTO GPRS MSG BUFFER
                 memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
                 Flag_NewConf = SET;
                 RcvData_Time = 0;
                 
                 Data_RcvCnt = 0;Flag_RecieveCfg = RESET;RcvData_Time = 0; 
                 memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
               }         
           }          
           else if((strstr(Recieve_Buffer,"CLRSOS")) && (Flag_Emrgcy == SET) && (Flag_STOPMSG == SET))
           {
             goto CLR_EMR;
           }
           else if(GPRSPtr = strstr((char*)Recieve_Buffer,"QIRD: 0"))
           {
             if(GPRSPtr = strstr((char*)Recieve_Buffer,"QIRD="))
             {
               GPRSPtr += 5;
               int SRV = *GPRSPtr - 48;
               Flag_ServerResp[SRV] = RESET;
            }
             
           }  
           else
           { 
             Rcv_CntCR++;
             if(Rcv_CntCR >= 4)
             {
               Rcv_CntCR = 0;
               strcat(Recieve_Buffer,"\r\n");
               Recieve_Buffer[Data_RcvCnt+3] = 0;
               Flag_RecieveCfg = RESET;
               
               if(GPtr = strstr(Recieve_Buffer,"QIRD:"))
               {
                 if(*(GPtr + 6) == '0')return;
                 strcpy(Config_Buffer,GPtr);                                // COPY RECIEVE BUFFER INTO GPRS MSG BUFFER
                 memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
                 Flag_NewConf = SET;
                 RcvData_Time = 0;
               }
               
               Data_RcvCnt = 0;Flag_RecieveCfg = RESET;RcvData_Time = 0; 
               memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
             }
           }
        }
        else 
        {
          Recieve_Buffer[Data_RcvCnt] = RcvIn;
          Data_RcvCnt++;
          if(Data_RcvCnt > 2308){Data_RcvCnt = 0;Flag_RecieveCfg = RESET;RcvData_Time = 0;}
        }
        
    }
    else
    {  
    if(RcvIn != 0)
    {
      if(RcvIn == 10 && Flag_ReadData == RESET)                                                          // LF
      { 
       /*----- OK RESPONSE ---------------------------------------------------*/  
       if((strstr((char*)GPRS_Buffer,"OK"))  && Flag_WrongIMEI == RESET){Flag_ModemOK = SET;GPRS_Count = 0;Modem_NoResp = 0;}                                                          
       /*----- MESSAGE RESPONSE ----------------------------------------------*/
       else if(strstr((char*)GPRS_Buffer,"MTI:") || strstr((char*)GPRS_Buffer,"MGR:"))
       {
          if(strstr((char*)GPRS_Buffer,"+CMTI") && (strstr((char*)GPRS_Buffer,"SM") || strstr((char*)GPRS_Buffer,"ME")))
           {
              if((GPtr = strstr((char*)GPRS_Buffer,"SM")) || (GPtr = strstr((char*)GPRS_Buffer,"ME")))
              {
                GPtr += 4;
                SMS_LOC = atoi(GPtr);
                Flag_SMSReady = SET;
                GPRSPtr =  GPtr - 4;
                if(strstr((char*)GPRS_Buffer,"ME"))Flag_MsgME = SET;
               }
            }
           else 
           {
             if(GPtr = strstr((char*)GPRS_Buffer,"REC"))
             {
               if(Flag_ReadSMSData == RESET)Flag_ReadSMSData = SET;
               else
               {
                 Flag_ReadSMSData = RESET;
                 memset(GPRS_MsgBuffer,0,sizeof(GPRS_MsgBuffer));
                 strcat((char*)GPRS_MsgBuffer,GPtr);
                 Flag_SMSRead = SET;
               }
             }
            }    
         
       }               
       /*----- RECIEVE DATA FROM SERVER --------------------------------------*/ 
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"\"recv\""))
       {
         GPRSPtr += 7; 
         int SRV = *GPRSPtr - 48;
         if(SRV < 3)
         {
           Flag_ServerConf[SRV] = SET;
           memset(CC_Source,0, sizeof(CC_Source_TER));
           sprintf(CC_Source,"%s,", SRVR_IP[SRV]);
           
           Flag_RecieveCfg = SET; 
           memset(Recieve_Buffer,0, sizeof(Recieve_Buffer));
           Data_RcvCnt = 0;
           ChkSRVR_Time = 12;
           RcvData_Time = 10;
         }
       }  
       /*----- STK RESPONSE ---------------------------------------------------*/  
       else if(strstr((char*)GPRS_Buffer,"STKGI: "))
       {
         if(strstr((char*)GPRS_Buffer,"STKGI: 37") && strstr((char*)GPRS_Buffer,"OK"))Flag_ModemOK = SET;
         else if((strstr((char*)GPRS_Buffer,"STKGI: 36") && strstr((char*)GPRS_Buffer,"OK")))Flag_ModemOK = SET;
       }                  
       /*------- Sockect Connect Error -----------------------------------------*/
       else if(GPtr = strstr((char*)GPRS_Buffer, ",56"))
       {
          GPtr--;
          Flag_TCPConnect[*GPtr - '0'] = RESET;
          Flag_CloseSocket[*GPtr - '0'] = SET;
       }                   
       /*----- SOCKET OPEN -------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: "))
       {
              if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 0,0")) { Flag_SocketOpned[MAIN] = SET; Flag_TCPConnect[MAIN] = SET; }
         else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 1,0")) { Flag_SocketOpned[TER] = SET; Flag_TCPConnect[TER] = SET; }
         else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 2,0")) { Flag_SocketOpned[EMR] = SET; Flag_TCPConnect[EMR] = SET; }
         else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 0,56")){ Flag_CloseSocket[MAIN] = SET; Flag_TCPConnect[MAIN] = RESET; }
         else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 1,56")){ Flag_CloseSocket[TER] = SET; Flag_TCPConnect[TER] = RESET; }
         else if(GPRSPtr = strstr((char*)GPRS_Buffer,"OPEN: 2,56")){ Flag_CloseSocket[EMR] = SET; Flag_TCPConnect[EMR] = RESET; }
       }    
       /*----- Cell Changed ------------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"+CGREG: 1,"))Flag_CellChange = SET;
       /*----- PDP DEACTIVATED -----------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"deact")){Flag_TCPConnect[MAIN] = RESET; Flag_TCPConnect[TER] = RESET;Flag_TCPConnect[EMR] = RESET; Flag_Error = SET;GPRS_Count = 0;}       
       /*----- ERROR RESPONSE ------------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"+CME ERROR: 10")){Flag_TCPConnect[MAIN] = RESET; Flag_TCPConnect[TER] = RESET;Flag_TCPConnect[EMR] = RESET; Flag_Error = SET;GPRS_Count = 0;}
       /*----- While TCP SEND ERRORS --------------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"CMS ERROR")){ GNSS_Time = 10; GPRS_Count = 0;}
        /*----- ERROR RESPONSE INVALID LENGTH------------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"ROR: invalid length")){GNSS_Time = 10;GPRS_Count = 0;}
       /*----- POWER DOWN RESPONSE -------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"UNDER_VOLTAGE WARNING")){Low_Battery_CNT++; if((Low_Battery_CNT > 1) && (Vehicle.Battery[0] != 'C'))Flag_STDBY = SET;}   
       /*----- CONNECTION CLOSED ---------------------------------------------*/
       else if(GPRSPtr = strstr((char*)GPRS_Buffer,"URC: \"closed\","))
       {
         GPRSPtr +=14; 
         if(*GPRSPtr == '0'){ Flag_TCPConnect[MAIN] = RESET; TCP_State[MAIN] = OPEN_SOCKET; Flag_SocketOpned[MAIN] = RESET; }
         else if(*GPRSPtr == '1'){ Flag_TCPConnect[TER] = RESET; TCP_State[TER] = OPEN_SOCKET; Flag_SocketOpned[TER] = RESET; }
         else if(*GPRSPtr == '2'){ Flag_TCPConnect[EMR] = RESET; TCP_State[EMR] = OPEN_SOCKET; Flag_SocketOpned[EMR] = RESET; }
         GPRS_Count = 0; 
       }            
       if(GPRSPtr != NULL){*GPRSPtr = ' '; GPRSPtr++;*GPRSPtr = ' '; GPRSPtr = NULL;};
      }/*----- IF RECIEVED IS NOT LF -------------------------------------------*/
      else 
      {
         /*--------------------- STORE DATA INTO GPRS BUFFER ---------------------*/
         GPRS_Buffer[GPRS_Count] = RcvIn;                                            
         GPRS_Count++;
         if(GPRS_Count > GPRSBuffer_Size-2)GPRS_Count = 0;
      }; 
    }/*----- END OF CHARACTER NOT 0 -------------------------------------------*/
   };
     Flag_GPRSUpdate = RESET;
   
     
    if((Flag_Debug_GPRS == SET) && (Flag_Serial == RESET))
    {
      if((Flag_TxENB == SET))
      {
        Flag_TxENB = RESET;
        TxData[0] = RcvIn;
        
        DMA2_Channel1->CNDTR = 1;
        DMA_Cmd(DMA2_Channel1, ENABLE);
       }
    }
    
    
  }
   else
   {
     USART_ClearITPendingBit(GPRS_USART,USART_IT_CTS);
     USART_ClearITPendingBit(GPRS_USART,USART_IT_TC);
     USART_ClearITPendingBit(GPRS_USART,USART_IT_RXNE);
     USART_ClearITPendingBit(GPRS_USART,USART_IT_ORE);
     
     RcvIn = (unsigned short)(GPRS_USART->RDR & (unsigned short)0x01FF);
   };
}

void USART2_IRQHandler(void)
{
 char *GNSSPtr;
 uint8_t RcvIn;
 static uint8_t RcvCnt_GNSS;                                                                    // BUFFER COUNTER FOR GNSS

 GNSSPtr = NULL;
 
 if(USART_GetITStatus(GNSS_USART, USART_IT_RXNE) != RESET)
  {
    RcvIn = (unsigned short)(GNSS_USART->RDR & (unsigned short)0x01FF);
    
    if((Flag_LogIn == SET) && (Flag_Debug_GNSS == SET) && (Flag_Serial == RESET))
    {
      if(Flag_TxENB == SET)
      {
        Flag_TxENB = RESET;
        TxData[0] = RcvIn;
        
        DMA2_Channel1->CNDTR = 1;
        DMA_Cmd(DMA2_Channel1, ENABLE);
       }
    }
    
    if(RcvIn == 10)
     {
       IRNSS_Buffer[RcvCnt_GNSS] = 0;
       
       if(GNSSPtr = strstr((char*)IRNSS_Buffer,"RMC,"))
       {
         GNSSPtr -= 3;
         memset(Rcv_IR_RMC,0,99);
         strncpy(Rcv_IR_RMC,GNSSPtr,99);
         if((strstr(GNSSPtr,",D,")) || (strstr(GNSSPtr,",A,")))
         {
            Flag_IRNSSFix = SET; IRNSS_FailTime = 0;
         }
       }
       else if(GNSSPtr = strstr((char*)IRNSS_Buffer,"GGA,"))
       {
         GNSSPtr -= 2;RcvCnt_GNSS -= 1;
         memset(Rcv_IRNGGA,0,99);
         strncpy(Rcv_IRNGGA, GNSSPtr,99);
        }
       else if(GNSSPtr = strstr((char*)IRNSS_Buffer,"$GPGSV,"))
       {
          GNSSPtr +=11; GPS_Visible_Satellite = atoi(GNSSPtr);   
         }
       else if(GNSSPtr = strstr((char*)IRNSS_Buffer,"$IRGSV,"))                 // IRNSS satellite view number
       {
         GNSSPtr +=11; IRNSS_Visible_Satellite = atoi(GNSSPtr);
       }
       else if(GNSSPtr = strstr((char*)IRNSS_Buffer,"GSA,"))
       {
         GNSSPtr -= 2;RcvCnt_GNSS -= 1;
         memset(Rcv_GSA,0,99);
         strncpy(Rcv_GSA,GNSSPtr,99);
        }
       else if(GNSSPtr = strstr((char*)IRNSS_Buffer,"GNS,"))                      //GNGNS
       {
         GNSSPtr -= 3;
         if((strstr(GNSSPtr,",D,")) || (strstr(GNSSPtr,",A,")))
         {
           if(Flag_IRNSSFix == RESET)GNSS_Time = 1;
           Flag_IRNSSFix = SET; IRNSS_FailTime = 0;
           
           GPS_HH = (IRNSSData.TIME[0] - '0')*10 + (IRNSSData.TIME[1] - '0');      // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
           GPS_MM = (IRNSSData.TIME[2] - '0')*10 + (IRNSSData.TIME[3] - '0');      // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
           GPS_SS = (IRNSSData.TIME[4] - '0')*10 + (IRNSSData.TIME[5] - '0');      // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2
         
           if(GPS_SS > RTC_Time.RTC_Seconds + 3 || GPS_SS < RTC_Time.RTC_Seconds - 3) Flag_RTCSet = SET;
         
         }
         else {Flag_IRNSSFix = RESET;}
        }
       
       RcvCnt_GNSS = 0;
       memset(IRNSS_Buffer,0,100);
     }
    else
     {
      IRNSS_Buffer[RcvCnt_GNSS] = RcvIn;
      RcvCnt_GNSS++;if(RcvCnt_GNSS > 99)RcvCnt_GNSS = 0;
     };
    
     USART_ClearITPendingBit(GNSS_USART,USART_IT_RXNE);
    
   }
else 
  {
    USART_ClearITPendingBit(GNSS_USART,USART_IT_CTS);
    USART_ClearITPendingBit(GNSS_USART,USART_IT_TC);
    USART_ClearITPendingBit(GNSS_USART,USART_IT_RXNE);
    USART_ClearITPendingBit(GNSS_USART,USART_IT_ORE);

    RcvIn = (unsigned short)(GNSS_USART->RDR & (unsigned short)0x01FF);
  }
}

void USART3_8_IRQHandler(void)
{
  static uint16_t RcvCnt_Serial;                                                // FOR SERIAL DATA
  uint8_t RcvIn;
  FlagStatus Flag_logRST = RESET;
 
  char *SerPtr;
  SerPtr = NULL;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {
    
    RcvIn = (unsigned short)(USART3->RDR & (unsigned short)0x01FF);
    
    // start recieving string #START_TEST
   // if(RcvIn == '#')RcvCnt_Serial = 0;
    
    if(RcvIn == 10)
    {
      Recieve_Buffer[RcvCnt_Serial] = RcvIn;

      if(strstr((char*)Recieve_Buffer,"SET_OB"))Flag_OB_APLY = SET;
      
      if(strstr((char*)Recieve_Buffer,"FLASH"))
      {
        Flag_Serial_IAP = SET;
      }
      if(strstr((char*)Recieve_Buffer,"LOG_RST"))
      {
        Flag_logRST = SET;
        Flag_USN = SET;
        Flag_TxENB = RESET;
        Flag_LogIn = RESET;
        Flag_PWD_Req = RESET;
        Flag_Debug_GNSS = RESET;
        Flag_Debug_GPRS = RESET;
      }
      /*------------- TEST MODE ----------------------------------------------------*/      
      if(Flag_SelfTest)
      {
        if(strstr((char*)Recieve_Buffer,"DIN1234_ON"))
        {
          DIN = 0;
          
          if(GPIO_ReadInputDataBit(PORT_DIN1,PIN_DIN1) == RESET)DIN |= 0x01;
          if(GPIO_ReadInputDataBit(PORT_DIN2,PIN_DIN2) == RESET)DIN |= 0x02;
          if(GPIO_ReadInputDataBit(PORT_DIN3,PIN_DIN3) == RESET)DIN |= 0x04;
          if( ADCConvertedValue[5] < 2000) DIN |= 0x08;   
          //if(GPIO_ReadInputDataBit(PORT_DIN4,PIN_DIN4) == SET)  DIN |= 0x08;
          
         
        }
        else if(strstr((char*)Recieve_Buffer,"DIN1234_OFF"))
        {
          if((GPIO_ReadInputDataBit(PORT_DIN1,PIN_DIN1) == SET)   && ((DIN & 0x01) == 1))Vehicle.DIN |= 0x01; else DIN &= 0xFE;
          if((GPIO_ReadInputDataBit(PORT_DIN2,PIN_DIN2) == SET)   && ((DIN & 0x02) == 2))Vehicle.DIN |= 0x02; else DIN &= 0xFD;
          if((GPIO_ReadInputDataBit(PORT_DIN3,PIN_DIN3) == SET)   && ((DIN & 0x04) == 4))Vehicle.DIN |= 0x04; else DIN &= 0xFB;
          
          if(( ADCConvertedValue[5] < 3600))Vehicle.DIN |= 0x08; else DIN &= 0xF7;    

          //if((GPIO_ReadInputDataBit(PORT_DIN4,PIN_DIN4) == RESET) && ((DIN & 0x08) == 8))Vehicle.DIN |= 0x08; else DIN &= 0xF7;
          
         
          Flag_DO = SET;
        }
        
        else if(strstr((char*)Recieve_Buffer,"EP_ON")){if(GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET)Flag_EP = SET;}
        else if(strstr((char*)Recieve_Buffer,"EP_OFF")){if((GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == SET) && (Flag_EP == SET))Vehicle.Battery[0] = 'C'; else Vehicle.Battery[0] = 'B';}  
        
        else if(strstr((char*)Recieve_Buffer,"IGN_ON")){if(GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == RESET)Flag_IGN = SET;}
        else if(strstr((char*)Recieve_Buffer,"IGN_OFF")){if((GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == SET) && (Flag_IGN == SET))Vehicle.Ignition[0] = 'I'; else Vehicle.Ignition[0] = 'F';}  
        
        else if(strstr((char*)Recieve_Buffer,"EP_Test_Done"))Flag_LDR = SET;
        
        else if(strstr((char*)Recieve_Buffer,"COVER LDR"))Flag_CheckLDR1 = SET;
        else if(strstr((char*)Recieve_Buffer,"GLOW LDR")) Flag_CheckLDR2 = SET;
          
      }
      
/*-------------------- TEST MODE END -----------------------------------------*/ 
      else if(Flag_LogIn == SET)
      {
/*--------------------- GENEREAL SERIAL COMMANDS  ----------------------------*/   
         
        if(strstr((char*)Recieve_Buffer,"SET"))Flag_SerialConfig = SET; 
        else if(((strstr((char*)Recieve_Buffer,"STOP_MSG")) || (strstr((char*)Recieve_Buffer,"CLRSOS"))) && (Flag_Emrgcy == SET) && (Flag_STOPMSG == SET))
        {
           ED_Timer = 0;
           Flag_Emrgcy = RESET;
           
           Save_EMR();
           
           Flag_EA = RESET;
           Flag_EA_OFF = SET;  
           
           Flag_MAIN_PKT = SET;
           Flag_TER_PKT = SET;
           ServerHit_Time = 2; 
           TER_ServerHit_Time = TER_HitTime + 3;
        
           Flag_EM_LED = RESET; 
           Get_EMRGY_PKT(); 
           Flag_HitServ = SET;
           Flag_USN = SET;
        }
       else if(strstr((char*)Recieve_Buffer,"@SET#"))Flag_SerialConfig = SET; 
        /*---------SERIAL COMMANDS -------------------------------------------*/
        //else if(strstr((char*)Recieve_Buffer,"SET"))Flag_SerialConfig = SET; 
       else if(strstr((char*)Recieve_Buffer,"#START_TEST"))
        {
          Flag_SelfTest = SET;
          Flag_USN      = RESET;
          Flag_TxENB    = RESET;
          Flag_LogIn    = RESET;
          Flag_PWD_Req  = RESET;
          Flag_Debug_GNSS = RESET;
          Flag_Debug_GPRS = RESET;
        }
       else if(strstr((char*)Recieve_Buffer,"@GET#IMEI"))
        {
          Flag_USN   = RESET;
          Flag_TxENB = RESET;
          Flag_LogIn = RESET;
          Flag_PWD_Req = RESET;
          Flag_Debug_GNSS = RESET;
          Flag_Debug_GPRS = RESET;
          Flag_GETIMEI = SET;
        }
       
        else if(strstr((char*)Recieve_Buffer,"CLR"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"GET"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"RST"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"FCTRY"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"GEOFCTRY"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"POLL"))Flag_SerialConfig = SET; 
        
        else if(strstr((char*)Recieve_Buffer,"GPSD"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"GPSC"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"GSMD"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"GSMC"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"ACTV"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"HCHK"))Flag_SerialConfig = SET; 
        else if(strstr((char*)Recieve_Buffer,"HLTH"))Flag_SerialConfig = SET; 
        

        else if(strstr((char*)Recieve_Buffer,"@CLR#"))Flag_SerialConfig = SET;
        else if(strstr((char*)Recieve_Buffer,"@GET#IMEI")){Flag_GETIMEI = SET;}
        else if(strstr((char*)Recieve_Buffer,"@PASS#EFP"))Flag_SerialConfig = SET;
        else if(strstr((char*)Recieve_Buffer,"@IRNSSCOLD#"))Flag_Cold = SET;
        else if(strstr((char*)Recieve_Buffer,"@IRNSSWARM#"))Flag_Warm = SET;
        else if(strstr((char*)Recieve_Buffer,"@IRNSSHOT#"))Flag_Hot = SET;
        else if(strstr((char*)Recieve_Buffer,"#set$"))Flag_SerialConfig = SET;
        else 
        {
          Flag_SER_SYNErr = SET;
          Flag_USN  = RESET;
          Flag_PWD_Req = RESET;
        }
/*--------------------- GENEREAL SERIAL COMMANDS END -------------------------*/     
/*--------------------- SERIAL COMMANDS -------------------------------*/   
        
       if(Flag_SerialConfig == SET)
       {
         strcpy(Config_Buffer,Recieve_Buffer); 
       }
       }
      
      else 
      {
        if(strstr((char*)Recieve_Buffer,"#START_TEST"))
        {
          Flag_SelfTest = SET;
          Flag_USN      = RESET;
          Flag_TxENB    = RESET;
          Flag_LogIn    = RESET;
          Flag_PWD_Req  = RESET;
          Flag_Debug_GNSS = RESET;
          Flag_Debug_GPRS = RESET;
        }
        else if(strstr((char*)Recieve_Buffer,"@GET#IMEI"))
        {
          Flag_GETIMEI = SET;
          Flag_USN = RESET;
          Flag_TxENB = RESET;
          Flag_LogIn = RESET;
          Flag_PWD_Req = RESET;
          Flag_Debug_GNSS = RESET;
          Flag_Debug_GPRS = RESET;
        }
        else if(SerPtr = strstr((char*)Recieve_Buffer,"\r"))
        {
          *SerPtr = 0;
        
          if(Flag_PWD_Req == SET)
          {
            if((strcmp((char*)Recieve_Buffer,SER_PWD) == 0) || (strstr(Recieve_Buffer,"@MASS#PWD")))
            {
              Flag_PWD_Req = RESET;
              Flag_LogIn = SET;
              Flag_TxENB = SET;
              Flag_USN = SET;
            }
            else 
            {
              Flag_Ser_Err = SET;
            }
          }
          else if((strcmp((char*)Recieve_Buffer,SER_USN) == 0) || (strstr(Recieve_Buffer,"@MASS#PWD")))
          {
            Flag_USN = SET;
            Flag_PWD_Req = SET;
          }
          else 
          {
            if(Flag_logRST == SET)Flag_logRST = RESET;
            else Flag_Ser_Err = SET;
          }
        }
        
      }
                                         
      memset(Recieve_Buffer,0,strlen(Recieve_Buffer));
      RcvCnt_Serial = 0;
      
    }
    else
     {
      Recieve_Buffer[RcvCnt_Serial] = RcvIn;
      if(RcvIn != 0) RcvCnt_Serial++;
      if(RcvCnt_Serial > 2308)RcvCnt_Serial = 0; 
     };
    
   USART_ClearITPendingBit(USART3,USART_IT_RXNE);
  }  
#if defined (RS485_ENABLE)
  if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
  {
    RcvIn = (unsigned short)(USART6->RDR & (unsigned short)0x01FF);
    if(RcvIn == 10)
    {
      RcvIn = 0;
    }
    else
     {
      Recieve_Buffer[RcvCnt_Serial] = RcvIn;
      RcvCnt_Serial++;if(RcvCnt_Serial > 2308)RcvCnt_Serial = 0; 
     };
    
    USART_ClearITPendingBit(USART6,USART_IT_RXNE);
  }
#endif
  else
  { 
     USART_ClearITPendingBit(USART3,USART_IT_CTS);
     USART_ClearITPendingBit(USART3,USART_IT_TC);
     USART_ClearITPendingBit(USART3,USART_IT_RXNE);
     USART_ClearITPendingBit(USART3,USART_IT_ORE);
     RcvIn = (unsigned short)(USART3->RDR & (unsigned short)0x01FF);
#if defined (RS485_ENABLE)    
     USART_ClearITPendingBit(USART6,USART_IT_CTS);
     USART_ClearITPendingBit(USART6,USART_IT_TC);
     USART_ClearITPendingBit(USART6,USART_IT_RXNE);
     USART_ClearITPendingBit(USART6,USART_IT_ORE);
     RcvIn = (unsigned short)(USART6->RDR & (unsigned short)0x01FF);
#endif
  };


}

// EXTI IGN & EP
void EXTI0_1_IRQHandler(void) 
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET || EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    /* Clear the  EXTI line 1 pending bit */
    
    if((RTC_ReadBackupRegister(RTC_BKP_DR2) & 0x0000FFFF) == 'S')
    {      
      if(EXTI_GetITStatus(EXTI_Line1) == SET)
        RTC_WriteBackupRegister(RTC_BKP_DR2,((ServerHit_Time << 16) | 'E'));
      if(EXTI_GetITStatus(EXTI_Line0) == SET)
        RTC_WriteBackupRegister(RTC_BKP_DR2,((ServerHit_Time << 16) | 'I'));
      
      if(Flag_SPIOK == SET)
      {
        TAKE_FUL_BKP();
      }
      Flag_DeepSleep = RESET;
      SYSTEM_SW_RESET;
    }
  
  
    EXTI_ClearITPendingBit(EXTI_Line0);
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

// EXTI RI & MEMS
void EXTI4_15_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line8) != RESET)EXTI_ClearITPendingBit(EXTI_Line8); 
  if(EXTI_GetITStatus(EXTI_Line7) != RESET || EXTI_GetITStatus(EXTI_Line6) != RESET)
  {
    /* Clear the  EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line7); 
    if((EXTI_GetITStatus(EXTI_Line7) == SET) && ((RTC_ReadBackupRegister(RTC_BKP_DR2) & 0x0000FFFF) == 'S'))  //module RI
    {
      
      RTC_WriteBackupRegister(RTC_BKP_DR2,((ServerHit_Time << 16) | 'Q'));
      if(Flag_SPIOK == SET)
      {
        TAKE_FUL_BKP();
      }
      Flag_DeepSleep = RESET;
      
      SYSTEM_SW_RESET;
    }
  }
  
}

void DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler(void)
{
   if(DMA_GetITStatus(DMA2_IT_TC1) == SET)
    { 
       /* Disable DMA */
       DMA_ClearFlag(DMA2_IT_TC1);
       DMA_Cmd(DMA2_Channel1, DISABLE);
       
       if(Flag_LogIn == SET)Flag_TxENB = SET; 
    }
}



/*************** TIMER 14 TO CALIBRATE LSI CLOCK ******************************/
void TIM14_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM14, TIM_IT_CC1) != RESET)
  {
    /* Clear TIM14 Capture Compare 1 interrupt pending bit */
    TIM_ClearITPendingBit(TIM14, TIM_IT_CC1);
    
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      IC1ReadValue1 = TIM_GetCapture1(TIM14);
      CaptureNumber = 1;
    }
    else if(CaptureNumber == 1)
    {
       /* Get the Input Capture value */
       IC1ReadValue2 = TIM_GetCapture1(TIM14); 
       TIM_ITConfig(TIM14, TIM_IT_CC1, DISABLE);

       /* Capture computation */
       if (IC1ReadValue2 > IC1ReadValue1)
       {
         PeriodValue = (IC1ReadValue2 - IC1ReadValue1);
       }
       else
       {
         PeriodValue = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2);
       }
       /* capture of two values is done */
       CaptureNumber = 2;
    }
  }
}



/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */