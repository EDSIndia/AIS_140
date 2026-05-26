/**
  ******************************************************************************
  * @file           : mx_usart1.h
  * @brief          : Header for mx_usart1.c file.
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
#ifndef MX_USART1_H
#define MX_USART1_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_ll.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USE_USART1

/** Primary aliases for USART1_RX pin */
#define GPRS_RX_PORT                          GPIOA
#define GPRS_RX_PIN                           LL_GPIO_PIN_10

/** Primary aliases for USART1_TX pin */
#define GPRS_TX_PORT                          GPIOA
#define GPRS_TX_PIN                           LL_GPIO_PIN_9

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/******************************************************************************/
/* Exported functions for UART in LL layer */
/******************************************************************************/
/**
  * @brief mx_usart1_uart init function
  * This function configures the hardware resources used in this example
  * @retval pointer to handle or NULL in case of failure
  */
USART_TypeDef *mx_usart1_uart_init(void);

/**
  * @brief  De-initialize mx_usart1_uart instance and return it.
  * @retval None
  */
void mx_usart1_uart_deinit(void);

/******************************************************************************/
/*         USART1 global interrupt is managed directly in user code.          */
/******************************************************************************/

/* void USART1_IRQHandler(void); */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_USART1_H */
