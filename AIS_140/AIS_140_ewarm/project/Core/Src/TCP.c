/******************** (C) COPYRIGHT 2022 EDS INDIA *****************************
* File Name     : TCP.c					                       *
* Author        : SANTANU ROY                                                  *
* Date          : 16/12/2022                                                   *
* Description   : This file Configures TCP & Deals with ITs functions         *
* Revision	: Rev0 			                                       *
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Main.h"

/*******************************************************************************
* Function Name : TCP_Connection
* Description   : Manages the state Machine for TCP
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void TCP_Connection(char SRVR)
{
  char *Ptr;
  char *TPtr;
  uint8_t Check_Sum = 0;
  static char Protocol_Ver[8] = "1.0.0"; 
  
  uint32_t
  FlashAddress,
  DataSent = 0,                                                                 // SENT DATA 
  DataRemain = 0,                                                               // REMAINING DATA TO BE ACKNOWLEDGED 
  DataAck = 0;  
  uint64_t E_CKSM = 0;
  uint16_t CH_SUM = 0;
  char buff[50] = {0};
  
  switch(TCP_State[SRVR])
  {
  case OPEN_SOCKET      :
    Check_Signal_Nw_GPRS();                                                     // CHECK SIGNAL STRENGTH NETWORK REGISTRATION AND GPRS ATTACHMENT
    if(Flag_ValidNetwork == SET && Flag_APNSet == RESET)Set_APN();              // IF APN IS NOT VALID SET APN FIRST
    if(Flag_APNSet)
    {
      if(Flag_TCPConnect[SRVR] == RESET)
      {
        sprintf(GPRS_MsgBuffer,"AT+QIOPEN=1,%hu,\"tcp\",\"%s\",%hu,0,0",SRVR,&SRVR_IP[SRVR][0],SRVR_Port[SRVR]);// SAVE TCP CONNECT COMMAND AS STRING  
        Modem_PutString(GPRS_MsgBuffer);Response_Time[SRVR] = 30;
        TCP_State[SRVR] = WAIT_OPEN;
        mSec_Delay(300); // Wait for responseto open
      }
      else TCP_State[SRVR] = TCP_CONNECTED;
    }
    break;
    
  case WAIT_OPEN        :    
      sprintf(GPRS_SendBuffer,"PEN: %d,56",SRVR);
      if(TPtr = strstr((char*)GPRS_Buffer, GPRS_SendBuffer))
      {
        TPtr--;
        Flag_TCPConnect[*TPtr - '0'] = RESET;
        sprintf(GPRS_MsgBuffer,"AT+QICLOSE=%d",SRVR);
        Modem_PutString(GPRS_MsgBuffer);
        Response_Time[SRVR] = 0; TCP_State[SRVR] = OPEN_SOCKET;
        Flag_CloseSocket[SRVR] = RESET;
      }
      else if(Flag_CloseSocket[SRVR] == SET && Flag_TCPConnect[SRVR] == SET)
      {
        Flag_TCPConnect[SRVR] = RESET;
        Flag_CloseSocket[SRVR] = RESET;
        sprintf(GPRS_MsgBuffer,"AT+QICLOSE=%d",SRVR);
        Modem_PutString(GPRS_MsgBuffer);
        Response_Time[SRVR] = 0; TCP_State[SRVR] = CLOSE_SOCKET;
      }
      else if(strstr((char*)GPRS_Buffer, "PEN: 0,0:") || strstr((char*)GPRS_Buffer, "PEN: 1,0:") || strstr((char*)GPRS_Buffer, "PEN: 2,0:") || Flag_SocketOpned[SRVR] == SET) // Socket Just Opned send Login
      {
        if(SRVR == MAIN)
        {
          Flag_SocketOpned[SRVR] = RESET;

          if(Protocol == UK_PC)
          {
           if(Last_Lat[0] == 0)sprintf(GPRS_MsgBuffer,"$%s,%s,%s,%s,%s,%s,00.0000000,N,000.000000,E*",HEADER_ID,VENDOR_ID,Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,Protocol_Ver);
            else sprintf(GPRS_MsgBuffer,"$%s,%s,%s,%s,%s,%s,%s,N,%s,E*",HEADER_ID,VENDOR_ID,Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,Protocol_Ver,Last_Lat,Last_Lon);
            
            Ptr = &GPRS_MsgBuffer[0];
            while(*Ptr != 0) 
            {
             //E_CKSM ^= *Ptr;
             Check_Sum ^= *Ptr;
             Ptr++;
            }
           memset(buff,0,9); 
           //sprintf(buff,"%08llu",E_CKSM);
           sprintf(buff,"%02X",Check_Sum);
           
           Ptr = &GPRS_MsgBuffer[0];
           strcat(Ptr,buff);
          }
          else if(Protocol == NIC_PC || Protocol == NIC_OD ||  Protocol == BSNL_MH)
          {
             if(Protocol == BSNL_MH)
             {
               if(Flag_IRNSSFix == SET)
               {
                 sprintf(GPRS_MsgBuffer,"$LGN,%s,%s,%s,%s,AIS140,%s%s",VENDOR_ID,Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,IRNSSData.LAT,IRNSSData.LON);
                 GPRS_MsgBuffer[strlen(GPRS_MsgBuffer) - 1] = 0;
               }
               else
                 sprintf(GPRS_MsgBuffer,"$LGN,%s,%s,%s,%s,AIS140,00.0000000,N,000.000000,E",VENDOR_ID,Vehicle.VRN,GPRSData.IMEI_No,SW_Ver);
               Ptr = &GPRS_MsgBuffer[0];
               Check_Sum = 0;
               while(*Ptr != 0) 
               { 
                   Check_Sum ^= *Ptr;
                   Ptr++;
               }
               memset(buff,0,10); 
               sprintf(buff,"*%02X",Check_Sum);
               strcat(Ptr,buff);
             }
             else
             {
                if(Flag_IRNSSFix == SET)
                 sprintf(GPRS_MsgBuffer,"$LGN,%s,%s,%s,AIS140,%10.7f,%010.6f*",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,Geo_Lat,Geo_Long);
                else
                 sprintf(GPRS_MsgBuffer,"$LGN,%s,%s,%s,AIS140,00.0000000,000.000000*",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver);
                
                
                if(Protocol == NIC_OD)
                {
                   E_CKSM  = 0;Check_Sum = 0;
                   Ptr = &GPRS_MsgBuffer[0];
                   int len = strlen(Ptr);
                   GPRS_MsgBuffer[len - 1] = 0;
                   while(*Ptr != '\0') 
                   {    
                     E_CKSM += *Ptr;
                     Ptr++;
                     GP_Counter++;
                     if(GP_Counter > 240){GP_Counter = 0; break;}
                   }
                   
                   sprintf(buff,",%08llu*",E_CKSM);
                   Ptr = &GPRS_MsgBuffer[0];                                    // FIND BASE ADDRESS OF UPLOADING BACKUP PACKET
                   strcat(Ptr,buff);
                }
             }
          }   
          else                         
          {
            if(Flag_IRNSSFix == SET)
            {
              sprintf(GPRS_MsgBuffer,"$LP,%s,%s,%s,1.0,%s%s1,%s%s%s,%s",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,IRNSSData.DATE,IRNSSData.TIME,IRNSSData.LAT,IRNSSData.LON,IRNSSData.Altitude,IRNSSData.SPEED);
              GPRS_MsgBuffer[strlen(GPRS_MsgBuffer) - 1] = 0;
            }
            else
              sprintf(GPRS_MsgBuffer,"$LP,%s,%s,%s,1.0,%02d%02d20%02d,%02d%02d%02d,0,00.0000000,N,000.000000,E,000,000.0",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year, RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
           
             Ptr = &GPRS_MsgBuffer[0];
             Check_Sum = 0;
             while(*Ptr != 0) 
             { 
                 Check_Sum ^= *Ptr;
                 Ptr++;
             }
             memset(buff,0,10); 
             sprintf(buff,"*%02X",Check_Sum);
             strcat(Ptr,buff);
          }
           
           TPtr = &GPRS_MsgBuffer[0];
           sprintf(GPRS_SendBuffer,"AT+QISEND=%d",SRVR);
           Modem_PutString(GPRS_SendBuffer);mSec_Delay(300);
           Serial_PutString_GPRS(TPtr);Serial_SendData_GPRS(26);  
           TCP_State[SRVR] = TCP_CONNECTED;
           Response_Time[SRVR] = 30;
          
        }
        else if(SRVR == TER)
        {
           Flag_SocketOpned[SRVR] = RESET;          
           memset(GPRS_MsgBuffer,0,150);
          
           if(Flag_IRNSSFix == SET)
           {
             sprintf(GPRS_MsgBuffer,"$LP,%s,%s,%s,1.0,%s%s1,%s%s%s,%s",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,IRNSSData.DATE,IRNSSData.TIME,IRNSSData.LAT,IRNSSData.LON,IRNSSData.Altitude,IRNSSData.SPEED);
             GPRS_MsgBuffer[strlen(GPRS_MsgBuffer) - 1] = 0;
           }
           else
             sprintf(GPRS_MsgBuffer,"$LP,%s,%s,%s,1.0,%02d%02d20%02d,%02d%02d%02d,0,00.0000000,N,000.000000,E,000,000.0",Vehicle.VRN,GPRSData.IMEI_No,SW_Ver,RTC_Date.RTC_Date,RTC_Date.RTC_Month,RTC_Date.RTC_Year, RTC_Time.RTC_Hours,RTC_Time.RTC_Minutes,RTC_Time.RTC_Seconds);
           Ptr = &GPRS_MsgBuffer[0];
           Check_Sum = 0;
           while(*Ptr != 0) 
           { 
               Check_Sum ^= *Ptr;
               Ptr++;
           }
           memset(buff,0,10); 
           sprintf(buff,"*%02X",Check_Sum);
           strcat(Ptr,buff);
           
           TPtr = &GPRS_MsgBuffer[0];
           sprintf(GPRS_SendBuffer,"AT+QISEND=%d",SRVR);
           Modem_PutString(GPRS_SendBuffer);mSec_Delay(300);
           Serial_PutString_GPRS(TPtr);Serial_SendData_GPRS(26);  
           TCP_State[SRVR] = TCP_CONNECTED;
           Response_Time[SRVR] = 30;
        }
        else if(Flag_SocketOpned[SRVR] == SET)TCP_State[SRVR] = TCP_CONNECTED;
        
        
        Flag_ModemOK = SET;Flag_Error = RESET; 
        break;
      }
      else if(strstr((char*)GPRS_Buffer, "CLOSED"))
      {
        TPtr = strstr((char*)GPRS_Buffer, "CLOSED");*TPtr = ' ';TPtr++;*TPtr = ' ';TPtr -= 3;
        if(*TPtr == '0'){Flag_TCPConnect[MAIN] = RESET; }
        else if(*TPtr == '1'){Flag_TCPConnect[TER]  = RESET; }
        else if(*TPtr == '2'){Flag_TCPConnect[EMR]  = RESET; }
        WAIT_MODEM_RESP(2);                                                   // WAIT FOR MODEM RESPONSE       
        if(*TPtr == (SRVR + 48)) break;
      }
     
      if(Response_Time[SRVR] == 1)TCP_State[SRVR] = CLOSE_SOCKET;
      break;
    
  case TCP_CONNECTED    :
     TPtr = RAM_BACKUP[SRVR][0];
     if(*TPtr =='$')
     {
        sprintf(GPRS_MsgBuffer,"AT+QISEND=%d",SRVR);
        Modem_PutString(GPRS_MsgBuffer);
        
        // calculate the checksum
        if(Protocol == BSNL_MH && SRVR == MAIN)
        {
          if(DPtr = strstr(TPtr,")*"))
          {
             if(CC_Source_TER[0] != 0 && (strstr(TPtr,",OT,")))
             {
               TPtr = RAM_BACKUP[SRVR][0];
               Ptr = strstr(TPtr,"(0");
               int Len = 255 - (Ptr - TPtr);
               
               char Tempdata[50] = {0};
               memcpy(&Tempdata[0],Ptr+1, strlen(Ptr+1));
               *(Ptr+1) = 0;
               memset(Ptr+1, 0, Len);
               strcat(TPtr,CC_Source_TER);
               CH_SUM = 0x00;
               while(*TPtr != 0) {CH_SUM ^= *TPtr; TPtr++;}
               sprintf(buff,")*%02X",CH_SUM);
             }
             else
             {
               CH_SUM = 0x00;
               // Get Checksum 
               while(*TPtr != 0) {CH_SUM ^= *TPtr; TPtr++;}
               memset(buff,0,50); TPtr = RAM_BACKUP[SRVR][0];
               sprintf(buff,"%02X",CH_SUM);
             }
             
             TPtr = RAM_BACKUP[SRVR][0];
             strcat(TPtr,buff);
             
          }
       
        }
        else if((Protocol == NIC_PC || Protocol == NIC_OD) && SRVR == MAIN)
        {
          if(DPtr = strstr(TPtr,"XX,*"))
          {
             *DPtr = 0;
             E_CKSM = 0x00;
             // Get Checksum 
             while(*TPtr != 0) {E_CKSM ^= *TPtr; TPtr++;}
             memset(buff,0,50); TPtr = RAM_BACKUP[SRVR][0];
             sprintf(buff,"%04llu*",E_CKSM);
             
             TPtr = RAM_BACKUP[SRVR][0];
             strcat(TPtr,buff);
             
          }
        }      
        else
        {
          if(DPtr = strstr(TPtr,"XX,*"))
          {
             *DPtr = 0;
             CH_SUM = 0x00; E_CKSM = 0x00;
             // Get Checksum 
             while(*TPtr != 0) {CH_SUM ^= *TPtr; E_CKSM ^= *TPtr; TPtr++;}
             memset(buff,0,50); TPtr = RAM_BACKUP[SRVR][0];
                 
             if(SRVR == TER)
             {
               if(CC_Source_TER[0] != 0 && strstr(TPtr,",CC,"))
               {
                 for(int n=0; n < strlen(CC_Source_TER); n++)
                   if(CC_Source_TER[n] == ',')CC_Source_TER[n] = '-';
                   
                 TPtr = RAM_BACKUP[SRVR][0];
                 Ptr = strstr(TPtr,",,");
                 int Len = 255 - (Ptr - TPtr);
                 
                 char Tempdata[50] = {0};
                 memcpy(&Tempdata[0],Ptr+1, strlen(Ptr+1));
                 *(Ptr+1) = 0;
                 memset(Ptr+1, 0, Len);
                 strcat(TPtr,CC_Source_TER);
                 strcat(TPtr,&Tempdata[0]);
                 CH_SUM = 0x00;
                 while(*TPtr != 0) {CH_SUM ^= *TPtr; TPtr++;}
                 sprintf(buff,"%02X,*",CH_SUM);
                 memset(CC_Source_TER,0,sizeof(CC_Source_TER));
               }
               else sprintf(buff,"%02X,*",CH_SUM);
             }
             else if(SRVR == EMR)
             {
               if(Protocol == BSNL_PC || Protocol == HP_PC)
               {
                  sprintf(buff,"*%02X",CH_SUM);
               }
               else if(Protocol == NIC_PC)
               {
                  sprintf(buff,"*%04llu",E_CKSM);
               }
               else if(Protocol == NIC_OD)
               {
                  sprintf(buff,",%04llu*",E_CKSM);
               }
               else if(Protocol == BSNL_MH)
               {
                  sprintf(buff,"*%02X",CH_SUM);
               }
             }
             else sprintf(buff,"%02X,*",CH_SUM);
             
             TPtr = RAM_BACKUP[SRVR][0];
             strcat(TPtr,buff);
             
          }
        }
        
        mSec_Delay (250);           // wait to see if any error generated
         
        if(strstr((char*)GPRS_Buffer,"ERROR"))
        {
          Serial_SendData_GPRS(10);
          Serial_SendData_GPRS(26);  
          Flag_TCPConnect[SRVR] = RESET;  
          TCP_State[SRVR] = CLOSE_SOCKET;
          break;
        }
        else 
        {
          TPtr = RAM_BACKUP[SRVR][0];
          Serial_PutString_GPRS(TPtr);
          Serial_SendData_GPRS(10);
          Serial_SendData_GPRS(26);  
          WAIT_MODEM_RESP(1);
          TCP_State[SRVR] = ACK_DATA;
          Response_Time[SRVR] = 30;
        }
    } 
     else
     {
         memmove(&RAM_BACKUP[SRVR][0],&RAM_BACKUP[SRVR][1],4864); // 20 Buffer size so 19X256
         memset(&RAM_BACKUP[SRVR][19],0,256);
         if(RAM_BakCnt[SRVR])RAM_BakCnt[SRVR]--;
         if(TCP_BackCnt[SRVR])
          { 
             if(SRVR == 0)FlashAddress = SPIFLASH_BAKUP1 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
             else if(SRVR == 1)FlashAddress = SPIFLASH_BAKUP2 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
             else if(SRVR == 2)FlashAddress = SPIFLASH_BAKUP3 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
         
             TPtr = &RAM_BACKUP[SRVR][RAM_BakCnt[SRVR]][0];
             SPI_FLASH_BufferRead(TPtr,FlashAddress,256);
             TCP_BackCnt[SRVR]--;
             RAM_BakCnt[SRVR]++;
             RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
              
             if((TCP_BackCnt[SRVR] % 16) == 0 || TCP_BackCnt[SRVR] == 0)
             {
              sprintf(GPRS_SendBuffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],TCP_BackCnt[EMR]);  
              if(Flag_SPIOK == SET)
              {
                FlashAddress = SPIFLASH_CONFIG;  
                SPI_FLASH_SectorErase(FlashAddress);
                SPI_FLASH_PageWrite(GPRS_SendBuffer,FlashAddress,strlen(GPRS_SendBuffer));
              };
             }
           }
         
     }
    break;      
    
  case ACK_DATA         :
    if(Flag_TCPConnect[SRVR] == RESET)
    {
      TCP_State[SRVR] = CLOSE_SOCKET;
      break;
    }
    sprintf(GPRS_MsgBuffer,"AT+QISEND=%d,0",SRVR); // Qarry tge sent data
    Modem_PutString(GPRS_MsgBuffer);
    WAIT_MODEM_RESP(1);
    if(TPtr = strstr((char*)GPRS_Buffer,"SEND: "))
    {
      TPtr += 6;
      DataSent = atol(TPtr);                                    // SENT DATA
      TPtr = strchr(TPtr,0x2c);
      TPtr += 1;
      DataAck = atol(TPtr);                                     // ACKNOWLEDGE DATA
      TPtr = strchr(TPtr,0x2c);
      TPtr += 1;    
      DataRemain = atol(TPtr);                                  // REMAINING DATA
      
      if(DataSent == DataAck && DataRemain == 0 && Flag_SrvrLgn[SRVR] == RESET)
      {
        Response_Time[SRVR] = 0;
      
        No_Hit_Time = 0;
        TCP_State[SRVR] = TCP_CONNECTED;
        memmove(&RAM_BACKUP[SRVR][0],&RAM_BACKUP[SRVR][1],4864); // 20 Buffer size so 19X256
        memset(&RAM_BACKUP[SRVR][19],0,256);
        if(RAM_BakCnt[SRVR])RAM_BakCnt[SRVR]--;
        if(RAM_BakCnt[SRVR] < 5 && TCP_BackCnt[SRVR] > 0)
        {
          while(RAM_BakCnt[SRVR] < 5)
          {
            if(SRVR == 0)FlashAddress = SPIFLASH_BAKUP1 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
            else if(SRVR == 1)FlashAddress = SPIFLASH_BAKUP2 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
            else if(SRVR == 2)FlashAddress = SPIFLASH_BAKUP3 + ((TCP_BackCnt[SRVR]-1) * 0x100);           // SET FLASH ADDRESS FOR TAKING BACKUP INTO FLASH             
         
            if(TCP_BackCnt[SRVR])
            { 
               TPtr = &RAM_BACKUP[SRVR][RAM_BakCnt[SRVR]][0];
               SPI_FLASH_BufferRead(TPtr,FlashAddress,256); //FlashAddress += 256;
               TCP_BackCnt[SRVR]--;
               RAM_BakCnt[SRVR]++;
               RTC_WriteBackupRegister(RTC_BKP_DR0,(TCP_BackCnt[MAIN] << 16 | TCP_BackCnt[TER]));
                
               if((TCP_BackCnt[SRVR] % 16) == 0 || TCP_BackCnt[SRVR] == 0)
               {
                sprintf(GPRS_SendBuffer,"Backup1-%hu,Backup2-%hu,EMRGY-%hu",TCP_BackCnt[MAIN],TCP_BackCnt[TER],TCP_BackCnt[EMR]);  
                if(Flag_SPIOK == SET)
                {
                  FlashAddress = SPIFLASH_CONFIG;  
                  SPI_FLASH_SectorErase(FlashAddress);
                  SPI_FLASH_PageWrite(GPRS_SendBuffer,FlashAddress,strlen(GPRS_SendBuffer));
                };
               }
             }
             else break;
         }         
        }
      
        Get_HitTime(SRVR); // Get the last hit time
      }
    
      if(Flag_SrvrLgn[SRVR] == SET)TCP_State[SRVR] = TCP_CONNECTED; // No Data shift for Ack packet
      else
      {
        if(Flag_DeepSleep == SET && SRVR == TER && TCP_State[0] == TCP_CONNECTED && TCP_State[1] == TCP_CONNECTED && RAM_BakCnt[MAIN] == 0 && RAM_BakCnt[TER] == 0)Flag_STDBY = SET;        
      }
      Flag_SrvrLgn[SRVR] = RESET;
    }
    else if(strstr((char*)GPRS_Buffer,"ERROR") || Flag_Error == SET)
    {
      Flag_Error = RESET;
      Response_Time[SRVR] = 0;
      TCP_State[SRVR] = CLOSE_SOCKET;
    }
   
    if(Response_Time[SRVR] == 1)TCP_State[SRVR] = CLOSE_SOCKET;
     
    break;
    
  case CLOSE_SOCKET     :
    sprintf(GPRS_MsgBuffer,"AT+QICLOSE=%d",SRVR);
    Modem_PutString(GPRS_MsgBuffer);WAIT_MODEM_RESP(2);
    Response_Time[SRVR] = 0; TCP_State[SRVR] = OPEN_SOCKET;
    Flag_TCPConnect[SRVR] = RESET;
    break;
    
  case DEACT_CONTEXT    :
    sprintf(GPRS_MsgBuffer,"AT+QICLOSE=%d",SRVR);
    Modem_PutString(GPRS_MsgBuffer);WAIT_MODEM_RESP(2);
    Response_Time[SRVR] = 0; TCP_State[SRVR] = OPEN_SOCKET;
    break;
  }
}