/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED2_Pin GPIO_PIN_5
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_6
#define LED3_GPIO_Port GPIOE
#define FLASH_MISO_Pin GPIO_PIN_2
#define FLASH_MISO_GPIO_Port GPIOC
#define LTE1_POWER_EN_Pin GPIO_PIN_12
#define LTE1_POWER_EN_GPIO_Port GPIOF
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOH
#define BEEP_Pin GPIO_PIN_10
#define BEEP_GPIO_Port GPIOH
#define LTE1_EN_Pin GPIO_PIN_12
#define LTE1_EN_GPIO_Port GPIOB
#define LTE2_EN_Pin GPIO_PIN_3
#define LTE2_EN_GPIO_Port GPIOB
#define LTE2_POWER_EN_Pin GPIO_PIN_11
#define LTE2_POWER_EN_GPIO_Port GPIOD
#define FLASH_CS_Pin GPIO_PIN_0
#define FLASH_CS_GPIO_Port GPIOI
#define FLASH_CLK_Pin GPIO_PIN_1
#define FLASH_CLK_GPIO_Port GPIOI
#define FLASH_MOSI_Pin GPIO_PIN_3
#define FLASH_MOSI_GPIO_Port GPIOI
#define LTE1_MCU_TXD_Pin GPIO_PIN_5
#define LTE1_MCU_TXD_GPIO_Port GPIOD
#define LTE1_MCU_RXD_Pin GPIO_PIN_6
#define LTE1_MCU_RXD_GPIO_Port GPIOD
#define MCU_DBG_TX_Pin GPIO_PIN_6
#define MCU_DBG_TX_GPIO_Port GPIOB
#define MCU_DBG_RX_Pin GPIO_PIN_7
#define MCU_DBG_RX_GPIO_Port GPIOB
#define LTE1_DTR_Pin GPIO_PIN_4
#define LTE1_DTR_GPIO_Port GPIOI
#define LTE2_DTR_Pin GPIO_PIN_15
#define LTE2_DTR_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
