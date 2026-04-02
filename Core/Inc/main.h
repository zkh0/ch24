/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_iwdg.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#define 	u8   	unsigned char
#define 	u16   unsigned short
#define 	u32   unsigned int
#define 	u64   unsigned long long


#include "stdio.h"
#include "fifo.h"
#include "app.h"
#include "usart.h"
#include "comm.h"

/**************************************************************************************************************/

#define TRACE_ENABLE

#ifdef TRACE_ENABLE
#define Trace_Print  printf    /**< Select the method to print out debug message */
#else
#define Trace_Print(format,...)  
#endif
/**************************************************************************************************************/

/**************************************************************************************************************/

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define AR1_Pin LL_GPIO_PIN_2
#define AR1_GPIO_Port GPIOE
#define AR2_Pin LL_GPIO_PIN_3
#define AR2_GPIO_Port GPIOE
#define AR3_Pin LL_GPIO_PIN_4
#define AR3_GPIO_Port GPIOE
#define AR4_Pin LL_GPIO_PIN_5
#define AR4_GPIO_Port GPIOE
#define LED_Pin LL_GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define START_Pin LL_GPIO_PIN_12
#define START_GPIO_Port GPIOF
#define STOP_Pin LL_GPIO_PIN_13
#define STOP_GPIO_Port GPIOF
#define EN1_Pin LL_GPIO_PIN_6
#define EN1_GPIO_Port GPIOC
#define EN2_Pin LL_GPIO_PIN_7
#define EN2_GPIO_Port GPIOC
#define EN3_Pin LL_GPIO_PIN_0
#define EN3_GPIO_Port GPIOG
#define EN4_Pin LL_GPIO_PIN_1
#define EN4_GPIO_Port GPIOG
#define EN5_Pin LL_GPIO_PIN_2
#define EN5_GPIO_Port GPIOG
#define EN6_Pin LL_GPIO_PIN_3
#define EN6_GPIO_Port GPIOG
#define EN7_Pin LL_GPIO_PIN_4
#define EN7_GPIO_Port GPIOG
#define EN8_Pin LL_GPIO_PIN_8
#define EN8_GPIO_Port GPIOC
#define EN9_Pin LL_GPIO_PIN_9
#define EN9_GPIO_Port GPIOC
#define EN10_Pin LL_GPIO_PIN_8
#define EN10_GPIO_Port GPIOA
#define EN11_Pin LL_GPIO_PIN_9
#define EN11_GPIO_Port GPIOA
#define EN12_Pin LL_GPIO_PIN_10
#define EN12_GPIO_Port GPIOA
#define EN13_Pin LL_GPIO_PIN_6
#define EN13_GPIO_Port GPIOG
#define EN14_Pin LL_GPIO_PIN_7
#define EN14_GPIO_Port GPIOG
#define EN15_Pin LL_GPIO_PIN_8
#define EN15_GPIO_Port GPIOG
#define EN16_Pin LL_GPIO_PIN_9
#define EN16_GPIO_Port GPIOG
#define EN17_Pin LL_GPIO_PIN_0
#define EN17_GPIO_Port GPIOD
#define EN18_Pin LL_GPIO_PIN_1
#define EN18_GPIO_Port GPIOD
#define EN19_Pin LL_GPIO_PIN_4
#define EN19_GPIO_Port GPIOD
#define EN20_Pin LL_GPIO_PIN_5
#define EN20_GPIO_Port GPIOD
#define EN21_Pin LL_GPIO_PIN_6
#define EN21_GPIO_Port GPIOD
#define EN22_Pin LL_GPIO_PIN_7
#define EN22_GPIO_Port GPIOD
#define EN23_Pin LL_GPIO_PIN_3
#define EN23_GPIO_Port GPIOB
#define EN24_Pin LL_GPIO_PIN_4
#define EN24_GPIO_Port GPIOB
#define D_CLR_Pin LL_GPIO_PIN_6
#define D_CLR_GPIO_Port GPIOB
#define EN_1_2V_Pin LL_GPIO_PIN_7
#define EN_1_2V_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
