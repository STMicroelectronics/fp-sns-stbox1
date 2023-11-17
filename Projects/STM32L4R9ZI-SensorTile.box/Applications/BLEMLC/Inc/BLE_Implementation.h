/**
  ******************************************************************************
  * @file    BLEMLC\Inc\BLE_Implementation.h
  * @author  System Research & Applications Team - Catania Lab.
  * @version V1.6.0
  * @date    20-October-2023
  * @brief   BLE Implementation header template file.
  *          This file should be copied to the application folder and renamed
  *          to BLE_Implementation.h.
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
#ifndef _BLE_IMPLEMENTATION_H_
#define _BLE_IMPLEMENTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * User can added here the header file for the selected BLE features.
  * For example:
  * #include "BLE_Environmental.h"
  * #include "BLE_Inertial.h"
  */


#include "BLE_Inertial.h"
#include "BLE_FiniteStateMachine.h"
#include "BLE_MachineLearningCore.h"
#include "BLE_ActivityRecognition.h"
#include "BLE_Battery.h"

/* Exported Defines --------------------------------------------------------*/

/* STM32 Unique ID */
#define BLE_STM32_UUID STM32_UUID

/* STM32  Microcontrolles type */
#define BLE_STM32_MICRO "L4R9ZI"

/* STM32 board type*/
#define BLE_STM32_BOARD "STM32L4R9ZI-SensorTile.box"

/* Package Version firmware */
#define BLE_VERSION_FW_MAJOR    STBOX1_VERSION_MAJOR
#define BLE_VERSION_FW_MINOR    STBOX1_VERSION_MINOR
#define BLE_VERSION_FW_PATCH    STBOX1_VERSION_PATCH

/* Firmware Package Name */
#define BLE_FW_PACKAGENAME      STBOX1_PACKAGENAME

/* Exported Variables ------------------------------------------------------- */
extern volatile uint8_t  connected;

extern uint8_t AR_Enabled;
extern uint8_t MLC_Enabled;
extern uint8_t FSM_Enabled;
extern uint8_t BatteryTimerEnabled;

/* This array Maps the output of .ucf filt to the activities knowed by ST BLE Sensor application:
 *  - 0 : Stationary
 *  - 1 : Walking
 *  - 4 : Jogging
 *  - 8 : Biking
 *  - 12: Driving
 */
extern BLE_AR_output_t BLE_MappingToHAR_ouput_t[];

/* Exported functions ------------------------------------------------------- */
extern void BLE_InitCustomService(void);
extern void BLE_SetCustomAdvertiseData(uint8_t *manuf_data);
extern void BluetoothInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _BLE_IMPLEMENTATION_H_ */
