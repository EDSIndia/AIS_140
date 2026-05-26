#ifndef __STM32_CONFIG_H
#define __STM32_CONFIG_H
#include "main.h"

#define TAMPER_CHANNEL HAL_ADC_CHANNEL_1

/** Primary aliases for ADC1_IN2 pin */
#define EBV_SENSE_PORT                        HAL_GPIOC
#define EBV_SENSE_PIN                         HAL_GPIO_PIN_4

/** Primary aliases for ADC1_IN0 pin */
#define EV_SENSE_PORT                         HAL_GPIOA
#define EV_SENSE_PIN                          HAL_GPIO_PIN_0

/** Primary aliases for ADC1_IN4 pin */
#define TAMPER_PORT                           HAL_GPIOA
#define TAMPER_PIN                            HAL_GPIO_PIN_4

/** Primary aliases for ADC1_IN5 pin */
#define SOS_PORT                              HAL_GPIOA
#define SOS_PIN                               HAL_GPIO_PIN_5

/** Primary aliases for ADC1_IN11 pin */
#define IGN_SENSE_PORT                        HAL_GPIOC
#define IGN_SENSE_PIN                         HAL_GPIO_PIN_3


hal_adc_handle_t *mx_adc1_init(void);

void mx_adc1_deinit(void);

hal_adc_handle_t *mx_adc1_gethandle(void);


#endif