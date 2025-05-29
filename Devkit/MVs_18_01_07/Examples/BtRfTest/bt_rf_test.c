////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2011, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:bt_rf_test.c
//
//		Description	:
//					BT Flash saving code
//
//		Changelog	:
//					yzhou@2013.12.06 initial version
///////////////////////////////////////////////////////////////////////////////
#include "string.h"
#include "type.h"
#include "uart.h"
#include "delay.h"

const uint8_t ENABLE_ALLSCAN[] = {0x01, 0x1a, 0x0c, 0x01, 0x03};
const uint8_t AUTOACCEPT_CONNECT[] = {0x01, 0x05, 0x0c, 0x03, 0x02, 0x00, 0x02};
const uint8_t ENABLE_DUT[] = {0x01, 0x03, 0x18, 0x00};

#define MAX_CMD_BUFFER_LEN		40
extern int parse_command(uint8_t * cmd, uint8_t cmd_len);

void BtRfTest(void)
{
	uint8_t		recv_byte;
	uint8_t		recv_buf[MAX_CMD_BUFFER_LEN];
	uint8_t		recv_buf_len = 0;

	//goto RF test mode
	BuartSend((uint8_t*)ENABLE_ALLSCAN, sizeof(ENABLE_ALLSCAN));
	WaitMs(30);
	BuartSend((uint8_t*)AUTOACCEPT_CONNECT, sizeof(AUTOACCEPT_CONNECT));
	WaitMs(30);
	BuartSend((uint8_t*)ENABLE_DUT, sizeof(ENABLE_DUT));
	
	while(1)
	{
		if(-1 != FuartRecvByte(&recv_byte))
		{
			FuartSend(&recv_byte, 1); // echo
			recv_buf[recv_buf_len++] = recv_byte;

			if(recv_byte == '\r')
			{
				if(-1 != FuartRecvByte(&recv_byte))
				{
					FuartSend(&recv_byte, 1); // echo
					recv_buf[recv_buf_len++] = recv_byte;

					if(recv_byte == '\n')
					{
						parse_command(recv_buf, recv_buf_len - 2);
						memset(recv_buf, 0, MAX_CMD_BUFFER_LEN);
						recv_buf_len = 0;
					}
				}
			}
/*
			if(recv_byte != '\r')
			{
				if(recv_byte == '\b')
				{
					if(recv_buf_len > 0)
					{
						recv_buf_len--;
					}
				}
				else
				{
					recv_buf[recv_buf_len++] = recv_byte;
					if(recv_buf_len >= MAX_CMD_BUFFER_LEN)
						recv_buf_len = 0;
				}
			}
			else
			{
				FuartSend("\n", 1);
				parse_command(recv_buf, recv_buf_len);
				memset(recv_buf, 0, MAX_CMD_BUFFER_LEN);
				recv_buf_len = 0;
			}
*/
		}
	}
}


