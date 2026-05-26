/******************** (C) COPYRIGHT 2018 EDS-INDIA *****************************
* File Name     : App_init.c					       	       *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file provides all Application Initialization function   *
* Revision	: Rev0							       *
*******************************************************************************/
#include "Main.h"                                                               // HEADER FILE 
#include "App_Init.h"  

void INIT_APP(void)
{
  uint16_t  var;
  char *Ptr,*SPtr;
  char loc_buffer[70];
  uint32_t FlashAddress;
  uint8_t GP_Counter;
    
  
  memset(loc_buffer,0,70);
  GP_Counter   = 0;
  FlashAddress = 0;
  var = 0;                                                                      /* RESET THE VALUE */
  Ptr = NULL;                                                                   /* SET NULL POINTER */
  SPtr = NULL;
  
  Flag_ProgRTC = SET;
  Flag_ProgDate = SET;
  /*---------- TCP PACKET NUMBER -----------------------------------------------*/  
  
  Flag_Emrgcy = (FlagStatus)(RTC_ReadBackupRegister(RTC_BKP_DR3));
  Frame_Num = (RTC_ReadBackupRegister(RTC_BKP_DR3) >> 8);
  
  Flag_PowerOff = (FlagStatus)(RTC_ReadBackupRegister(RTC_BKP_DR1));
  Frame_Num_TER = (RTC_ReadBackupRegister(RTC_BKP_DR1) >> 8); 
  
  if(Flag_Emrgcy)Flag_Emrgcy = RESET;
  
  
  if(Flag_PowerOff == RESET)
  {
    REG_LM2576(ENABLE);                                                           /*  ENABLE EXTERNAL POWER SUPPLY  */
  }
  else
  {
    REG_LM2576(DISABLE);                                                           /*  DISABLE EXTERNAL POWER SUPPLY  */
  }

  MODULE_L89(ENABLE);                                                           /*  ENABLE MODULE L89 */                                                               
  
  CHRG_ON;
  Flag_Check_EXT_PWR = SET;
  
 /*---------- VEHICLE DATA ---------------------------------------------------*/
  
  //Vehicle.HDR[0] = HEADER_ID; Vehicle.HDR[1] = 0;                               // VENDOR HEADER 
  //sprintf(Vehicle.VID,VENDOR_ID);                                               // VENDOR ID 
  sprintf(Vehicle.VRN,"NA00000000");                                            // VEHICLE REGISTRATION NO.
  sprintf(&Vehicle.Master_No[0][0],"NA00000000");                                      // SET MASTER NUMBER 
  sprintf(&Vehicle.Master_No[1][0],"NA00000000");                                      // SET MASTER NUMBER 
  sprintf(&Vehicle.Master_No[2][0],"NA00000000");                                      // SET MASTER NUMBER 
  sprintf(&Vehicle.Master_No[3][0],"NA00000000");                                      // SET MASTER NUMBER 
  sprintf(&Vehicle.Master_No[4][0],"NA00000000");                                      // SET MASTER NUMBER 

/*---------- GNSS DATA -------------------------------------------------------*/
  GNSS_Time    = GNSS_INTERVAL;                                                      
 
/*---------- SIM RELATED VARIABLES -------------------------------------------*/
  Flag_SWAP = RESET;
  Flag_CSQErr = RESET;
  SIM_Err = 0;
  SwapCnt = 0;
  sprintf(GPRSData.SET_APN,"no-apn");
  
  if(Flag_Debug_GNSS == SET || Flag_Debug_GPRS == SET)Flag_TxENB = SET;
  /*---------- CHECK WAKE UP CAUSE ---------------------------------------------*/  
  if((RCC_GetFlagStatus(RCC_FLAG_PORRST)) == SET)
  {
    RCC_ClearFlag();                                                            // CLEAR RCC FLAGS 
  }
  else 
  {
    GP_Counter = 0;
    for(GP_Counter = 0; GP_Counter < 15; GP_Counter++)
    {
      if((IMEI[GP_Counter] < '0') || (IMEI[GP_Counter] > '9'))
      {
        memset(IMEI,0,16);
        break;
      }
      if(IMEI[0] == '0')
      {
         memset(IMEI,0,16); break;
      }
    }
    IMEI[15] = 0;
    
    if(((OFF_LAT[14] != 0) || (atof(OFF_LAT) < 7.0) || (atof(OFF_LAT) > 37.0)) || ((OFF_LON[14] != 0) || (atof(OFF_LON) < 67.0) || (atof(OFF_LON) > 98.0))) 
    {
      memset(OFF_LAT,0,25);
      memset(OFF_LON,0,25); 
      Flag_OFFGNSS = RESET;
    }
    if(((Last_Lat[14] != 0) || (atof(Last_Lat) < 7.0) || (atof(Last_Lat) > 37.0)) || ((Last_Lon[14] != 0) || (atof(Last_Lon) < 67.0) || (atof(Last_Lon) > 98.0)))
    {
      memset(Last_Lat,0,25);
      memset(Last_Lon,0,25);
    }     
  }
  
  IRNSS_FailTime = 50; // Initailly more than 30 Sec so that no Reset is activated
  GNSS_Time = 20;   // initial GNSS Set Time                                                         // Initial GNSS Check Time

  /*---------- GET DATA FROM MICROCONTROLLER FLASH MEMORY ----------------------*/
  Get_Config();                                                                 // GET SAVED/DEFAULT CNFIGURATION FROM FLASH
  
  
  /*---------- DIGITAL OUTPUT --------------------------------------------------*/
  if((DO_STATUS & 0x01) == 0x01){Vehicle.DOUT |= 0x01; OP1_START;}
  else {Vehicle.DOUT &= 0xFE; OP1_STOP;}  
  if((DO_STATUS & 0x02) == 0x02){Vehicle.DOUT |= 0x02; OP2_START;}
  else {Vehicle.DOUT &= 0xFD; OP2_STOP;} 


  
  /*---------- CHECK SPI FLASH MEMORY ------------------------------------------*/
  FlashAddress = SPI_FLASH_ReadID();                                            // GET FLASH STATUS WORKING 
  if(FlashAddress == SPI_FLASH_ID)
  {
    Flag_SPIOK   = SET;
  }
  else Flag_SPIOK = RESET;  

/*---------- READING BACKUP FROM FLASH ---------------------------------------*/
  FlashAddress = SPIFLASH_CONFIG;                                               // SET FLASH ADDRESS TO ZERO 
  if(Flag_SPIOK == SET)                                                     
  {
    memset(loc_buffer,0,70);
    SPI_FLASH_BufferRead(loc_buffer,FlashAddress,49);                        // FLASH READ FROM GPRS SEND BUFFER 
    if(Ptr = strstr(loc_buffer,"Backup1-"))
    {
      Ptr += 8;
      TCP_BackCnt[MAIN] = atoi(Ptr);                                           // TCP BACK COUNT VALUE OF PRIMARY SERVER 
   
      if(Ptr = strstr(Ptr,"Backup2-"))
       {
        Ptr += 8;
        TCP_BackCnt[TER] = atoi(Ptr);                                         // TCP BACK COUNT VALUE OF SECONDARY SERVER 
      }
      if(Ptr = strstr(Ptr,"EMRGY-"))
       {
       Ptr += 6;
       EMRGY_BKP_No = atoi(Ptr);
       }
    }
  }
  
 /*---------- READ ODOMETER VALUE FROM SPI FLASH -----------------------------*/
  FlashAddress = SPIFLASH_CONFIG_ODO;
  if(Flag_SPIOK == SET)                                                     
  {
    memset(loc_buffer,0,70);
    SPI_FLASH_BufferRead(loc_buffer,FlashAddress,69);                        // FLASH READ FROM GPRS SEND BUFFER 
    if(Ptr = strstr(loc_buffer,"KM-"))
    {
      Ptr += 3;
      if((KM < atoi(Ptr)) || (((KM - atoi(Ptr)) > 0) && ((KM - atoi(Ptr)) < 250)))
      {
        KM = atoi(Ptr); 
        if(Ptr = strstr(Ptr,"MTR-"))
         {
           Ptr += 4;
           MTR = atoi(Ptr);                                         // TCP BACK COUNT VALUE OF SECONDARY SERVER 
           if(MTR > 1000)MTR = 0;
         }
      }
      if(KM > 999999)KM = 0;
      
      if(Flag_POR == RESET)
      {
        if(Ptr = strstr(loc_buffer,"Lat-"))
        {
          Ptr += 4;
          SPtr = Ptr;
          memcpy(Last_Lat,SPtr,10);
        }
        if(Ptr = strstr(loc_buffer,"Lon-"))
        {
          Ptr += 4;
          SPtr = Ptr;
          memset(Last_Lon,0,15);
          memcpy(Last_Lon,SPtr,10);
        }
      }
      else 
      {
        Flag_POR = RESET;
        Write_Oddo();
      }
     }
   }
  
/*---------- READ FROM LOCAL BACKUP REGISTER ---------------------------------*/  
  var = RTC_ReadBackupRegister(RTC_BKP_DR0) >> 16;                              // MSB 16 BIT FOR MAIN SERVER 
  if(var != 0 && var < 40001)TCP_BackCnt[MAIN] = var;                           // EXTRACT TCP BACK COUNT OF PRIMARY SERVER FROM BACKUP REGISTER 0

  var = (RTC_ReadBackupRegister(RTC_BKP_DR0) & 0x00FF);                         // LSB 16 BITS FOR TERTIARY SERVER 
  if(var != 0 && var < 20001)TCP_BackCnt[TER] = var;                            // EXTRACT TCP BACK COUNT OF SECONDARY SERVER FROM BACKUP REGISTER 1
  
  if(GPIO_ReadInputDataBit(PORT_BATT,PIN_BATT) == RESET)Vehicle.Battery[0] = 'C';  // SOURCE POWER FROM EXTERNAL BATTERY   
  else 
  {
    Vehicle.Battery[0]  = 'B';                                                  // SOURCE POWER FROM INTERNAL BATTERY 
    Vehicle.Ignition[0] = 'F';                                                  // IGNITION OFF
    if(Flag_IRNSSFix)
    {
      Get_Location(); 
      if((Flag_ValidGNSSString == SET) && (Flag_OFFGNSS == RESET))
      {
        Flag_OFFGNSS = SET; 
        memset(OFF_LAT,0,15);
        memset(OFF_LON,0,15);
        memcpy(OFF_LAT,IRNSSData.LAT,14);
        memcpy(OFF_LON,IRNSSData.LON,14);
      }
    };
  }
  
  Vehicle.Batt_Status[0] = '1';                                                 // INITIAL BATTERY HEALTH OK CONSIDERED 
  Vehicle.Tamper[0]='1';                                                        // LET CONSIDER BOX IS OK AT START UP TIME  
    
  if(GPIO_ReadInputDataBit(PORT_DIN1,PIN_DIN1) == RESET)Vehicle.DIN |= 0x01;    // DIGITAL INPUT 1 ON/HIGH 1 ACTIVE HIGH
  else Vehicle.DIN &= 0xFE;                                                     // DIGITAL INPUT 1 OFF/LOW 0
  
  if(GPIO_ReadInputDataBit(PORT_DIN2,PIN_DIN2) == RESET)Vehicle.DIN |= 0x02;    // DIGITAL INPUT 2 ON/HIGH 1 ACTIVE HIGH
  else Vehicle.DIN &= 0xFD;                                                     // DIGITAL INPUT 2 OFF/LOW 0
                                                     
  if(GPIO_ReadInputDataBit(PORT_DIN3,PIN_DIN3) == RESET)Vehicle.DIN |= 0x04;    // DIGITAL INPUT 3 ON/HIGH 1 ACTIVE HIGH
  else Vehicle.DIN &= 0xFB;                                                     // DIGITAL INPUT 3 OFF/LOW 0
  
  /*---------- EMERGENCY BUTTON ----------------------------------------------*/
   Vehicle.DIN |= 0x08;                                                         // DIGITAL INPUT 4 OFF/LOW 0
  

  if(GPIO_ReadInputDataBit(PORT_IGN,PIN_IGN) == RESET) 
   {
    Vehicle.Ignition[0] = 'F';                                                  // IGNITION OFF
    if(Flag_IRNSSFix)
    {
      Get_Location(); 
      if((Flag_ValidGNSSString == SET) && (Flag_OFFGNSS == RESET))
      {
        Flag_OFFGNSS = SET; 
        memset(OFF_LAT,0,15);
        memset(OFF_LON,0,15);
        memcpy(OFF_LAT,IRNSSData.LAT,14);
        memcpy(OFF_LON,IRNSSData.LON,14);
      }
    };
   } 
  else
  {
    if((RTC_ReadBackupRegister(RTC_BKP_DR2) & 0x0000FFFF) == 'S')Flag_IgnOn = SET;
    Vehicle.Ignition[0] = 'I';                                                  // IGNITION ON
    Flag_OFFGNSS = RESET;
    memset(OFF_LAT,0,15);
    memset(OFF_LON,0,15);

    REG_LM2576(ENABLE);
   
            
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
  
  /***************** EXTERNAL BATTERY CHECK ******************************/
  Vehicle.EXT_VOLT = (AIN_VOLT[3]/50.0)*0.01352;
  if(Vehicle.EXT_VOLT > 6 && Vehicle.EXT_VOLT < 17){EXT_BATT_END = 10.5; EXT_PWR_FCTR=1;} // EXTERNAL POWER SOURCE 12V BATTERY            
  else if(Vehicle.EXT_VOLT > 17){EXT_BATT_END = 21; EXT_PWR_FCTR = 2;}            // EXTERNAL POWER SOURCE 24V BATTERY
  
  
  EXT_Batt_CutOff = EXT_BATT_END + (4.0 * EXT_PWR_FCTR * (EBL))/100.0;      // SET EXTERNAL BATTERY CUT-OFF VALUE
  /***********************************************************************/
   
  sprintf(GPRSData.IMEI_No,"N.A.00000000000");                                  // SET IMEI NUMBER                                  
  
  EC200_ONTime = 15;                                                             // Initial Turn ON
  
  /*---------------------- Define Statup Variables -----------------------------*/
  RAM_BakCnt[MAIN]    = 0;
  RAM_BakCnt[TER]     = 0;
  RAM_BakCnt[EMR]     = 0;
  
  No_Hit_Time  = 0;
  Modem_NoResp = 0;                                                             // RESET MODEM NO RESPONSE 
  
  HLTH_TIMER        = HLTH_FRQ;
  EMRGY_PKTHit_Time = ES_Time; 
  EMG_Button_Press  = EMG_BP_FREQ;                                              // VALUE IN MILISECONDS 

  char T = RTC_ReadBackupRegister(RTC_BKP_DR2) & 0x0000FFFF;
  if(Flag_DeepSleep == RESET && T != 'S')
  {
    ServerHit_Time    = StandBy_Time;
    Current_Hit_Time  = StandBy_Time;
  }
  
  Time2SleepAgain   = SleepAgainTime;
  TER_ServerHit_Time = Current_Hit_Time + 5;

  /*--------------- PROTOCOL SELECTION ---------------------------------------*/
  if(Protocol > NIC_OD)Protocol = 0;
 
  
}