/******************** (C) COPYRIGHT 2018 EDS INDIA******************************
* File Name     : GNSS.c   	              		       	               *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file has all subroutibnes for GNSS functions            *
* Revision	: Rev0			                                       *
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "Main.h"

static void Get_Satellite_IRNSS(void);
static float DegMin_DecDeg(float);
static void Check_Fence(void);
   
/*----- END OF DATA TYPES & VARIABLES ----------------------------------------*/
/*******************************************************************************
* Function Name : SWITCH_MODULE_L89
* Description   : SWITCH ON/OFF L89 MODULE 
* Input         : REQUEST STATE 
* Output        : None
* Return        : None
*******************************************************************************/
void MODULE_L89(uint8_t state)
{
  switch(state)
  {
    case ENABLE       :  GNSS_ENB; CMD_Time = 3; RUN_GNSS; Flag_GNSS_ENB = SET; break;                    /* ENABLE MODULE L89  */
    case DISABLE      :  if(Flag_IRNSSFix == SET && Flag_SleepMode == RESET)break;   // Do not disable if fixed
                         GNSS_DSB; Flag_IRNSSFix = RESET; Flag_GNSS_ENB = RESET; GPS_Visible_Satellite = 0;IRNSS_Visible_Satellite = 0;break;                                       /* DISABLE MODULE L89 */
    default           :  GNSS_ENB; CMD_Time = 3; Flag_GNSS_ENB = SET;break;
  }
  
  IRNSS_FailTime = 0;
}

/*******************************************************************************
* Function Name : MODULE_L89_FLR
* Description   : 1PPS FALSE LED RESET 
* Input         : REQUEST STATE 
* Output        : None
* Return        : None
*******************************************************************************/
void MODULE_L89_FLR(void)
{ 
  GNSS_PutString("$PSTMPPS,2,8,3");
  mSec_Delay(200);
}

void IRNSS_ENB(void)
{
  GNSS_PutString("$PSTMSETPAR,1201,4088535F");
  mSec_Delay(200);
  GNSS_PutString("$PSTMSAVEPAR");
  mSec_Delay(200);
  GNSS_PutString("$PSTMSETCONSTMASK,1033"); 
}
/*******************************************************************************
* Function Name : GNSS USART Initialization
* Description   : Initialization USART communication
* Input         : None
* Output        : None
* Return        : None
*******************************************************************************/
void INIT_USART_GNSS(void)
{
    hal_gpio_config_t                    GPIO_InitStructure;
    ll_usart_config_t                   USART_InitStructure;
    
    GPIO_PinAFConfig(GNSS_PORT,GPIO_PinSource2,GPIO_AF_1);                      // PIN CONNECTED TO GNSS RX
    GPIO_PinAFConfig(GNSS_PORT,GPIO_PinSource3,GPIO_AF_1);                      // PIN CONNECTED TO GNSS TX
       
    /* CONFIGURE USART2_Tx AS ALTERNATE FUNCTION PUSH-PULL USED TO TRANSMIT TO GPS */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                                // PIN CONFIGURED AS ALTERNATE FUNCTION 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                           // PIN MAX SPEED IS S50 MHz                            
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                              // PIN CONFIGURED AS OPEN DRAIN 
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;                           // NO PULL UP / PULL DOWN 
    GPIO_InitStructure.GPIO_Pin   = GNSS_Tx ;                                   // ONLY TRANSMIT FROM GNSS
    GPIO_Init(GNSS_PORT, &GPIO_InitStructure);                                  // INITIALIZE GNSS PORT
    
    
    /*---------- CONFIGURE USART2_Rx AS ALTERNATE FUNCTION -------------------*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                                // INTERNALLY PULLED UP
    GPIO_InitStructure.GPIO_Pin = GNSS_Rx ;                                     // ONLY RECEIVE FROM GNSS
    GPIO_Init(GNSS_PORT, &GPIO_InitStructure);                                  // INITIALIZE GNSS PORT
    
    
    /* USART2 configuration - for GNSS-----------------------------------------*/
    /* USART2 configured as follow:
          - BaudRate = Define in Board Defination
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);                      // ENABLE USART CLOCK
    USART_DeInit(GNSS_USART);                                                   // RESET TO DEFAULT STATE 

    USART_InitStructure.USART_BaudRate   = BAUD_RATE_GNSS;                              // SET GPS BAUDRATE => DEFINED IN HEADER FILE 
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                 // 8 BIT
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;                    // 1 STOP BIT
    USART_InitStructure.USART_Parity     = USART_Parity_No;                     // PARITY NONE
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// HARDWARE FLOW CONTROL NONE
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;             // TX & RX ENABLED 

    /*---------- INITIALIZE USART 1 ------------------------------------------*/
    USART_Init(GNSS_USART, &USART_InitStructure);                               // INITIALIZE USART 2

    /*---------- ENABLE USART2 RECEIVE INTERRUPT -----------------------------*/
    USART_ITConfig(GNSS_USART, USART_IT_RXNE, ENABLE);                          // ENABLE RECEIVE INTERRUPT

    /*---------- ENABLE USART2 -----------------------------------------------*/
    USART_Cmd(GNSS_USART, ENABLE);                                              // ENABLE GPS USART

   
  MODULE_L89_FLR();
}
/*******************************************************************************
* Function Name : GNSS_PutString
* Description   : Send data to GNSS Modem and add 13,10 at the end
* Input         : string
* Output        : None
* Return        : None
*******************************************************************************/
void GNSS_PutString(char *s)
{
  mSec_Delay(10);
  while (*s != '\0')
  {
   while (USART_GetFlagStatus(GNSS_USART, USART_FLAG_TXE) == RESET);		// WAIT FOR COMPLETE TRANSMISSION TILL FLAG IS 0
     
   GNSS_USART->TDR = (*s & (uint16_t)0x01FF);                                    // SEND NEXT STRING VALUE
   s ++;                                                                        // INCREMENT POINTER 
   mSec_Delay(2);
  };
  mSec_Delay(5);
  
  while (USART_GetFlagStatus(GNSS_USART, USART_FLAG_TXE) == RESET);	        
  GNSS_USART->TDR = (13 & (uint16_t)0x01FF);					// SEND DATA BYTE 13<CR>
  while (USART_GetFlagStatus(GNSS_USART, USART_FLAG_TXE) == RESET);	                  
  GNSS_USART->TDR = (10 & (uint16_t)0x01FF);					// Send Data Byte 10<LF>
}


/*******************************************************************************
* Function Name  : Get_Location
* Description    : Get current time, location, Speed, heading, & date
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_Location(void)
{
 static uint8_t
 SPD_CNT = 0;
 
 Flag_IRNSSFix = RESET;
 uint16_t
 ChkSum,
 Get_ChkSum,
 TempLength,
 GP_Counter;
 
 FlagStatus Flag_AngleHit;
 /*---------- LOCAL VARIABLES ------------------------------------------------*/ 
  
 char *Ptr_Start,*Ptr_End,*Ptr;                                                 // LOCAL POINTER VARIABLES 
 char LOC[14],temp_buffer[10];
 
 Current_Angle = 0.0;
 Temp_Dist = 0;
 ChkSum     = 0;
 Get_ChkSum = 0;
 TempLength = 0;
 GP_Counter = 0;
 Ptr_Start  = NULL; 
 Ptr_End    = NULL;  
 Ptr        = NULL;
 
 loc_float = 0.0;
 memset(LOC,0,14);
 memset(temp_buffer,0,10);
 Flag_AngleHit = RESET;

 Flag_ValidGNSSString = RESET;                                                  // RESET STRING STATUS 

 Mems_Last_Speed = Current_Speed;

 /*---------- CALCULATE CHECKSUM ---------------------------------------------*/
 ChkSum ^= 'G';ChkSum ^= Rcv_IR_RMC[2];ChkSum ^= 'R';ChkSum ^= 'M';ChkSum ^= 'C';ChkSum ^= ',';
 
 Ptr = strchr(Rcv_IR_RMC,'*');                                                // CHECK '*' EOF
 if(Ptr)
  {
   Ptr += 3;*Ptr = 0;                                                     // BREAK STRING AFTER CHECKSUM
   Ptr -= 2;                                                                 // SET POINTER TO CHECKSUM FIRST CHARACTER 
   Get_ChkSum = CONVERTHEX(*Ptr) *16 + CONVERTHEX(*(Ptr+1));              // EXTRACT CHECKSUM VALUE FROM STRING         
   GP_Counter = strlen(Rcv_IR_RMC) - 3;                                          // FIND LENGTH OF STRING EXCLUDING ( EOF + 2 Chksum)
   for(TempLength = 7; TempLength < GP_Counter;TempLength++)
   ChkSum ^= Rcv_IR_RMC[TempLength];                                             // CALCULATE CHECKSUM ENTRIRE STRING 
   if(Get_ChkSum == ChkSum)Flag_ValidGNSSString = SET;                          // COMPARE CHECKSUM RECEIVED AND CALCULATED 
  };
 if(Flag_ValidGNSSString == SET)                                                // EXTRACT DATA AFTER CHECKSUM VERIFICATION        
 {
   /*----- TIME ---------------------------------------------------------------*/
   Ptr_Start = &Rcv_IR_RMC[7];
   memcpy((char*)IRNSSData.TIME,Ptr_Start,6);
   IRNSSData.TIME[6] = ',';IRNSSData.TIME[7] = 0;                                 // GET TIME FROM VALID RMC STRING
   
   char IRNSS_MM = (IRNSSData.TIME[2] - '0')*10 + (IRNSSData.TIME[3] - '0'); 
   char IRNSS_HH = (IRNSSData.TIME[0] - '0')*10 + (IRNSSData.TIME[1] - '0');
   if(RTC_Time.RTC_Minutes != IRNSS_MM || RTC_Time.RTC_Hours != IRNSS_HH)
   {
       RTC_Time.RTC_Hours = IRNSS_HH;     // EXTRACT HOUR VALUE FROM POINTER LOCATION IN BY 2
       RTC_Time.RTC_Minutes = IRNSS_MM;   // EXTRACT MINUTE VALUE FROM POINTER LOCATION IN BY 2
       RTC_Time.RTC_Seconds = (IRNSSData.TIME[4] - '0')*10 + (IRNSSData.TIME[5] - '0');   // EXTRACT SECOND VALUE FROM POINTER LOCATION IN BY 2    
       if(RTC_Time.RTC_Hours > 12)RTC_Time.RTC_H12 = RTC_H12_PM;
       else RTC_Time.RTC_H12 = RTC_H12_AM;
       RTC_SetTime(RTC_Format_BIN, &RTC_Time); 
   }
     
   /*----- GNSS VALID FLAG ---------------------------------------------------*/
   if(strstr(Rcv_IR_RMC,",A,"))
   {
     Ptr_End = strstr((char*)Rcv_IR_RMC,",A,");
   }
   else if(strstr(Rcv_IR_RMC,",D,"))
   {
     Ptr_End = strstr((char*)Rcv_IR_RMC,",D,");
   }
   if(Ptr_End)
   {
     Flag_IRNSSFix = SET;
     Ptr_End += 3;
    /*----- LATITUDE ----------------------------------------------------------*/
   
    Ptr_Start = Ptr_End;
    Ptr_End = strstr(Ptr_Start,",N,");
    if(Ptr_End)                                                                 // EXTRACT DATA USING LATITUDE DIRECTION 
      {
        if(IRNSSData.LAT[0] != 0)
        {
          memset(Last_Lat,0,15);
          if(Ptr = strstr(IRNSSData.LAT,",N"))strncpy(Last_Lat,IRNSSData.LAT,Ptr-IRNSSData.LAT);                                          // LAST VALID LATITUDE                     
        }
             
        Ptr_End  += 3;                                                          // INCREASE POINTER 
        TempLength = Ptr_End - Ptr_Start;                                       // EXTRACT DATA LENGTH 
        memset(LOC,0,14);                                                       // CLEAR LOCAL BUFFER 
        memcpy((char*)LOC,Ptr_Start,TempLength);                                // PASTE LATITUDE INTO LOCAL BUFFER
        
        memset(IRNSSData.LAT,0,15);
      
        Ptr_Start = strstr(LOC,".");                                            // SEARCH "DOT" IN LOCAL BUFFER 
        Ptr_Start -= 2;                                                         // SET POINTER 2 STEPS BACKWORD TO EXTRACT MM.MMMM FROM DDDMM.MMMM
        memset(temp_buffer,0,10);                                               // CLEAR TEMP BUFFER 
        ftoa(atof(Ptr_Start)/60.0,temp_buffer, 0, 7);                             // CONVERT FLOAT VALUE INTO CHARACTER 
        *Ptr_Start = 0;                                                         // SEPARATE DDD FROM DDDMM.MMMM
        strcpy(IRNSSData.LAT,LOC);                                               // COPY DDD INTO GNSSDATA 
        strcat(IRNSSData.LAT,temp_buffer);                                       // ADD ".DDDDDD" INTO GNSSDATA
        strcat(IRNSSData.LAT,",N,");                                             // ADD DIRECTION INTO GNSSDATA
      
      Geo_Lat = atof(IRNSSData.LAT);
      };
   
     /*----- LONGITUDE -------------------------------------------------------*/
     Ptr_Start = Ptr_End;
     Ptr_End = strstr(Ptr_Start,",E,");
     if(Ptr_End)                                                                // EXTRACT DATA USING LONGITUDE DIRECTION 
     {
       if(IRNSSData.LON[0] != 0)
        {
          memset(Last_Lon,0,15);
          if(Ptr = strstr(IRNSSData.LON,",E"))strncpy(Last_Lon,IRNSSData.LON,Ptr-IRNSSData.LON);                                          // LAST VALID LATITUDE                     
        }

       Ptr_End += 3;                                                            // INCREASE POINTER
       TempLength = Ptr_End - Ptr_Start;                                        // EXTRACT DATA LENGTH
       memset(LOC,0,14);                                                        // CLEAR LOCAL BUFFER 
       memcpy((char*)LOC,Ptr_Start,TempLength);                                 // PASTE LONGITUDE INTO LOCAL BUFFER 
       
       memset(IRNSSData.LON,0,15);
       
       Ptr_Start = strstr(LOC,".");                                             // SEARCH "DOT" IN LOCAL BUFFER 
       Ptr_Start -= 2;                                                          // SET POINTER 2 STEPS BACKWARD TO EXTRACT MM.MMMM FROM DDDMM.MMMM 
       memset(temp_buffer,0,10);                                                // CLEAR TEMP BUFFER 
       ftoa(atof(Ptr_Start)/60.0,temp_buffer, 0, 7);                              // CONVERT FLOAT VALUE INTO CHARACTER 
       *Ptr_Start = 0;                                                          // SEPARATE DDD FROM DDDMM.MMMM
       char LP = 0;
       while(LOC[LP] == '0')LP++;
       strcpy(IRNSSData.LON,&LOC[LP]);                                                // COPY DDD INTO GNSSDATA 
       strcat(IRNSSData.LON,temp_buffer);                                        // ADD ".DDDDDD" INTO GNSSDATA
       strcat(IRNSSData.LON,",E,");                                              // ADD DIRECTION INTO GNSSDATA
     
      Geo_Long = atof(IRNSSData.LON);
     };
     

    
     /*----- SPEED -----------------------------------------------------------*/
     Ptr_Start = Ptr_End;
     Ptr_End = strchr(Ptr_Start,',');                                           // FIND NEXT FIELD END LOCATION 
     if(Ptr_End)
      {
       Current_Speed = atof(Ptr_Start);                                         // EXTRACT SPEED IN KNOTS 
       Current_Speed *= 1.852;                                                  // CONVERT TO KM/H
       memset((char*)IRNSSData.SPEED,0,7);                                       // CLEAR GNSSDARA 
//       if(Current_Speed<10)ftoa(Current_Speed,IRNSSData.SPEED,1,1);              // CONVERT FLOAT INTO CHARACTERS
//       else if(Current_Speed<100)ftoa(Current_Speed,IRNSSData.SPEED,2,1);        // CONVERT FLOAT INTO CHARACTERS
//       else 
       ftoa(Current_Speed,IRNSSData.SPEED,3,1);                             // CONVERT FLOAT INTO CHARACTERS
       strcat(IRNSSData.SPEED,",");                                              // SEPARATER 

       Avg_Speed += Current_Speed;
       SPD_CNT++;
       if(SPD_CNT > 4)
       {
         Avg_Speed = Avg_Speed/5;
         SPD_CNT = 0;
         Flag_GetStatus = SET;
       }
       if(Current_Speed < 5)Mems_Last_Speed = 0;
      };
     
     /*----- DIRECTION -------------------------------------------------------*/
     Ptr_Start = Ptr_End + 1;
     Ptr_End = strchr(Ptr_Start,',');
     if(Ptr_End)
      {
        if(Current_Speed > 3)
        {
           //TempLength = Ptr_End - Ptr_Start;                                        // EXTRACT DATA LENGTH 
           memset((char*)IRNSSData.DIRECTION,0,7);                                   // CLEAR GNSSDATA
           loc_float = atof(Ptr_Start);
//           if(loc_float < 10)ftoa(loc_float,IRNSSData.DIRECTION,1,1);              // CONVERT FLOAT INTO CHARACTERS
//           else if(loc_float < 100)ftoa(loc_float,IRNSSData.DIRECTION,2,1);        // CONVERT FLOAT INTO CHARACTERS
//           else 
             ftoa(loc_float,IRNSSData.DIRECTION,3,2);                             // CONVERT FLOAT INTO CHARACTERS
           strcat(IRNSSData.DIRECTION,","); 
           Current_Angle = atof(IRNSSData.DIRECTION);
        }
        else 
        {
          if(IRNSSData.DIRECTION[0] == '\0')sprintf(IRNSSData.DIRECTION,"000.00,");
        }
      };
     
     /*----- DATE ------------------------------------------------------------*/
     Ptr_Start = Ptr_End + 1;
     Ptr_End = strchr(Ptr_Start,',');
     if(Ptr_End)                                                                // EXTRACT CURRENT DATE 
      {
       TempLength = Ptr_End - Ptr_Start;
       memcpy((char*)IRNSSData.DATE,Ptr_Start,TempLength);               
       IRNSSData.DATE[TempLength + 1] = IRNSSData.DATE[TempLength-1];
       IRNSSData.DATE[TempLength] = IRNSSData.DATE[TempLength-2];
       IRNSSData.DATE[TempLength-2] = '2';                                       // MODIFIED AS PER DDMMYYYY FORMAT                        
       IRNSSData.DATE[TempLength-1] = '0';
       IRNSSData.DATE[TempLength + 2] = ',';
       IRNSSData.DATE[TempLength + 3] = 0;
       
       if(Flag_ProgDate)
        {
          Flag_ProgDate = RESET;
          RTC_Date.RTC_Date = (IRNSSData.DATE[0] - '0')*10 + IRNSSData.DATE[1]- '0';
          RTC_Date.RTC_Month = (IRNSSData.DATE[2] - '0')*10 + IRNSSData.DATE[3]- '0';
          RTC_Date.RTC_Year = (IRNSSData.DATE[6] - '0')*10 + IRNSSData.DATE[7]- '0';
          RTC_SetDate(RTC_Format_BIN,&RTC_Date);
        }
      };
      
     
   }
   else Flag_IRNSSFix = RESET;
   
   Get_Satellite_IRNSS();
   if(Vehicle.Ignition[0] == 'I')Check_Fence();
   
   
/******************************* CALCULATE TRIP METER *************************/
     /*----- ANGLE BASED HIT CALCULATION -------------------------------------*/ 
     loc_float = abs((int)Current_Angle - (int)Last_Angle);                                              // MEASURE ANGLE DIFFERENCE 
     if(((Last_Speed > 5.0) || (Current_Speed > 5.0)) && Vehicle.Ignition[0] == 'I' && loc_float > 30.0) // CHECK CONDITION FOR ANGLE BASED HITTING
     {
       Last_Angle = Current_Angle; 
       Flag_AngleHit = SET;                                                     // SAVE HIT LAT & LONG
            
       Set_TCPData(TER);                                                        // SET TCP SERVER DATA for Angle
       if(Flag_PKTErr == RESET)Get_RAMBackup(TER);
       if(Vehicle.Ignition[0] == 'I')TER_ServerHit_Time = TER_HitTime-1;        // RELOAD SERVER HIT TIME 
       else TER_ServerHit_Time = Current_Hit_Time-1;
      };   
   
      /*----- TRIP METER CALCULATION ------------------------------------------*/
     if(Old_Lat == 0.0){Old_Lat = Geo_Lat;Old_Long = Geo_Long;}
     else Temp_Dist = Calc_Distance(Geo_Lat,Geo_Long,Old_Lat,Old_Long);
    
      
      if((Temp_Dist > 70 || Flag_AngleHit) && (IRNSSData.LAT[0] != 0) && (Vehicle.Ignition[0] == 'I')) // INCREMENT ODOMETER ONLY WHEN IGINITION ON & LAT, LONG IS VALID
      {
         Flag_AngleHit = RESET; 
         MTR = MTR + Temp_Dist;                                      // CALCULATE TOTAL DISTANCE TRAVELLED IN METER
         Old_Lat = Geo_Lat;Old_Long = Geo_Long;                                 // SAVE LAST LAT, LONG COORDINATES FROM WHICH DISTANCE WAS CALCULATED 
         
         if(MTR > 1000)
         {
          Temp_Dist = MTR / 1000; 
          KM  += Temp_Dist;                                                      // TRIP METER IN KM
          MTR -= Temp_Dist * 1000;                                              // DISTANCE TRAVELLED IN METER
         }             
      };
      Last_Speed = Current_Speed;
   
     if((Vehicle.Ignition[0] == 'I') && (Current_Speed > (float)Speed_Limit) && (Speed_Limit > 4) && (Current_Speed < 180) && (Flag_OverSpeed == RESET))
     {
       OVS_Buzz_Timer = 0;
       Flag_OverSpeed = SET; 
       OVS_Time = OVS_DeadTime;//300; 
       Flag_OVS = SET;
       
       Flag_MAIN_PKT = SET;
       Flag_TER_PKT = SET;
       TER_ServerHit_Time = TER_HitTime + 2;
       ServerHit_Time = 2; 
       
       memset(OS_Value,0,7); 
       memcpy(OS_Value,IRNSSData.SPEED,7);
       memset(OS_LAT,0,15);
       memset(OS_LON,0,15);
       memcpy(OS_LAT,IRNSSData.LAT,15);
       memcpy(OS_LON,IRNSSData.LON,15);
     }
     
     // TOW CHECK 
    if((Vehicle.Ignition[0] == 'F') && (Current_Speed >  10.0))
    {
      Check_StateCNT = 0;
      Check_TowCNT ++;
      if(Check_TowCNT > 4)
      {
        Check_TowCNT = 0;
        Flag_TOW = SET;
        Flag_OFFGNSS = RESET;
        memset(OFF_LAT,0,15);
        memset(OFF_LON,0,15);
      }
    }
    else 
    {
      Check_TowCNT = 0;
      Check_StateCNT++;
      if(Check_StateCNT > 4)
      {
        Check_StateCNT = 0;
        Flag_TOW = RESET;
      }
      
    }
 }// end of valid flag
 else Flag_IRNSSFix = RESET;
 
 if(Flag_TOW == SET)
 {
    memcpy(OFF_LAT,IRNSSData.LAT,strlen(IRNSSData.LAT));
    memcpy(OFF_LON,IRNSSData.LON,strlen(IRNSSData.LON));
 }
 
}
/*******************************************************************************
* Function Name  : Get_Satellite_TRNSS
* Description    : Get used satellite, HDOP, ALTITUDE & PDOP
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Get_Satellite_IRNSS(void)
{
 char *Ptr_Start,*Ptr_End;
 short TempLength = 0;
 float loc_float, ALT;
 
 loc_float = 0.0;
 Ptr_Start = 0; Ptr_End = 0;
 
  if(Flag_IRNSSFix)                                                               // EXTRACT DATA ONLY AFTER GPS/GNSS FIX 
  {
  /***************************** GGA STRING *************************************/ 
  Ptr_Start = strstr((char*)Rcv_IRNGGA,",E,");
  if(Ptr_Start)                                  
  {
     Ptr_Start +=3;
   
     /*----- NUMBER OF USED SATELLITE ----------------------------------------*/
     Ptr_Start = strchr(Ptr_Start,',') + 1;
     IRNSSData.Used_Satellite =  atoi(Ptr_Start);
     
     
     /*----- HDOP VALUE ------------------------------------------------------*/
     Ptr_Start = strchr(Ptr_Start,',') + 1;
     Ptr_End = strchr(Ptr_Start,',');
     //TempLength = Ptr_End - Ptr_Start;
     loc_float = atof(Ptr_Start);
     if(loc_float > 10.0)loc_float = 9.9;
     ftoa(loc_float,IRNSSData.HDOP,1,1); 
     
//     memset((char*)GNSSData.HDOP,0,5);
//     if(TempLength < 5)memcpy((char*)GNSSData.HDOP,Ptr_Start,TempLength);
//     else memcpy((char*)GNSSData.HDOP,Ptr_Start,4);
     
     /*---- ALTITUDE ---------------------------------------------------------*/ 
     Ptr_Start =Ptr_End+1;
     Ptr_End = strchr(Ptr_Start,',');
     if(Ptr_End)
      {
       ALT = atof(Ptr_Start);
       //TempLength = Ptr_End - Ptr_Start;
       
       memset((char*)IRNSSData.Altitude,0,10);
       sprintf(IRNSSData.Altitude,"%5.1f",ALT);
       
      }
     }
  
  /***************************** GSA STRING *************************************/ 
  Ptr_Start = strstr((char*)Rcv_GSA,",");
  if(Ptr_Start )
  {
    Ptr_Start += 1;
    Ptr_Start = strstr(Ptr_Start,",");
    Ptr_Start += 1;
    
    TempLength = 13;
    while(TempLength){Ptr_Start = strstr(Ptr_Start,",");Ptr_Start += 1;TempLength--;}
    
     /*----- PDOP VALUE ------------------------------------------------------*/
    Ptr_End = strchr(Ptr_Start,',');
    if(Ptr_End)
     {
       //TempLength = Ptr_End - Ptr_Start;
       memset((char*)IRNSSData.PDOP,0,5);    
       loc_float = atof(Ptr_Start);
       if(loc_float > 10.0)loc_float = 9.9;
       ftoa(loc_float,IRNSSData.PDOP,1,1); 
       //if(TempLength < 5)memcpy((char*)GNSSData.PDOP,Ptr_Start,TempLength);     
      // else memcpy((char*)GNSSData.PDOP,Ptr_Start,4);
     }
  }
  
 };                                                                                                   
/**************************************************************************************************************/
}



/*******************************************************************************
* Function Name  : Calc_Distance
* Description    : This Function Calculate the Distance between Two Lat & Long
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int Calc_Distance(float LT1, float LN1, float LT2, float LN2)
{
  /* LOCAL VARIABLES */
  float lat1, lon1, lat2, lon2;                                                 
  float dist;                                                           
  int Dist_Mts;

  if(LT1 < 100.0){lat1 = LT1; lon1 = LN1;}                                      // NO CHANGE IF ALREADY IN DECIMAL DEGREE
  else{lat1 = DegMin_DecDeg(LT1);  lon1 = DegMin_DecDeg(LN1);}                                                                              // CONVERT LAT LONG INTO DECIMAL DEGREE minus'-' coz of West 'W' direction 
 
  if(LT2 < 100.0){lat2 = LT2; lon2 = LN2;}                                      // NO CHANGE IF ALREADY IN DECIMAL DEGREE
  else{lat2 = DegMin_DecDeg(LT2); lon2 = DegMin_DecDeg(LN2);}                                                                             // CONVERT LAT LONG INTO DECIMAL DEGREE                                                               /* CONVERT DEGREE MINUTE DATA TO DECIMAL DEGREE */

  
  if(lat1 == lat2 && lon1 == lon2)return (0);                                   // RETURN 0 IF SAME LOCATION

  /* CONVERT COORDINATES FROM DECIMAL DEGREES TO RADIANS */
  lat1*=0.0174532925;                                                           //  Pi/180 = 0.0174532925
  lat2*=0.0174532925;
  lon1*=0.0174532925;
  lon2*=0.0174532925;

  dist = acos(sin(lat1)*sin(lat2) + cos(lat1)* cos(lat2) * cos(lon2-lon1)) * 6371; // CALCULATE DISTANCE IN KM AS PER HAVERSHINE FORMULA
  if(dist > 100)dist = 100;
  Dist_Mts = (int)(dist * 1000);                                                // RETURN DISTANCE IN METERS

return (Dist_Mts);
}
/*******************************************************************************
* Function Name  : DegMin_DecDeg
* Description    : This Function converts Degree Minute to Decimal Degree
* Input          : GEO Coordinate in degree minute
* Output         : GEO Coordinate in decimal degree
* Return         : None
*******************************************************************************/
static float DegMin_DecDeg(float Lat_Lon)
{
 unsigned char Deg;
 float Min,DEC_DEG;
 
 Deg    = (int)Lat_Lon/100;                                                     // EXTRACT DEGREE  ONLY 
 Min    = Lat_Lon - (float)Deg*100.0;                                           // EXTRACT MINUTE  ONLY  
 Min    = Min/60.0;                                                             // CONVERT MINUTE INTO DEGREE
 DEC_DEG = (float)Deg + Min;                                                    // GET EQUIVALENT DECIMAL DEGREE 
 
 return(DEC_DEG);                                                               // RETURN DECIMAL DEGREE
}
/*******************************************************************************
* Function Name  : Check_Fence
* Description    : THIS FUNCTION USED FOR CHECK CURRENT POSITION AS PER GIVEN FENCE AREAS
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Check_Fence(void)
{
   unsigned char 
   GP_Counter = 0,
   NOP = 0,
   FA  = 0,                                                                      // FENCE AREA 
   FP  = 0,                                                                      // FENCE POINT 
   AFP = 0, // Previous FENCE POINT 
   Max_Point = 0,
   FS  = OUT;
  // Flag_Inside = RESET;
   
   for(FA = 0; FA < 10; FA++)
   { 
     NOP = 0;
     for(GP_Counter = 0; GP_Counter < 10; GP_Counter++)
     {
       if((GF[FA].LAT[GP_Counter] > 0) && (GF[FA].LON[GP_Counter] > 0))NOP++;
     }
     if(NOP > 2)
     {
       FS  = OUT;
       Max_Point = 0;
       while(GF[FA].LAT[Max_Point] > 0.0)
       {
         Max_Point++;
         if(Max_Point > 9)break;
       }
       AFP = Max_Point - 1;
       
       for (FP = 0; FP < Max_Point; FP++) 
        {
          if((((GF[FA].LON[FP] < Geo_Long) && (GF[FA].LON[AFP] >= Geo_Long)) || ((GF[FA].LON[AFP] < Geo_Long) && (GF[FA].LON[FP] >= Geo_Long))) && ((GF[FA].LAT[FP] <= Geo_Lat) || (GF[FA].LAT[AFP] <= Geo_Lat))) 
           {
              FS ^= (GF[FA].LAT[FP]+(Geo_Long-GF[FA].LON[FP])/(GF[FA].LON[AFP]-GF[FA].LON[FP])*(GF[FA].LAT[AFP]-GF[FA].LAT[FP]) < Geo_Lat); 
           }
          AFP = FP;    
        }
       
         if((GF[FA].Last_FS == OUT) && (FS == IN))                              // GENERATE PACKET FOR IN FENCE 
         {
           Flag_GEOEntry   = SET; 
           FenceArea = GF[FA].ID; 
           GF[FA].Last_FS = FS;
           Flag_MAIN_PKT = SET;
           Flag_TER_PKT = SET;
           Gen_PKT(); 
           if(Protocol == HP_PC)
           {     
             OP2_START;
             GEO_Buzz_Time = 200;
           }
           break;
         }
         else if((GF[FA].Last_FS == IN) && (FS == OUT))
         {
           Flag_GEOExit = SET;  
           FenceArea = GF[FA].ID; 
           GF[FA].Last_FS = FS;
           Flag_MAIN_PKT = SET;
           Flag_TER_PKT = SET;
           Gen_PKT();
           if(Protocol == HP_PC)
           {
             OP2_START;
             GEO_Buzz_Time = 200;
           }
           break;
         }        // GENERATE PACKET FOR OUT FENCE 
         
       GF[FA].Last_FS = FS;
     }
   } 
    
}