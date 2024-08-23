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

Protocol_StatusTypeDef CustomProtocol_Init(Protocol_HandleTypeDef *protocol)
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

Protocol_StatusTypeDef CustomProtocol_TransmitData(
	Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size)
{
	if (protocol == NULL)
	{
		return PROTOCOL_ERROR;
	}

	protocol->buffer[0] = CUSTOM_PROTOCOL_START_BIT;
	protocol->buffer[1] = CUSTOM_PROTOCOL_COMMAND_WRITE;
	protocol->buffer[2] = (size >> 8) & 0xFF;
	protocol->buffer[3] = size & 0xFF;
	for (int16_t i = 0; i < size; i++)
	{
		protocol->buffer[4 + i] = pData[i];
	}
	protocol->msg->checksum = calculate_checksum(pData,
												 size);

	protocol->buffer[4 + size] = (protocol->msg->checksum >> 8) & 0xFF;
	protocol->buffer[5 + size] = protocol->msg->checksum & 0xFF;
	protocol->buffer[6 + size] = CUSTOM_PROTOCOL_END_BIT;

	return protocol->Transmit(protocol->buffer, size + 7);
}

Protocol_StatusTypeDef CustomProtocol_ReciverData(
	Protocol_HandleTypeDef *protocol, uint8_t *pData, uint16_t size)
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

#if CUSTOM_PROTOCOL_TIMEOUT
	uint32_t tickStart = (uint32_t)protocol->get_tick_ms();

	while (status == PROTOCOL_BUSY)
	{
		const uint32_t timeElapsed = (uint32_t)protocol->get_tick_ms() - tickStart;

		if (timeElapsed >= protocol->timeout)
		{
			status = PROTOCOL_TIMEOUT;
			break;
		}
		else
		{
			if (protocol->flag == CUSTOM_PROTOCOL_INTERRUPT_ENABLE)
			{
				protocol->flag = CUSTOM_PROTOCOL_INTERRUPT_DISABLE;
				protocol->msg->start_bit = (uint8_t)protocol->buffer[0];
				protocol->msg->command = (uint8_t)protocol->buffer[1];
				protocol->msg->length = (protocol->buffer[2] << 8) | protocol->buffer[3];
				memcpy(protocol->msg->data, &protocol->buffer[4],
					   protocol->msg->length);
				protocol->msg->checksum = (protocol->buffer[4 + protocol->msg->length] << 8) | protocol->buffer[5 + protocol->msg->length];
				protocol->msg->end_bit = (uint8_t)protocol->buffer[6 + protocol->msg->length];

				if (protocol->msg->command == CUSTOM_PROTOCOL_COMMAND_READ && protocol->msg->start_bit == CUSTOM_PROTOCOL_START_BIT && protocol->msg->end_bit == CUSTOM_PROTOCOL_END_BIT)
				{

					if (protocol->msg->checksum == calculate_checksum(protocol->msg->data,
																	  protocol->msg->length))
					{
						pData = protocol->msg->data;
						status = PROTOCOL_OK;
						// HAL_UART_Transmit(&huart1, pData, protocol->msg->length, 1000);
						return status;
					}
					else
					{
						status = PROTOCOL_ERROR;
						return status;
					}
				}
				else
				{
					status = PROTOCOL_ERROR;
					return status;
				}
			}
		}
	}
#else
	if (protocol->flag == CUSTOM_PROTOCOL_INTERRUPT_ENABLE)
	{
		protocol->msg->start_bit = (uint8_t)protocol->buffer[0];
		protocol->msg->command = (uint8_t)protocol->buffer[1];
		protocol->msg->length = (protocol->buffer[2] << 8) | protocol->buffer[3];
		memcpy(protocol->msg->data, &protocol->buffer[4],
			   protocol->msg->length);
		protocol->msg->checksum = (protocol->buffer[4 + protocol->msg->length] << 8) | protocol->buffer[5 + protocol->msg->length];
		protocol->msg->end_bit = (uint8_t)protocol->buffer[6 + protocol->msg->length];

		if (protocol->msg->command == CUSTOM_PROTOCOL_COMMAND_READ && protocol->msg->start_bit == CUSTOM_PROTOCOL_START_BIT && protocol->msg->end_bit == CUSTOM_PROTOCOL_END_BIT)
		{

			if (protocol->msg->checksum == calculate_checksum(protocol->msg->data,
															  protocol->msg->length))
			{
				pData = protocol->msg->data;
				status = PROTOCOL_OK;
			}
			else
			{
				status = PROTOCOL_ERROR;
			}
		}
		else
		{
			status = PROTOCOL_ERROR;
		}
	}
#endif
	return status;
}

Protocol_StatusTypeDef CustomProtocol_SetFlag(Protocol_HandleTypeDef *protocol,
											  const CustomProtocolInterrupt status, uint16_t size)
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
