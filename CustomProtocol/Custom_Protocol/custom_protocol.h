/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-12
 */
#ifndef CUSTOMPROTOCOL_H_
#define CUSTOMPROTOCOL_H_

#include "stm32f1xx_hal.h"

#define CUSTOM_PROTOCOL_COMMAND_READ 0x01
#define CUSTOM_PROTOCOL_COMMAND_WRITE 0x00

#define CUSTOM_PROTOCOL_START_BIT 0xFF

#define CUSTOM_PROTOCOL_SIZE 255U

typedef enum
{
    CUSTOM_PROTOCOL_STATE_RESET,
    CUSTOM_PROTOCOL_STATE_OK,
    CUSTOM_PROTOCOL_STATE_BUSY,
    CUSTOM_PROTOCOL_STATE_TIMEOUT,
    CUSTOM_PROTOCOL_STATE_ERROR
} Custom_Protocol_StateTypeDef;

typedef struct
{
    uint8_t start_bit;
    uint8_t command;
    uint16_t length;
    uint8_t data[CUSTOM_PROTOCOL_SIZE - 12U];
    uint16_t checksum;
    uint8_t end_bit;
} CustomProtocolMessage;

typedef enum
{
    CUSTOM_PROTOCOL_INTERRUPT_ENABLE,
    CUSTOM_PROTOCOL_INTERRUPT_DISABLE
} CustomProtocolInterrupt;

typedef struct
{
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef *hdma;
    CustomProtocolMessage *msg;
    CustomProtocolInterrupt flag;
    uint8_t buffer[CUSTOM_PROTOCOL_SIZE];
} Custom_Protocol_HandleTypeDef;

uint16_t custom_protocol_checksum(uint8_t *data, uint16_t length);

Custom_Protocol_StateTypeDef Custom_Protocol_Init(Custom_Protocol_HandleTypeDef *custom_protocol, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
Custom_Protocol_StateTypeDef Custom_Protocol_SendData(Custom_Protocol_HandleTypeDef *custom_protocol, uint8_t *data, uint16_t length);
Custom_Protocol_StateTypeDef Custom_Protocol_ReceiveData(Custom_Protocol_HandleTypeDef *custom_protocol, uint32_t timeout);

#endif
