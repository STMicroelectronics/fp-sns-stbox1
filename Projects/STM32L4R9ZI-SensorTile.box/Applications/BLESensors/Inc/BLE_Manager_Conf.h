/**
  ******************************************************************************
  * @file    BLESensors\Inc\BLE_Manager_Conf.h
  * @author  System Research & Applications Team - Catania Lab.
  * @version V1.6.0
  * @date    20-October-2023
  * @brief   BLE Manager configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to BLE_Manager_Conf.h.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLE_MANAGER_CONF_H__
#define __BLE_MANAGER_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Exported define ------------------------------------------------------------*/
#define BLUE_CORE BLUENRG_1_2

#define BLE_MANAGER_SDKV2

/* Define the Max dimesion of the Bluetooth characteristics for each packet  */
#define DEFAULT_MAX_CHAR_LEN 250

#define DEFAULT_MAX_CONFIG_CHAR_LEN     20
#define DEFAULT_MAX_STDOUT_CHAR_LEN     DEFAULT_MAX_CHAR_LEN
#define DEFAULT_MAX_STDERR_CHAR_LEN     20
#define DEFAULT_MAX_EXTCONFIG_CHAR_LEN  DEFAULT_MAX_CHAR_LEN

#define BLE_MANAGER_MAX_ALLOCABLE_CHARS 32U

/* For enabling the capability to handle BlueNRG Congestion */
#define ACC_BLUENRG_CONGESTION

/* Define the Delay function to use inside the BLE Manager (HAL_Delay/osDelay) */
#define BLE_MANAGER_DELAY HAL_Delay

/****************** Memory management functions **************************/
#define BLE_MALLOC_FUNCTION      malloc
#define BLE_FREE_FUNCTION        free
#define BLE_MEM_CPY              memcpy

/*---------- Print messages from BLE Manager files at middleware level -----------*/
/* Uncomment/Comment the following define for  disabling/enabling print messages from BLE Manager files */
#define BLE_MANAGER_DEBUG

#define BLE_DEBUG_LEVEL 1

#ifdef BLE_MANAGER_DEBUG
/**
  * User can change here printf with a custom implementation.
  * For example:
  * #include "STBOX1_config.h"
  * #include "main.h"
  * #define BLE_MANAGER_PRINTF  STBOX1_PRINTF
  */

#include "STBOX1_config.h"
#include "main.h"
#define BLE_MANAGER_PRINTF  STBOX1_PRINTF

/*  #include <stdio.h>
    #define BLE_MANAGER_PRINTF(...) printf(__VA_ARGS__) */
#else /* BLE_MANAGER_DEBUG */
#define BLE_MANAGER_PRINTF(...)
#endif /* BLE_MANAGER_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* __BLE_MANAGER_CONF_H__*/
