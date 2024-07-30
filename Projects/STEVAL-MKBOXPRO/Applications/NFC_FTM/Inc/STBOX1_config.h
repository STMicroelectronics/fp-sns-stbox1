/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    NFC_FTM\Inc\STBOX1_config.h
  * @author  System Research & Applications Team - Agrate/Catania Lab.
  * @brief   FP-SNS-STBOX1 configuration
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __STBOX1_CONFIG_H
#define __STBOX1_CONFIG_H

/* Exported define ------------------------------------------------------------*/

/* For enabling the printf */
#define STBOX1_ENABLE_PRINTF

/* For Enabling Bip Sound at Start up */
#define STBOX1_ENABLE_START_BIP

#include "stm32u5xx_hal.h"
/* Exported defines for TIMER -------------------------------------------------*/
extern TIM_HandleTypeDef htim1;
#define TIM_CC_HANDLE    htim1
#define TIM_CC_INSTANCE  TIM1
#define NFC_FTM_TIMx_CLK_ENABLE                  __HAL_RCC_TIM1_CLK_ENABLE
#define NFC_FTM_TIMx_FORCE_RESET                 __HAL_RCC_TIM1_FORCE_RESET
#define NFC_FTM_TIMx_RELEASE_RESET               __HAL_RCC_TIM1_RELEASE_RESET
#define NFC_FTM_GPIO_AF1_TIMx                    GPIO_AF1_TIM1

/**************************************
 *  Lab/Experimental section defines  *
***************************************/

/**************************************
 * Don't Change the following defines *
***************************************/

/* Package Version only numbers 0->9 */
#define STBOX1_VERSION_MAJOR 2
#define STBOX1_VERSION_MINOR 0
#define STBOX1_VERSION_PATCH 0

/* st25dv tag sizes */
#define NFCTAG_4K_SIZE           ((uint32_t) 0x200)

/* Eval-SmarTag2 includes the st25dv64k */
#define ST25DV_MAX_SIZE           NFCTAG_4K_SIZE
/* Dimension of the CC file in bytes */
#define ST25DV_CC_SIZE            8

#define STBOX1_MSB_PASSWORD 0x90ABCDEF
#define STBOX1_LSB_PASSWORD 0x12345678

/* Package Name */
#define STBOX1_PACKAGENAME "NFC_FTM"

#ifdef STBOX1_ENABLE_PRINTF
#define STBOX1_PRINTF(...) printf(__VA_ARGS__)
#else /* STBOX1_ENABLE_PRINTF */
#define STBOX1_PRINTF(...)
#endif /* STBOX1_ENABLE_PRINTF */

#endif /* __STBOX1_CONFIG_H */

