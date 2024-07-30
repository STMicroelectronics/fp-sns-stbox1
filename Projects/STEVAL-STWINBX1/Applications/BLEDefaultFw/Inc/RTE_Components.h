/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file
  * @author  MCD Application Team
  * @version V2.0.0
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
#ifndef  __RTE_COMPONENTS_H__
#define  __RTE_COMPONENTS_H__

/* Defines ------------------------------------------------------------------*/
/* STMicroelectronics.FP-SNS-STBOX1.2.0.0 */
#define STWINBOX_DEFAULT_FW 1
#define USE_STWINBX1 1
/* STMicroelectronics.X-CUBE-BLE2.3.3.0 */
#define HCI_TL
#define HCI_TL_INTERFACE
/* STMicroelectronics.X-CUBE-BLEMGR.3.1.0 */
#define USE_PARSON
#define BLE_MANAGER_BATTERY
#define BLE_MANAGER_ENVIRONMENTAL
#define BLE_MANAGER_INERTIAL
#define BLUENRG_2
/* STMicroelectronics.X-CUBE-MEMS1.10.0.0 */
#define ILPS22QS_PRESSTEMPQVAR_I2C
#define STTS22H_TEMP_I2C
#define ISM330DHCX_ACCGYR_SPI
#define IIS2DLPC_ACC_SPI
#define IIS2ICLX_ACC_SPI
#define IIS3DWB_ACC_SPI
#define IIS2MDC_MAG_I2C
/* STMicroelectronics.X-CUBE-NFC4.3.0.0 */
#define ST25DV_NFC4_I2C
#define LIB_NDEF

#endif /* __RTE_COMPONENTS_H__ */
