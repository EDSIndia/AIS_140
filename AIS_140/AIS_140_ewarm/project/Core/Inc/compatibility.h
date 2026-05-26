#ifndef __COMPATIBILITY_H
#define __COMPATIBILITY_H

#include "stm32c5xx_hal.h"
#include "stm32c5xx_hal_rtc.h"
#include "main.h"
/*------------------------------------------------------------------
  SPL Compatibility
------------------------------------------------------------------*/

typedef enum
{
    RESET = 0,
    SET = !RESET
} FlagStatus;

/*------------------------------------------------------------------
  RTC Compatibility
------------------------------------------------------------------*/

typedef hal_rtc_date_t RTC_DateTypeDef;
typedef hal_rtc_time_t RTC_TimeTypeDef;

/*------------------------------------------------------------------
  Enable / Disable
------------------------------------------------------------------*/

#ifndef ENABLE
#define ENABLE  1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

/*------------------------------------------------------------------
  RTC Backup Register Compatibility
------------------------------------------------------------------*/

#define RTC_ReadBackupRegister(x)         READ_REG(x)
#define RTC_WriteBackupRegister(x,val)    WRITE_REG(x,val)

#define RTC_BKP_DR0    TAMP->BKP0R
#define RTC_BKP_DR1    TAMP->BKP1R
#define RTC_BKP_DR2    TAMP->BKP2R
#define RTC_BKP_DR3    TAMP->BKP3R
#define RTC_BKP_DR4    TAMP->BKP4R
#define RTC_BKP_DR5    TAMP->BKP5R
#define RTC_BKP_DR6    TAMP->BKP6R
#define RTC_BKP_DR7    TAMP->BKP7R

#endif