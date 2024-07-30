/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   BLEMLC\Inc\app_blemlc.h
  * @author  System Research & Applications Team - Agrate/Catania Lab.
  * @brief  Header for app_blemlc.c file.
  *         This file contains the common defines of the application.
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
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_BLEMLC_H__
#define __APP_BLEMLC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "steval_stwinbx1.h"
#include "STWIN.box_conf.h"

/* Exported macro ------------------------------------------------------------*/
#define MCR_HEART_BIT()   \
{                         \
  BSP_LED_On(LED_GREEN);  \
  HAL_Delay(200);         \
  BSP_LED_Off(LED_GREEN); \
  HAL_Delay(400);         \
  BSP_LED_On(LED_GREEN);  \
  HAL_Delay(200);         \
  BSP_LED_Off(LED_GREEN); \
  HAL_Delay(1000);        \
}

#define MCR_HEART_BIT2()   \
{                          \
  BSP_LED_On(LED_ORANGE);  \
  HAL_Delay(200);          \
  BSP_LED_Off(LED_ORANGE); \
  HAL_Delay(400);          \
  BSP_LED_On(LED_ORANGE);  \
  HAL_Delay(200);          \
  BSP_LED_Off(LED_ORANGE); \
  HAL_Delay(1000);         \
}

#ifndef MEMS_UCF_SHARED_TYPES
#define MEMS_UCF_SHARED_TYPES
/** Common data block definition **/
typedef struct {
  uint8_t address;
  uint8_t data;
} ucf_line_t;

#endif /* MEMS_UCF_SHARED_TYPES */

/* Exported Variables --------------------------------------------------------*/

extern int32_t CurrentActiveBank;

extern ucf_line_t *MLCCustomUCFFile;
extern uint32_t MLCCustomUCFFileLength;
extern ucf_line_t *FSMCustomUCFFile;
extern uint32_t FSMCustomUCFFileLength;
extern char *MLCCustomLabels;
extern uint32_t MLCCustomLabelsLength;
extern char *FSMCustomLabels;
extern uint32_t FSMCustomLabelsLength;
extern void *MotionCompObj[];

/* Exported Functions Prototypes ---------------------------------------------*/
extern void STBOX1_Error_Handler(int32_t ErrorCode,char *File,int32_t Line);
extern void InitAcc(void);
extern void InitAcc_MLC(uint32_t UseCustomIfAvailable);
extern void InitAcc_FSM(uint32_t UseCustomIfAvailable);
extern void DeInit_Acc(void);
void MX_BLEMLC_Init(void);
void MX_BLEMLC_Process(void);
void BSP_Enable_LDO(void);
void BSP_Disable_LDO(void);

/* Exported defines -----------------------------------------------------------*/
#define STBOX1_ERROR_INIT_BLE 1
#define STBOX1_ERROR_FLASH 2
#define STBOX1_ERROR_SENSOR 3
#define STBOX1_ERROR_HW_INIT 4
#define STBOX1_ERROR_BLE 5
#define STBOX1_ERROR_TIMER 6

/* STM32 Unique ID */
#define STM32_UUID ((uint32_t *)0x0BFA0700)

/* STM32 MCU_ID */
#define STM32_MCU_ID ((uint32_t *)0xE0044000)

/* Context of LSM6DSV16X */
#define ISM330DHCX_Contex (&(((ISM330DHCX_Object_t *)MotionCompObj[ISM330DHCX_0])->Ctx))

#ifdef __cplusplus
}
#endif

#endif /*__APP_BLEMLC_H__ */

