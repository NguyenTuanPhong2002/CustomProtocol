/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-21
 */

#ifndef CUSTOMPROTOCOLAPI_H
#define CUSTOMPROTOCOLAPI_H

#include "protocol.h"

#include "stm32f1xx_hal.h"

Protocol_StatusTypeDef UART_init(void);
Protocol_StatusTypeDef UART_Transmit(uint8_t *pData, uint16_t size);
Protocol_StatusTypeDef UART_Receive(uint8_t *pData, uint16_t size);
uint32_t UART_Gettick(void);

#endif
