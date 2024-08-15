/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-12
 */
#include "custom_protocol.h"
#include <string.h>

Custom_Protocol_StateTypeDef Custom_Protocol_Init(
    Custom_Protocol_HandleTypeDef *custom_protocol,
    UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma)
{
    if (custom_protocol == NULL)
    {
        return CUSTOM_PROTOCOL_STATE_ERROR;
    }

    if (custom_protocol->msg == NULL)
    {
        custom_protocol->msg = malloc(sizeof(*custom_protocol->msg));
        if (custom_protocol->msg == NULL)
        {
            return CUSTOM_PROTOCOL_STATE_ERROR;
        }
    }

    custom_protocol->huart = huart;
    custom_protocol->hdma = hdma;
    custom_protocol->msg->start_bit = 0xFF;
    custom_protocol->msg->command = CUSTOM_PROTOCOL_COMMAND_READ;
    custom_protocol->msg->length = 0;
    memset(custom_protocol->msg->data, '\0', CUSTOM_PROTOCOL_SIZE - 12U);
    custom_protocol->msg->end_bit = 0x00;

    custom_protocol->flag = CUSTOM_PROTOCOL_INTERRUPT_DISABLE;

    return CUSTOM_PROTOCOL_STATE_OK;
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

Custom_Protocol_StateTypeDef Custom_Protocol_SendData(
    Custom_Protocol_HandleTypeDef *custom_protocol, uint8_t *data,
    uint16_t length)
{
    if (custom_protocol == NULL)
    {
        return CUSTOM_PROTOCOL_STATE_ERROR;
    }

    custom_protocol->msg->command = CUSTOM_PROTOCOL_COMMAND_WRITE;
    custom_protocol->msg->length = length;
    custom_protocol->msg->checksum = calculate_checksum(data, length);
    
    HAL_UART_Transmit(custom_protocol->huart,
                      custom_protocol->msg->start_bit, sizeof(custom_protocol->msg->start_bit), HAL_MAX_DELAY);

    HAL_UART_Transmit(custom_protocol->huart, CUSTOM_PROTOCOL_COMMAND_WRITE,
                      sizeof(CUSTOM_PROTOCOL_COMMAND_WRITE), HAL_MAX_DELAY);

    uint8_t data_length[2];
    data_length[0] = (uint8_t)(custom_protocol->msg->length >> 8);
    data_length[1] = (uint8_t)(custom_protocol->msg->length & 0xFF);
    HAL_UART_Transmit(custom_protocol->huart, data_length, sizeof(data_length), HAL_MAX_DELAY);

    HAL_UART_Transmit(custom_protocol->huart, data,
                      length, HAL_MAX_DELAY);

    data_length[0] = (uint8_t)(custom_protocol->msg->checksum >> 8);
    data_length[1] = (uint8_t)(custom_protocol->msg->checksum & 0xFF);
    HAL_UART_Transmit(custom_protocol->huart, data_length, sizeof(data_length), HAL_MAX_DELAY);

    HAL_UART_Transmit(custom_protocol->huart, custom_protocol->msg->end_bit,
                      sizeof(custom_protocol->msg->end_bit), HAL_MAX_DELAY);

    custom_protocol->flag = CUSTOM_PROTOCOL_INTERRUPT_DISABLE;

    return CUSTOM_PROTOCOL_STATE_OK;
}

Custom_Protocol_StateTypeDef Custom_Protocol_ReceiveData(
    Custom_Protocol_HandleTypeDef *custom_protocol, uint32_t timeout)
{
    if (custom_protocol == NULL)
    {
        return CUSTOM_PROTOCOL_STATE_ERROR;
    }

    Custom_Protocol_StateTypeDef status = CUSTOM_PROTOCOL_STATE_BUSY;

    HAL_UARTEx_ReceiveToIdle_DMA((UART_HandleTypeDef *)custom_protocol->huart,
                                 (uint8_t *)custom_protocol->buffer, CUSTOM_PROTOCOL_SIZE);
    __HAL_DMA_DISABLE_IT((DMA_HandleTypeDef *)custom_protocol->hdma, DMA_IT_HT);

    memset(custom_protocol->buffer, '\0', CUSTOM_PROTOCOL_SIZE);

    uint32_t tickStart = HAL_GetTick();

    while (status == CUSTOM_PROTOCOL_STATE_BUSY)
    {
        const uint32_t timeElapsed = HAL_GetTick() - tickStart;

        if (timeElapsed >= timeout)
        {
            status = CUSTOM_PROTOCOL_STATE_TIMEOUT;
            break;
        }
        else
        {
            if (custom_protocol->flag == CUSTOM_PROTOCOL_INTERRUPT_ENABLE)
            {
                custom_protocol->msg->command = custom_protocol->buffer[1];
                custom_protocol->msg->length = (custom_protocol->buffer[2] << 8) | custom_protocol->buffer[3];
                memcpy(custom_protocol->msg->data, &custom_protocol->buffer[4],
                       custom_protocol->msg->length);
                custom_protocol->msg->checksum = (custom_protocol->buffer[2 + custom_protocol->msg->length] << 8) | custom_protocol->buffer[3 + custom_protocol->msg->length];

                if (custom_protocol->msg->checksum == calculate_checksum(custom_protocol->msg->data,
                                                                         custom_protocol->msg->length))
                {
                    status = CUSTOM_PROTOCOL_STATE_OK;
                    break;
                }
                else
                {
                    status = CUSTOM_PROTOCOL_STATE_ERROR;
                    break;
                }
            }
        }
    }

    return status;
}
