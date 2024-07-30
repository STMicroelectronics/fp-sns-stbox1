/**
  ******************************************************************************
  * @file    custom_mems_conf.h
  * @author  MEMS Software Solutions Team
  * @brief   This file contains definitions of the MEMS components bus interfaces for custom boards
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CUSTOM_MEMS_CONF_H
#define CUSTOM_MEMS_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "steval_stwinbx1_bus.h"
#include "steval_stwinbx1_errno.h"

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#define USE_CUSTOM_MOTION_SENSOR_IIS2DLPC_0       0U

#define USE_CUSTOM_MOTION_SENSOR_IIS2MDC_0        0U

#define USE_CUSTOM_MOTION_SENSOR_ISM330DHCX_0     0U

#define USE_CUSTOM_ENV_SENSOR_STTS22H_0           0U

#define USE_CUSTOM_MOTION_SENSOR_IIS2ICLX_0       0U

#define USE_CUSTOM_ENV_SENSOR_ILPS22QS_0          0U

#define USE_CUSTOM_MOTION_SENSOR_IIS3DWB_0        0U

#define CUSTOM_IIS2DLPC_0_SPI_Init BSP_SPI2_Init
#define CUSTOM_IIS2DLPC_0_SPI_DeInit BSP_SPI2_DeInit
#define CUSTOM_IIS2DLPC_0_SPI_Send BSP_SPI2_Send
#define CUSTOM_IIS2DLPC_0_SPI_Recv BSP_SPI2_Recv

#define CUSTOM_IIS2DLPC_0_CS_PORT GPIOH
#define CUSTOM_IIS2DLPC_0_CS_PIN GPIO_PIN_6

#define CUSTOM_IIS2MDC_0_I2C_Init BSP_I2C2_Init
#define CUSTOM_IIS2MDC_0_I2C_DeInit BSP_I2C2_DeInit
#define CUSTOM_IIS2MDC_0_I2C_ReadReg BSP_I2C2_ReadReg
#define CUSTOM_IIS2MDC_0_I2C_WriteReg BSP_I2C2_WriteReg

#define CUSTOM_ISM330DHCX_0_SPI_Init BSP_SPI2_Init
#define CUSTOM_ISM330DHCX_0_SPI_DeInit BSP_SPI2_DeInit
#define CUSTOM_ISM330DHCX_0_SPI_Send BSP_SPI2_Send
#define CUSTOM_ISM330DHCX_0_SPI_Recv BSP_SPI2_Recv

#define CUSTOM_ISM330DHCX_0_CS_PORT GPIOH
#define CUSTOM_ISM330DHCX_0_CS_PIN GPIO_PIN_15

#define CUSTOM_STTS22H_0_I2C_Init BSP_I2C2_Init
#define CUSTOM_STTS22H_0_I2C_DeInit BSP_I2C2_DeInit
#define CUSTOM_STTS22H_0_I2C_ReadReg BSP_I2C2_ReadReg
#define CUSTOM_STTS22H_0_I2C_WriteReg BSP_I2C2_WriteReg

#define CUSTOM_IIS2ICLX_0_SPI_Init BSP_SPI2_Init
#define CUSTOM_IIS2ICLX_0_SPI_DeInit BSP_SPI2_DeInit
#define CUSTOM_IIS2ICLX_0_SPI_Send BSP_SPI2_Send
#define CUSTOM_IIS2ICLX_0_SPI_Recv BSP_SPI2_Recv

#define CUSTOM_IIS2ICLX_0_CS_PORT GPIOI
#define CUSTOM_IIS2ICLX_0_CS_PIN GPIO_PIN_7

#define CUSTOM_ILPS22QS_0_I2C_Init BSP_I2C2_Init
#define CUSTOM_ILPS22QS_0_I2C_DeInit BSP_I2C2_DeInit
#define CUSTOM_ILPS22QS_0_I2C_ReadReg BSP_I2C2_ReadReg
#define CUSTOM_ILPS22QS_0_I2C_WriteReg BSP_I2C2_WriteReg

#define CUSTOM_IIS3DWB_0_SPI_Init BSP_SPI2_Init
#define CUSTOM_IIS3DWB_0_SPI_DeInit BSP_SPI2_DeInit
#define CUSTOM_IIS3DWB_0_SPI_Send BSP_SPI2_Send
#define CUSTOM_IIS3DWB_0_SPI_Recv BSP_SPI2_Recv

#define CUSTOM_IIS3DWB_0_CS_PORT GPIOF
#define CUSTOM_IIS3DWB_0_CS_PIN GPIO_PIN_12

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_MEMS_CONF_H*/
