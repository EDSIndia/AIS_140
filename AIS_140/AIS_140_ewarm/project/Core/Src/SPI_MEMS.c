/******************** (C) COPYRIGHT 2018 EDS INDIA *****************************
* File Name     : SPI_MEMS.c					               *
* Author        : DILEEP SINGH                                                 *
* Date          : 24/01/2018                                                   *
* Description   : This file defines SPI MEMS & deals with its functions        *
* Revision	: Rev0			                                       *
*******************************************************************************/
#include "Main.h"

/* Exported macro ------------------------------------------------------------*/

#include "lsm6dsl_reg.h"
#include <string.h>


static axis3bit16_t data_raw_acceleration;
static axis3bit16_t data_raw_angular_rate;
//static axis1bit16_t data_raw_temperature;
//static float acceleration_mg[3];
static float angular_rate_mdps[3];
//static float temperature_degC;
static uint8_t whoamI, rst;

static int32_t read(uint8_t Reg, uint8_t *Bufp, uint16_t len);
static int32_t write(uint8_t Reg, uint8_t *Bufp,uint16_t len);
static uint8_t SPI_Mems_SendByte(uint8_t byte);


//static uint8_t LIS3DH_ReadReg(uint8_t , uint8_t* );
uint8_t LIS3DH_WriteReg(uint8_t Reg, uint8_t Data);

static void SPI_Mems_Write_Reg(uint8_t regAddr, uint8_t data);
//static uint8_t SPI_Mems_Read_Reg(uint8_t reg);
/*---------- ISM330 DEVICE READ WIRITE REGISTERS -----------------------------*/
lsm6dsl_ctx_t           dev_ctx;
lsm6dsl_reg_t           reg;


/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_MEMS_Init()
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD,ENABLE);     // ENABLE CLOCKS GPIO
 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);                           // ENABLE CLOCK SPI2
  
  
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_0);                           // ASSIGN ALTERNATE FUNCTION CLK                             
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_0);                           // ASSIGN ALTERNATE FUNCTION MISO   
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_0);                           // ASSIGN ALTERNATE FUNCTION MOSI  
  
 
  /*---------- CONFIGURE SPI2 PINS: SCK, MISO and MOSI -----------------------*/
  GPIO_InitStructure.GPIO_Pin = PIN_MEMS_SCK | PIN_MEMS_MISO | PIN_MEMS_MOSI;                    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(PORT_MEMS_DATA, &GPIO_InitStructure);                                      
  
  /*---------- CONFIGURE SPI2 PINS: CS, INT1, INT2 ---------------------------*/
  GPIO_InitStructure.GPIO_Pin = PIN_MEMS_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(PORT_MEMS_CS, &GPIO_InitStructure);
  
  
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin   = PIN_MEMS_INT1; GPIO_Init(PORT_MEMS_INT1, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin   = PIN_MEMS_INT2; GPIO_Init(PORT_MEMS_INT2, &GPIO_InitStructure);
  /* Deselect the FLASH: Chip Select high */
  MEMS_OFF; 

  /* SPI2 configuration */
  SPI_I2S_DeInit(SPI2);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS       = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  SPI_InitStructure.SPI_FirstBit  = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);
  SPI_RxFIFOThresholdConfig(SPI2, SPI_RxFIFOThreshold_QF);
  /* Enable SPI1  */
  SPI_Cmd(SPI2, ENABLE);
}

uint8_t LIS3DH_WriteReg(uint8_t WriteAddr, uint8_t Data) {
  
  //To be completed with either I2c or SPI writing function
  SPI_Mems_Write_Reg(WriteAddr, Data);  
  return 1;
}
   
//uint8_t LIS3DH_ReadReg(uint8_t Reg, uint8_t* Data) 
//{
//  
//  //To be completed with either I2c or SPI reading function
//  *Data = SPI_Mems_Read_Reg( Reg );  
//  return 1;
//}  


static void SPI_Mems_Write_Reg(uint8_t regAddr, uint8_t data)
{

  // Select Mems Sensor: Chip Select low 
  MEMS_ON;

  // Send Register Address
  SPI_Mems_SendByte(regAddr);
  /* Write a byte into the MEMS Sensor Register*/
  SPI_Mems_SendByte(data);  

  /* Deselect Mems Sensor: Chip Select high */
  MEMS_OFF;  

}

//static uint8_t SPI_Mems_Read_Reg(uint8_t reg) 
//{
//  uint8_t Temp;
//
//  reg += 0x80; //reading procedure has to set the most significant bit
//  // Select Mems Sensor: Chip Select low 
//  MEMS_ON;
//
//  // Send Register Address
//  SPI_Mems_SendByte(reg);
//  /* Read a byte from the MEMS Sensor */
//  Temp = SPI_Mems_SendByte(0); // DUMMY_BYTE
//
//  /* Deselect Mems Sensor: Chip Select high */
//  MEMS_OFF;  
//
//  return Temp;
//}
/*******************************************************************************
* Function Name  : MEMS REGISTER WRITE
* Description    : WRITE A VALUE OF DEDICATED LENGTH INTO A DEDICATED REGISTER 
* Input          : REGISTER ADDRESS, READ BUFFER, DATA LENGTH
* Output         : NONE
* Return         : NONE
*******************************************************************************/
static int32_t write(uint8_t Reg, uint8_t *Bufp,uint16_t len)
{
  unsigned short i =0; 
  MEMS_ON;
  SPI_Mems_SendByte(Reg);
  while(i< len)
  {
  SPI_Mems_SendByte(*Bufp++);  
  i++;
  }
  MEMS_OFF;  
  return 0;
}
/*******************************************************************************
* Function Name  : MEMS REGISTER READ
* Description    : WRITE A VALUE OF DEDICATED LENGTH INTO A DEDICATED REGISTER 
* Input          : REGISTER ADDRESS, READ BUFFER, DATA LENGTH
* Output         : DATA FROM REQUESTED REGISTER 
* Return         : DATA
*******************************************************************************/
static int32_t read(uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
 unsigned short i = 0;
 Reg |= 0x80;

MEMS_ON;
SPI_Mems_SendByte(Reg);
 while(i < len)
 {
   *Bufp++ = SPI_Mems_SendByte(0); //Dummy_Byte  
   i++;
 }
  MEMS_OFF; 
  return 0;
}
/*******************************************************************************
* Function Name  : SPI_Mems_SendByte
* Description    : WRITE A BYTE INTO DATA REGISTER OF SPI2  
* Input          : 8 BIT DATA
* Output         : 8 BIT DATA FROM REQUESTED REGISTER 
* Return         : DATA
*******************************************************************************/
static uint8_t SPI_Mems_SendByte(uint8_t byte)
{
  int TimeOut = 1000;
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
  {
    if(Flag_mSec)
    {
      Flag_mSec = RESET;
      TimeOut --;
      if(TimeOut == 0)break;
    }
  }

  /* Send byte through the SPI1 peripheral */
  SPI_SendData8(SPI2, byte);

  /* Wait to receive a byte */
  
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if(Flag_mSec)
    {
      Flag_mSec = RESET;
      TimeOut --;
      if(TimeOut == 0)break;
    }
  };

  /* Return the byte read from the SPI bus */
  return SPI_ReceiveData8(SPI2);
}
/*******************************************************************************
* Function Name  : MEMS_INIT
* Description    : INITIALIZE LSM6DSL3 MEMS IC  
* Input          : NONE
* Output         : NONE 
* Return         : NONE
*******************************************************************************/
void MEMS_INIT(void)
{
  unsigned char TimeOut = 0;
  dev_ctx.write_reg = write;
  dev_ctx.read_reg = read;

  whoamI = 0;
  
  mSec_Delay(100);
  lsm6dsl_device_id_get(&dev_ctx, &whoamI);
  mSec_Delay(100);
  if((whoamI == LSM6DSL_ID ) || (whoamI == ISM330))
  {
  lsm6dsl_reset_set(&dev_ctx, PROPERTY_ENABLE);
  do {
    lsm6dsl_reset_get(&dev_ctx, &rst);
    if(Flag_Second == SET){Flag_Second = RESET; TimeOut++; if(TimeOut > 2)break;}
  } while (rst);
  
//  lsm6dsl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
//  lsm6dsl_xl_data_rate_set(&dev_ctx, LSM6DSL_XL_ODR_104Hz);//LSM6DSL_XL_ODR_12Hz5 
//  lsm6dsl_gy_data_rate_set(&dev_ctx, LSM6DSL_GY_ODR_104Hz);
//  lsm6dsl_xl_full_scale_set(&dev_ctx, LSM6DSL_2g);  //LSM6DSL_2g
//  lsm6dsl_gy_full_scale_set(&dev_ctx, LSM6DSL_2000dps);
//  lsm6dsl_xl_filter_analog_set(&dev_ctx, LSM6DSL_XL_ANA_BW_400Hz); //400
//  lsm6dsl_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);//100
//  lsm6dsl_gy_band_pass_set(&dev_ctx, LSM6DSL_HP_260mHz_LP1_STRONG);
  
    /*****************************************************/
//  lsm6dsl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
//  lsm6dsl_xl_data_rate_set(&dev_ctx, LSM6DSL_XL_ODR_6k66Hz); //6k66Hz
//  lsm6dsl_xl_full_scale_set(&dev_ctx, LSM6DSL_2g); 
//  lsm6dsl_xl_filter_analog_set(&dev_ctx, LSM6DSL_XL_ANA_BW_400Hz);
//  lsm6dsl_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);
  /******************************************************/
  
  /*******************************************************/
  lsm6dsl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsl_xl_data_rate_set(&dev_ctx, LSM6DSL_XL_ODR_6k66Hz);//LSM6DSL_XL_ODR_12Hz5 
  lsm6dsl_gy_data_rate_set(&dev_ctx, LSM6DSL_GY_ODR_6k66Hz);
  lsm6dsl_xl_full_scale_set(&dev_ctx, LSM6DSL_16g);  //LSM6DSL_2g
  lsm6dsl_gy_full_scale_set(&dev_ctx, LSM6DSL_2000dps);
  lsm6dsl_xl_filter_analog_set(&dev_ctx, LSM6DSL_XL_ANA_BW_400Hz); //400
  lsm6dsl_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);//100
  lsm6dsl_gy_band_pass_set(&dev_ctx, LSM6DSL_HP_260mHz_LP1_STRONG);
  
  LIS3DH_WriteReg(0x5C,0x00);
  if(Accel_WK_Thrs < 1.0)LIS3DH_WriteReg(0x5B,0x01); //0x02 //wake up threshold
  else if(Accel_WK_Thrs > 5.0)LIS3DH_WriteReg(0x5B,0x05); //0x02 //wake up threshold
  else LIS3DH_WriteReg(0x5B,(int)(Accel_WK_Thrs)); //0x02 //wake up threshold
  LIS3DH_WriteReg(0x58,0x80);
  mSec_Delay(4);
  LIS3DH_WriteReg(0x5E,0x20);

//  Flag_MEMSOK = SET; 
/********************************************************/

  Flag_MEMSOK = SET; 
  }
  else Flag_MEMSOK = RESET;
}
/*******************************************************************************
* Function Name  : GET MEMS DATA
* Description    : READ MEMS DATA FROM ITS REGISTER   
* Input          : NONE
* Output         : NONE 
* Return         : NONE
*******************************************************************************/
void GET_MEMSData(void)
{
   static uint8_t MEMS_Init = 6;
   static uint8_t Tilt_CNT = 0;
  
   FlagStatus Flag_Tilt = RESET;
   
   lsm6dsl_status_reg_get(&dev_ctx, &reg.status_reg);
   
   if(reg.status_reg.xlda)
    {
    lsm6dsl_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
     
     X = data_raw_acceleration.i16bit[0];
     Y = data_raw_acceleration.i16bit[1];
     Z = data_raw_acceleration.i16bit[2];
     
     x_g = (X *  488.0) / 1000000.0;  
     y_g = (Y *  488.0) / 1000000.0;
     z_g = (Z *  488.0) / 1000000.0;  //16g
     
     if(y_g < HA)Y= 0;
     
     //if(y_g > 5)y_g = 0;

     X= 0; Y= 0; Z= 0;
     
/************ Tilt Angle Calculation ************************************/
     roll = 180 * atan(x_g/sqrt(y_g * y_g + z_g * z_g)) / 3.14;     
     pitch = 180 * atan(y_g/sqrt(x_g * x_g + z_g * z_g)) / 3.14;   
     Py += pitch;
     Rx += roll; 
   
     counter2++;
     
     if(counter2 > 0) 
     { 
     P_Y = (Py/1);
     R_X = (Rx/1);
     Py = 0;Rx = 0;counter2 = 0;     
     }
   
     
     
    if(z_g > 0.0)
     {
       if(R_X < 0.0){R_X *= (-1);  /*R_X = 360 - R_X;*/}
       if(P_Y < 0.0){P_Y *= (-1);  /*P_Y = 360 - P_Y;*/}
     }
     else 
     {
       if(R_X < 0.0){R_X *= (-1); R_X = 180 - R_X;}
       else 
       {
         R_X = 180 - R_X;
       }
       
       if(P_Y < 0.0){P_Y *= (-1); P_Y = 180 - P_Y;}
       else P_Y = 180 - P_Y;
     }
     
     ACTUAL_RX = R_X;
     ACTUAL_PY = P_Y;
     
     
     if(R_X > OFFSET_X)
     {
       if((R_X - OFFSET_X) > TA)Flag_Tilt = SET; 
     }
     else 
     {
       if((OFFSET_X - R_X) > TA)Flag_Tilt = SET; 
     }

     if(P_Y > OFFSET_Y)
     {
       if((P_Y - OFFSET_Y) > TA)Flag_Tilt = SET; 
     }
     else 
     {
       if((OFFSET_Y - P_Y) > TA)Flag_Tilt = SET; 
     }
     

     if(Flag_Tilt == SET) Tilt_CNT++;
     else Tilt_CNT = 0;
//    if((R_X  > TA) || (P_Y > TA))
    if((Flag_Tilt == SET) && (Tilt_CNT > 24))//((P_Y > TA) || (z_g < 0.0))
    {
      Tilt_CNT = 25;
      if((Flag_TILT == RESET) && (Flag_TILT_Alert == RESET))
      {
        Flag_TILT  = SET; 
        if(Protocol == HP_PC)
        {
          Flag_TILT_Alert = SET;
          
          Flag_MAIN_PKT = SET;
          Flag_TER_PKT = SET;
          ServerHit_Time = 2;
          TER_ServerHit_Time = TER_HitTime + 2;
        }
        else 
        {
          Flag_TILT_Alert = SET;
          
          Flag_TER_PKT = SET;
          TER_ServerHit_Time = TER_HitTime + 2;         
        }

        /* comment cause of => not to generate packet right now only*/
      }   
    }
    else 
    {
      Flag_TILT = RESET;  
    }
     
/*******************************************************************************/

    if((z_g < 0.80 ) || (z_g > 1.2));//{while(Z_Time)Z_Time--;}//0.80 1.2
      else
      {         
       if(MEMS_Init)MEMS_Init--;
       else 
       {          
	  if((y_g  < -HA) && (Flag_HA == RESET) && (HA_HB == 0)) 
          {
            if((Current_Speed > Mems_Last_Speed) || (Mems_Last_Speed == 0.0))
            {
              if(Current_Hit_Time)
              {
                Flag_HA = SET;Flag_HB = RESET;//Flag_RT = RESET; 
                Flag_MAIN_PKT = SET;
                Flag_TER_PKT = SET;
                Gen_PKT();
                y_g = 0;x_g = 0;
                HA_HB = 10;
              }
            
            }
          } 
	else if((y_g  > HB) && (Flag_HB == RESET) && (HA_HB == 0))
          {
            if((Mems_Last_Speed  > Current_Speed) && (Current_Hit_Time > 0))
            {
              Flag_HB = SET; Flag_HA = RESET;//Flag_RT = RESET;
              Flag_MAIN_PKT = SET;
              Flag_TER_PKT = SET;
              Gen_PKT();
              y_g = 0;x_g = 0;
              HA_HB = 10;
            }
          }
	   
       }                                                                         // close else mems Initiazize                         	   
       }                                                                         // close else from z limit
   }                                                                             // acc reg read close 
  
  /********************** Read gyro values ******************************/
   if (reg.status_reg.gda)
    {
      /* Read Gyro field data */
      memset(data_raw_angular_rate.u8bit, 0x00, 3*sizeof(int16_t));
      lsm6dsl_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate.u8bit);

      angular_rate_mdps[0] = LSM6DSL_FROM_FS_2000dps_TO_mdps(data_raw_angular_rate.i16bit[0]);
      angular_rate_mdps[1] = LSM6DSL_FROM_FS_2000dps_TO_mdps(data_raw_angular_rate.i16bit[1]);
      angular_rate_mdps[2] = LSM6DSL_FROM_FS_2000dps_TO_mdps(data_raw_angular_rate.i16bit[2]);
      Gx =  angular_rate_mdps[0] / 10.0;
      Gy =  angular_rate_mdps[1] / 10.0;
      Gz =  angular_rate_mdps[2] / 10.0;

      if(Gx < 0.0)Gx = 0.0;
      if(Gy < 0.0)Gy = 0.0;
      //if(Gz < 0.0)Gz = 0.0;
      
      if(((Gz > RT) || (Gz < -RT)) && (Flag_RT == RESET) && (HA_HB == 0)) 
      {
        if(Current_Hit_Time)
        {
         Flag_RT  = SET;
         Flag_MAIN_PKT = SET;
         Flag_TER_PKT = SET;
         Gen_PKT();
         HA_HB = 10;
        }
      }
    }   // gda read end  

}                                                                                // close main function