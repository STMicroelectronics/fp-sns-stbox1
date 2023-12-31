/**
  ******************************************************************************
  * @file    DataLogExtended\Inc\demo_serial.h
  * @brief   Header for demo-serial.c
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
#ifndef DEMO_SERIAL_H
#define DEMO_SERIAL_H

#include "cube_hal.h"
#include "serial_protocol.h"
#include "Serial_CMD.h"

#define SENDER_UART                     0x01
#define SENDER_USB                      0x02
#define SENDER_SPI                      0x03

#define DEV_ADDR                        50U
#define I2C_DATA_MAX_LENGTH_BYTES       16

#define STREAMING_MSG_LENGTH            81

extern volatile uint8_t DataLoggerActive;

int32_t HandleMSG(TMsg *Msg);
void BUILD_REPLY_HEADER(TMsg *Msg);
void BUILD_NACK_HEADER(TMsg *Msg);
void INIT_STREAMING_MSG(TMsg *Msg);
void INIT_STREAMING_HEADER(TMsg *Msg);

#endif /* DEMO_SERIAL_H */

