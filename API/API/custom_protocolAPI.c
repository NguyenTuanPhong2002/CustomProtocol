/*
 * File:    custom_protocol.h
 * Author:  Nguyen Tuan Phong
 * Date:    2024-08-21
 */

#include "custom_protocolAPI.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

Protocol_StatusTypeDef UART_Init(void)
{
    // huart1.Instance = USART1;
    // huart1.Init.BaudRate = 115200;
    // huart1.Init.WordLength = UART_WORDLENGTH_8B;
    // huart1.Init.StopBits = UART_STOPBITS_1;
    // huart1.Init.Parity = UART_PARITY_NONE;
    // huart1.Init.Mode = UART_MODE_TX_RX;
    // iAf (HAL_UART_Init(&huart1) != HAL_OK)
    // {
    //     return PROTOCOL_ERROR;
    // }
    return PROTOCOL_OK;
}

Protocol_StatusTypeDef UART_Transmit(uint8_t *pData, uint16_t size)
{
    if (HAL_UART_Transmit(&huart1, pData, size, 1000) != HAL_OK)
    {
        return PROTOCOL_ERROR;
    }
    return PROTOCOL_OK;
}

Protocol_StatusTypeDef UART_Receive(uint8_t *pData, uint16_t size)
{
    if (pData == NULL)
    {
        return PROTOCOL_ERROR;
    }

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, pData, CUSTOM_PROTOCOL_SIZE);
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

    return PROTOCOL_OK;
}

uint32_t UART_Gettick(void)
{
    return HAL_GetTick();
}

Protocol_HandleTypeDef Protocol_Interface = {
    .Init = UART_Init,
    .Transmit = UART_Transmit,
    .Receive = UART_Receive,
    .get_tick_ms = UART_Gettick,
};
