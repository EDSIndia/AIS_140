/**
  ******************************************************************************
  * @file           : mx_usart1.c
  * @brief          : USART1 Peripheral initialization
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

/* Includes ------------------------------------------------------------------*/
#include "mx_usart1.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototype------------------------------------------------*/
/* Exported variables by reference--------------------------------------------*/

/* Exported function definition ----------------------------------------------*/

/******************************************************************************/
/* Exported functions for UART in LL layer */
/******************************************************************************/

USART_TypeDef *mx_usart1_uart_init(void)
{
  uint32_t reg_temp;

  /* Basic configuration */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  /* LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2); */ /* Configuration matches register reset state at startup. */

  LL_USART_ConfigAsyncMode(USART1);

  reg_temp = (LL_USART_DATAWIDTH_8_BIT | LL_USART_PARITY_NONE
              | LL_USART_DIRECTION_TX_RX | LL_USART_OVERSAMPLING_16);
  LL_USART_ConfigXfer(USART1, reg_temp, LL_USART_STOP_BIT_1);

  /* LL_USART_SetHWFlowCtrl(USART1, LL_USART_HWCONTROL_NONE); */ /* Configuration matches register reset state at startup. */

  /* LL_USART_SetPrescaler(USART1, LL_USART_PRESCALER_DIV1); */ /* Configuration matches register reset state at startup. */

  LL_USART_SetBaudRate(USART1, 144000000, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16,
                       115200);

  /* GPIO Clocks activation */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

  /**
    USART1 GPIO Configuration

    [GPIO Pin] ------> [Signal Name]

       PA10    ------>   USART1_RX
       PA9     ------>   USART1_TX
    **/

  /* Configure IO output speed (Low, Medium, High or Very-High) */
  /* LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW); */ /* Configuration matches register reset state at startup. */
  /* LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW); */ /* Configuration matches register reset state at startup. */

  /* Configure IO output type (Push-Pull or Open-Drain) */
  /* LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_10 | LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL); */ /* Configuration matches register reset state at startup. */

  /* Activate the Pull-up, Pull-down resistor, or No pull for the current IO */
  /* LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_NO); */ /* Configuration matches register reset state at startup. */
  /* LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO); */ /* Configuration matches register reset state at startup. */

  /* Configure the Alternate Function in current IO */
  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_7);
  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_7);

  /* Configure IO direction mode (Input, Output, Alternate or Analog) */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);

  /* Enable interrupt */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  LL_USART_Enable(USART1);
  return USART1;
}

void mx_usart1_uart_deinit(void)
{
  LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_USART1);
  LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_USART1);

  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_USART1);

  /* Disable interrupt */
  NVIC_DisableIRQ(USART1_IRQn);
  LL_USART_Disable(USART1);

  /* ### GPIO deinitialization of USART1: USART1_RX,USART1_TX ########################### */

  /* Configure IO in Analog Mode */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ANALOG);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ANALOG);

  /* Configure the default Alternate Function in current IO */
  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_0);
  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_0);

  /* Configure the default value for IO Speed */
  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW);

  /* Configure the default value IO Output Type */
  LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_10 | LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL);

  /* Deactivate the Pull-up and Pull-down resistor for the current IO */
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);

  /* Reset the IO output state */
  LL_GPIO_WriteOutputPin(GPIOA, LL_GPIO_PIN_10 | LL_GPIO_PIN_9, LL_GPIO_PIN_RESET);
}

/******************************************************************************/
/*         USART1 global interrupt is managed directly in user code.          */
/******************************************************************************/
/* void USART1_IRQHandler(void)
{
}
  */
