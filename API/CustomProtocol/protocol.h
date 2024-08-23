/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-21
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define CUSTOM_PROTOCOL_TIMEOUT (1)

#define CUSTOM_PROTOCOL_CHECK_EVENT (0)

#define CUSTOM_PROTOCOL_COMMAND_READ 0x01
#define CUSTOM_PROTOCOL_COMMAND_WRITE 0x00

#define CUSTOM_PROTOCOL_START_BIT 0xFF

#define CUSTOM_PROTOCOL_SIZE (3000U) // 65535

typedef enum
{
    PROTOCOL_OK,
    PROTOCOL_ERROR,
    PROTOCOL_BUSY,
    PROTOCOL_TIMEOUT
} Protocol_StatusTypeDef;

typedef enum
{
    CUSTOM_PROTOCOL_INTERRUPT_ENABLE,
    CUSTOM_PROTOCOL_INTERRUPT_DISABLE
} CustomProtocolInterrupt;

typedef struct
{
    uint8_t start_bit;
    uint8_t command;
    uint16_t length;
    uint8_t data[CUSTOM_PROTOCOL_SIZE - 12U];
    uint16_t checksum;
    uint8_t end_bit;
} Protocol_MsgTypeDef;

typedef struct
{
    Protocol_StatusTypeDef (*Init)(void);
    Protocol_StatusTypeDef (*Transmit)(uint8_t *pData, uint16_t size);
    Protocol_StatusTypeDef (*Receive)(uint8_t *pData, uint16_t size);
    Protocol_StatusTypeDef (*Interupt)(void);
#ifdef CUSTOM_PROTOCOL_TIMEOUT
    uint32_t (*get_tick_ms)(void);
#endif

    Protocol_MsgTypeDef *msg;
    CustomProtocolInterrupt flag;
    uint16_t size;
    uint16_t timeout;

    char buffer[CUSTOM_PROTOCOL_SIZE];

} Protocol_HandleTypeDef;

Protocol_StatusTypeDef CustomProtocol_Init(Protocol_HandleTypeDef *protocol);
Protocol_StatusTypeDef CustomProtocol_TransmitData(
    Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size);
Protocol_StatusTypeDef CustomProtocol_ReciverData(
    Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size);

Protocol_StatusTypeDef CustomProtocol_SetFlag(Protocol_HandleTypeDef *protocol, const CustomProtocolInterrupt status, uint16_t size);

uint16_t calculate_checksum(uint8_t *data, uint16_t length);

#endif