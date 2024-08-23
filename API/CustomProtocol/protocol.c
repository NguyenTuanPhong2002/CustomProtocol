/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-21
 */

#include "protocol.h"
#include <string.h>

#include "stm32f1xx_hal.h"
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

Protocol_MsgTypeDef msg = {0};

Protocol_StatusTypeDef
CustomProtocol_Init(Protocol_HandleTypeDef *protocol)
{
    if (protocol == NULL)
    {
        return PROTOCOL_ERROR;
    }

    protocol->flag = CUSTOM_PROTOCOL_INTERRUPT_DISABLE;
    protocol->size = 0;
    protocol->msg = &msg;
#ifdef CUSTOM_PROTOCOL_TIMEOUT
    protocol->timeout = 2000;
#endif

    return protocol->Init();
}

Protocol_StatusTypeDef CustomProtocol_TransmitData(Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size)
{
    if (protocol == NULL)
    {
        return PROTOCOL_ERROR;
    }

    return protocol->Transmit(pData, size);
}

Protocol_StatusTypeDef CustomProtocol_ReciverData(Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size)
{
    if (protocol == NULL)
    {
        return PROTOCOL_ERROR;
    }

    if (size > CUSTOM_PROTOCOL_SIZE)
    {
        return PROTOCOL_ERROR;
    }

    Protocol_StatusTypeDef status = PROTOCOL_BUSY;

    protocol->Receive((uint8_t *)protocol->buffer, size);

#ifdef CUSTOM_PROTOCOL_TIMEOUT

    uint32_t tickStart = (uint32_t)protocol->get_tick_ms();

    while (status == PROTOCOL_BUSY)
    {
        const uint32_t timeElapsed = HAL_GetTick() - tickStart;

        if (timeElapsed >= protocol->timeout)
        {
            status = PROTOCOL_TIMEOUT;
            break;
        }
        else
        {
#endif
            if (protocol->flag == CUSTOM_PROTOCOL_INTERRUPT_ENABLE)
            {
                protocol->msg->command = (uint8_t)protocol->buffer[1];
                if (protocol->msg->command == CUSTOM_PROTOCOL_COMMAND_READ)
                {
                    protocol->msg->length = (protocol->buffer[2] << 8) | protocol->buffer[3];

                    memcpy(protocol->msg->data, &protocol->buffer[4], protocol->msg->length);

                    protocol->msg->checksum = (protocol->buffer[4 + protocol->msg->length] << 8) | protocol->buffer[5 + protocol->msg->length];

                    if (protocol->msg->checksum == calculate_checksum(protocol->msg->data, protocol->msg->length))
                    {
                        status = PROTOCOL_OK;
                        break;
                    }
                    else
                    {
                        status = PROTOCOL_ERROR;
                        break;
                    }
                }
                else
                {
                    status = PROTOCOL_ERROR;
                    break;
                }
#ifdef CUSTOM_PROTOCOL_TIMEOUT
            }
        }
    }
#endif

    protocol->flag = CUSTOM_PROTOCOL_INTERRUPT_DISABLE;
    return status;
}

Protocol_StatusTypeDef CustomProtocol_SetFlag(Protocol_HandleTypeDef *protocol, const CustomProtocolInterrupt status, uint16_t size)
{
    if (protocol == NULL)
    {
        return PROTOCOL_ERROR;
    }

    protocol->flag = status;
    protocol->size = size;

#ifdef CUSTOM_PROTOCOL_CHECK_EVENT
    CustomProtocol_ReciverData(protocol, protocol->buffer, size);
#endif

    return PROTOCOL_OK;
}

uint16_t calculate_checksum(uint8_t *data, uint16_t length)
{
    uint16_t checksum = 0;
    for (uint16_t i = 0; i < length; i++)
    {
        checksum += data[i];
    }
    return checksum;
}
