/**
  ******************************************************************************
  * @file           : mx_spi2.h
  * @brief          : Header for mx_spi2.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_stm32c5xx_hal_drivers_license.md file
  * in the same directory as the generated code.
  * If no mx_stm32c5xx_hal_drivers_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_SPI2_H
#define MX_SPI2_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_ll.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USE_SPI2

/** Primary aliases for SPI2_MOSI pin */
#define SPI2_MOSI_PORT                        GPIOB
#define SPI2_MOSI_PIN                         LL_GPIO_PIN_15

/** Primary aliases for SPI2_MISO pin */
#define SPI2_MISO_PORT                        GPIOB
#define SPI2_MISO_PIN                         LL_GPIO_PIN_14

/** Primary aliases for SPI2_SCK pin */
#define SPI2_SCK_PORT                         GPIOB
#define SPI2_SCK_PIN                          LL_GPIO_PIN_13

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/******************************************************************************/
/* Exported functions for SPI in LL layer */
/******************************************************************************/
/**
  * @brief mx_spi2 init function
  * This function configures the hardware resources used in this example
  * @retval pointer to handle or NULL in case of failure
  */
SPI_TypeDef *mx_spi2_init(void);

/**
  * @brief  De-initialize spi2 instance and return it.
  */
void mx_spi2_deinit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MX_SPI2_H */
