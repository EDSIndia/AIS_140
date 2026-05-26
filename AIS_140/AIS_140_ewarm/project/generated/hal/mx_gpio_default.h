/**
  ******************************************************************************
  * @file           : mx_gpio_default.h
  * @brief          : Header for mx_gpio_default.c file.
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
#ifndef MX_GPIO_DEFAULT_H
#define MX_GPIO_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32_hal.h"
#include "mx_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/******************************************************************************/
/* Exported defines for gpio_default in HAL layer (SW instance MyGPIO_1) */
/******************************************************************************/

/* Primary aliases for GPIO PA8 pin */
#define CAN_STB_PORT                                    HAL_GPIOA
#define CAN_STB_PIN                                     HAL_GPIO_PIN_8
#define CAN_STB_INIT_STATE                              HAL_GPIO_PIN_RESET
#define CAN_STB_ACTIVE_STATE                            HAL_GPIO_PIN_SET
#define CAN_STB_INACTIVE_STATE                          HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PA15(JTDI) pin */
#define SPI1_MEMS_PORT                                  HAL_GPIOA
#define SPI1_MEMS_PIN                                   HAL_GPIO_PIN_15
#define SPI1_MEMS_INIT_STATE                            HAL_GPIO_PIN_RESET
#define SPI1_MEMS_ACTIVE_STATE                          HAL_GPIO_PIN_SET
#define SPI1_MEMS_INACTIVE_STATE                        HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB6 pin */
#define GPS_RESET_PORT                                  HAL_GPIOB
#define GPS_RESET_PIN                                   HAL_GPIO_PIN_6
#define GPS_RESET_INIT_STATE                            HAL_GPIO_PIN_RESET
#define GPS_RESET_ACTIVE_STATE                          HAL_GPIO_PIN_SET
#define GPS_RESET_INACTIVE_STATE                        HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB7 pin */
#define REG_ON_OFF_PORT                                 HAL_GPIOB
#define REG_ON_OFF_PIN                                  HAL_GPIO_PIN_7
#define REG_ON_OFF_INIT_STATE                           HAL_GPIO_PIN_RESET
#define REG_ON_OFF_ACTIVE_STATE                         HAL_GPIO_PIN_SET
#define REG_ON_OFF_INACTIVE_STATE                       HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB8 pin */
#define BT_LED_PORT                                     HAL_GPIOB
#define BT_LED_PIN                                      HAL_GPIO_PIN_8
#define BT_LED_INIT_STATE                               HAL_GPIO_PIN_RESET
#define BT_LED_ACTIVE_STATE                             HAL_GPIO_PIN_SET
#define BT_LED_INACTIVE_STATE                           HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB9 pin */
#define RLED_PORT                                       HAL_GPIOB
#define RLED_PIN                                        HAL_GPIO_PIN_9
#define RLED_INIT_STATE                                 HAL_GPIO_PIN_RESET
#define RLED_ACTIVE_STATE                               HAL_GPIO_PIN_SET
#define RLED_INACTIVE_STATE                             HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PB12 pin */
#define SPI2_CS_PORT                                    HAL_GPIOB
#define SPI2_CS_PIN                                     HAL_GPIO_PIN_12
#define SPI2_CS_INIT_STATE                              HAL_GPIO_PIN_RESET
#define SPI2_CS_ACTIVE_STATE                            HAL_GPIO_PIN_SET
#define SPI2_CS_INACTIVE_STATE                          HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC2 pin */
#define GPS_LED_PORT                                    HAL_GPIOC
#define GPS_LED_PIN                                     HAL_GPIO_PIN_2
#define GPS_LED_INIT_STATE                              HAL_GPIO_PIN_RESET
#define GPS_LED_ACTIVE_STATE                            HAL_GPIO_PIN_SET
#define GPS_LED_INACTIVE_STATE                          HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC6 pin */
#define SPI_INT2_PORT                                   HAL_GPIOC
#define SPI_INT2_PIN                                    HAL_GPIO_PIN_6
#define SPI_INT2_INIT_STATE                             HAL_GPIO_PIN_RESET
#define SPI_INT2_ACTIVE_STATE                           HAL_GPIO_PIN_SET
#define SPI_INT2_INACTIVE_STATE                         HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC7 pin */
#define SPI_INT1_PORT                                   HAL_GPIOC
#define SPI_INT1_PIN                                    HAL_GPIO_PIN_7
#define SPI_INT1_INIT_STATE                             HAL_GPIO_PIN_RESET
#define SPI_INT1_ACTIVE_STATE                           HAL_GPIO_PIN_SET
#define SPI_INT1_INACTIVE_STATE                         HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC8 pin */
#define D2_OUT_PORT                                     HAL_GPIOC
#define D2_OUT_PIN                                      HAL_GPIO_PIN_8
#define D2_OUT_INIT_STATE                               HAL_GPIO_PIN_RESET
#define D2_OUT_ACTIVE_STATE                             HAL_GPIO_PIN_SET
#define D2_OUT_INACTIVE_STATE                           HAL_GPIO_PIN_RESET

/* Primary aliases for GPIO PC9 pin */
#define D1_OUT_PORT                                     HAL_GPIOC
#define D1_OUT_PIN                                      HAL_GPIO_PIN_9
#define D1_OUT_INIT_STATE                               HAL_GPIO_PIN_RESET
#define D1_OUT_ACTIVE_STATE                             HAL_GPIO_PIN_SET
#define D1_OUT_INACTIVE_STATE                           HAL_GPIO_PIN_RESET

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/******************************************************************************/
/* Exported functions for gpio_default in HAL layer (SW instance MyGPIO_1) */
/******************************************************************************/
/**
  * @brief mx_gpio_default init function
  * This function configures the hardware resources used in this example
  * @retval 0  GPIO group correctly initialized
  * @retval -1 Issue detected during GPIO group initialization
  */
system_status_t mx_gpio_default_init(void);

/**
  * @brief  De-initialize gpio_default instance.
  */
system_status_t mx_gpio_default_deinit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_GPIO_DEFAULT_H */
