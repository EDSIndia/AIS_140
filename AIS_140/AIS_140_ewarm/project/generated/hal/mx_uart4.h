/**
  ******************************************************************************
  * @file           : mx_uart4.h
  * @brief          : Header for mx_uart4.c file.
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
#ifndef MX_UART4_H
#define MX_UART4_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_ll.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USE_UART4

/** Primary aliases for UART4_TX pin */
#define RS232_TX_PORT                         GPIOC
#define RS232_TX_PIN                          LL_GPIO_PIN_10

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/******************************************************************************/
/* Exported functions for UART in LL layer */
/******************************************************************************/
/**
  * @brief mx_uart4_uart init function
  * This function configures the hardware resources used in this example
  * @retval pointer to handle or NULL in case of failure
  */
USART_TypeDef *mx_uart4_uart_init(void);

/**
  * @brief  De-initialize mx_uart4_uart instance and return it.
  * @retval None
  */
void mx_uart4_uart_deinit(void);

/******************************************************************************/
/*          UART4 global interrupt is managed directly in user code.          */
/******************************************************************************/

/* void UART4_IRQHandler(void); */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_UART4_H */
