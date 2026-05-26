/**
  ******************************************************************************
  * @file           : mx_uart4.c
  * @brief          : UART4 Peripheral initialization
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
#include "mx_uart4.h"

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

USART_TypeDef *mx_uart4_uart_init(void)
{
  uint32_t reg_temp;

  /* Basic configuration */

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
  /* LL_RCC_SetUARTClockSource(LL_RCC_UART4_CLKSOURCE_PCLK1); */ /* Configuration matches register reset state at startup. */

  LL_USART_ConfigAsyncMode(UART4);

  reg_temp = (LL_USART_DATAWIDTH_8_BIT | LL_USART_PARITY_NONE
              | LL_USART_DIRECTION_TX_RX | LL_USART_OVERSAMPLING_16);
  LL_USART_ConfigXfer(UART4, reg_temp, LL_USART_STOP_BIT_1);

  /* LL_USART_SetHWFlowCtrl(UART4, LL_USART_HWCONTROL_NONE); */ /* Configuration matches register reset state at startup. */

  /* LL_USART_SetPrescaler(UART4, LL_USART_PRESCALER_DIV1); */ /* Configuration matches register reset state at startup. */

  LL_USART_SetBaudRate(UART4, 144000000, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16,
                       115200);

  /* GPIO Clocks activation */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);

  /**
    UART4 GPIO Configuration

    [GPIO Pin] ------> [Signal Name]

       PC11    ------>   UART4_RX
       PC10    ------>   UART4_TX
    **/

  /* Configure IO output speed (Low, Medium, High or Very-High) */
  /* LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_11, LL_GPIO_SPEED_FREQ_LOW); */ /* Configuration matches register reset state at startup. */
  /* LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW); */ /* Configuration matches register reset state at startup. */

  /* Configure IO output type (Push-Pull or Open-Drain) */
  /* LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_11 | LL_GPIO_PIN_10, LL_GPIO_OUTPUT_PUSHPULL); */ /* Configuration matches register reset state at startup. */

  /* Activate the Pull-up, Pull-down resistor, or No pull for the current IO */
  /* LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_11, LL_GPIO_PULL_NO); */ /* Configuration matches register reset state at startup. */
  /* LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_10, LL_GPIO_PULL_NO); */ /* Configuration matches register reset state at startup. */

  /* Configure the Alternate Function in current IO */
  LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_11, LL_GPIO_AF_8);
  LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_10, LL_GPIO_AF_8);

  /* Configure IO direction mode (Input, Output, Alternate or Analog) */
  LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_11, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);

  /* Enable interrupt */
  NVIC_SetPriority(UART4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(UART4_IRQn);

  LL_USART_Enable(UART4);
  return UART4;
}

void mx_uart4_uart_deinit(void)
{
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_UART4);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_UART4);

  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_UART4);

  /* Disable interrupt */
  NVIC_DisableIRQ(UART4_IRQn);
  LL_USART_Disable(UART4);

  /* ### GPIO deinitialization of UART4: UART4_RX,UART4_TX ########################### */

  /* Configure IO in Analog Mode */
  LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_11, LL_GPIO_MODE_ANALOG);
  LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_10, LL_GPIO_MODE_ANALOG);

  /* Configure the default Alternate Function in current IO */
  LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_11, LL_GPIO_AF_0);
  LL_GPIO_SetAFPin_8_15(GPIOC, LL_GPIO_PIN_10, LL_GPIO_AF_0);

  /* Configure the default value for IO Speed */
  LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_11, LL_GPIO_SPEED_FREQ_LOW);
  LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_LOW);

  /* Configure the default value IO Output Type */
  LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_11 | LL_GPIO_PIN_10, LL_GPIO_OUTPUT_PUSHPULL);

  /* Deactivate the Pull-up and Pull-down resistor for the current IO */
  LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_11, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_10, LL_GPIO_PULL_NO);

  /* Reset the IO output state */
  LL_GPIO_WriteOutputPin(GPIOC, LL_GPIO_PIN_11 | LL_GPIO_PIN_10, LL_GPIO_PIN_RESET);
}

/******************************************************************************/
/*          UART4 global interrupt is managed directly in user code.          */
/******************************************************************************/
/* void UART4_IRQHandler(void)
{
}
  */
