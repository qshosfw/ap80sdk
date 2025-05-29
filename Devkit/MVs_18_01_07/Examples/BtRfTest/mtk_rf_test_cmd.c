////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2011, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:
//					mtk_rf_test_cmd.c
//
//		Description	:
//					MTK RF test command
//
//		Changelog	:
//					Halley - init version 2015-08-07
///////////////////////////////////////////////////////////////////////////////
#include "string.h"
#include "type.h"
#include "uart.h"

uint8_t MTK_RF_TEST_COMMAND[] = {
	0x01, 0x0d, 0xfc, 0x17, 0x00, 0x00, 
	0x01, // tx pattern - default TX 0000 pattern
	0x00, // frequency single or hopping - default single frequency
	0x01, // TX frequency
	0x01, // RX frequency
	0x00, 0x01, 
	0x04, // Packet type
	0x22, 0x00, // Packet length
	0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
}; //  total 27 bytes


uint8_t * tx_pattern = &MTK_RF_TEST_COMMAND[6];			// pttrn
uint8_t * single_hopping = &MTK_RF_TEST_COMMAND[7];		// sngle
uint8_t * tx_freq = &MTK_RF_TEST_COMMAND[8];			// txfrq
uint8_t * rx_freq = &MTK_RF_TEST_COMMAND[9];			// rxfrq
uint8_t * packet_type = &MTK_RF_TEST_COMMAND[12];		// ptype
uint8_t * packet_length = &MTK_RF_TEST_COMMAND[13];


/*
* pattern values : 
*
* 01 : TX 0000 pattern
* 02 : TX 1111 pattern
* 03 : TX 1010 pattern
* 04 : TX Pesudo random bit sequence
* 09 : TX 11110000 pattern
* 16 : TX single tone
*/
void MTK_RF_TEST_CMD_set_tx_pattern(uint8_t pattern)
{
	*tx_pattern = pattern;
}

/*
* single frequency or hopping :
*
* 00 : Single Frequency
* 01 : 79 channels frequency hopping
*/
void MTK_RF_TEST_CMD_set_single_hopping(uint8_t hopping)
{
	*single_hopping = hopping;
}

/*
* Tx frequency :
* 
* Only can be set in single frequency mode, the value range is : 1 ~ 79
*/
void MTK_RF_TEST_CMD_set_tx_freq(uint8_t freq)
{
	*tx_freq = freq;
}

/*
* Rx frequency :
* 
* Only can be set in single frequency mode, the value range is : 1 ~ 79
*/
void MTK_RF_TEST_CMD_set_rx_freq(uint8_t freq)
{
	*rx_freq = freq;
}

/*
* Packet type: (Hex)
* 0x00 //NULL
* 0x01  //POLL
* 0x02 //FHS
* 0x03 //DM1
* 0x04 //DH1
* 0x05 //HV1
* 0x06 //HV2
* 0x07 //HV3
* 0x08 //DV
* 0x09 //AUX
* 0x0a //DM3
* 0x0b //DH3
* 0x0e //DM5
* 0x0f //DH5
* 0x17 //EV3
* 0x1c //EV4
* 0x1d //EV5
* 0x24 //2DH1
* 0x28 //3DH1
* 0x2a //2DH3
* 0x2b //3DH3
* 0x2e //2DH5
* 0x2f //3DH5
* 0x36 //2EV3
* 0x37 //3EV3
* 0x3c //2EV5
* 0x3d //3EV5
*/

void MTK_RF_TEST_CMD_set_packet_type(uint8_t type)
{
	*packet_type = type;
	
}

/*
* Packet length:
* 
* Packet length is associate with packet type
*
*/
void MTK_RF_TEST_CMD_set_packet_length(uint16_t length)
{
	*packet_length = length & 0xFF;
	*(packet_length+1) = (length >> 8) & 0xFF;
}


const int PacketTypeLength[27*2] = //{type, size}
{
	0x00 , 9, //NULL	idx: 0
	0x01 , 9, //POLL
	0x02 , 25,//FHS
	0x03 , 24,//DM1
	0x04 , 34,//DH1

	0x05 , 30,//HV1		idx:5
	0x06 , 30,//HV2
	0x07 , 30,//HV3
	0x08 , 30,//DV
	0x09 , 36,//AUX

	0x0a, 129,//DM3		idx:10
	0x0b, 191,//DH3
	0x0e, 232,//DM5
	0x0f, 347,//DH5
	0x17, 30,//EV3

	0x1c, 120,//EV4		idx:15
	0x1d, 180,//EV5
	0x24, 62,//2DH1
	0x28, 91,//3DH1
	0x2a, 375,//2DH3

	0x2b, 560,//3DH3	idx:20
	0x2e, 687,//2DH5
	0x2f, 1029,//3DH5
	0x36, 60,//2EV3
	0x37, 90,//3EV3

	0x3c, 360,//2EV5	idx:25
	0x3d, 540,//3EV5
};

/*
* Set the packet type.
*
* idx : index of the packet type. refer to the comment of PacketTypeLength
* 
* Example:
*	GFSK & DH1 , we should use the index  4;
*	GFSK & DH3 , we should use the index  11;
*	GFSK & DH5 , we should use the index  13;
*	4DQPSK & DH1 , we should use the index  17;
*	4DQPSK & DH3 , we should use the index  19;
*	4DQPSK & DH5 , we should use the index  21;
*	8DQPSK & DH1 , we should use the index  18;
*	8DQPSK & DH3 , we should use the index  20;
*	8DQPSK & DH5 , we should use the index  22;
*	
*/
void MTK_RF_TEST_CMD_set_packet_type_length(uint8_t idx)
{
	MTK_RF_TEST_CMD_set_packet_type(PacketTypeLength[idx*2]);
	MTK_RF_TEST_CMD_set_packet_length(PacketTypeLength[idx*2+1]);
}

void MTK_RF_TEST_CMD_go(void)
{
	BuartSend((uint8_t*)MTK_RF_TEST_COMMAND, sizeof(MTK_RF_TEST_COMMAND));
}

const uint8_t unknow_cmd_msg[] = {"\r\nunknow command\r\n"};
const uint8_t error_param_msg[] = {"error parameters\r\n"};
const uint8_t usage_msg[] =
{
	"\r\n[usage]\r\n"
	"\r\n"
	"commands:\r\n"
	"\t pttrn <value>: set tx partten. (value: refer to help file)\r\n"
	"\t sngle <value>: set single frequency or hopping frequency (value: refer to help file)\r\n"
	"\t txfrq <value>: set tx frequency (value: refer to help file)\r\n"
	"\t rxfrq <value>: set rx frequency (value: refer to help file)\r\n"
	"\t ptype <value>: set the packet type (value: refer to help file)\r\n"
	"\t cmdgo: make the parameters valid\r\n"
};

void print_usage(void)
{
	FuartSend((uint8_t  *)usage_msg, sizeof(usage_msg));
}

int8_t get_params(uint8_t * param, uint8_t param_len)
{
	uint8_t		i;
	uint8_t		result = 0;
	int8_t		ret = -1;
	
	for(i = 0; i < param_len; i++)
	{
		if(param[i] >= '0' && param[i] <= '9')
		{
			result = result*10 + (param[i] - '0');
			ret = result;
		}
	}
	
	return ret;
}

void get_pc_params(uint8_t * param, uint8_t param_len)
{
	uint8_t		i;
	
	for(i = 0; i < 27; i++)
		MTK_RF_TEST_COMMAND[i] = param[i];
}

int parse_command(uint8_t * cmd, uint8_t cmd_len)
{
	int8_t		param = -1;
	
	if(strncmp("pccmd", (const char*)cmd, 5) == 0 )
	{
		get_pc_params(&cmd[5], cmd_len - 5);
		BuartSend((uint8_t*)MTK_RF_TEST_COMMAND, sizeof(MTK_RF_TEST_COMMAND));
		return 0;
	}
	
	if(cmd_len > 5)
	{
		param = get_params(&cmd[5], cmd_len - 5);
	
		if(param == -1)
			FuartSend((uint8_t  *)error_param_msg, sizeof(error_param_msg));
	}
	
	if( strncmp("pttrn", (const char*)cmd, 5) == 0 )
	{
		if(param != -1)
			MTK_RF_TEST_CMD_set_tx_pattern(param);
		return 0;
	}
	else if(strncmp("sngle", (const char*)cmd, 5) == 0)
	{
		if(param != -1)
			MTK_RF_TEST_CMD_set_single_hopping(param);
		return 0;
	}
	else if(strncmp("txfrq", (const char*)cmd, 5) == 0)
	{
		if(param != -1)
			MTK_RF_TEST_CMD_set_tx_freq(param);
		return 0;
	}
	else if(strncmp("rxfrq", (const char*)cmd, 5) == 0)
	{
		if(param != -1)
			MTK_RF_TEST_CMD_set_rx_freq(param);
		return 0;
	}
	else if(strncmp("ptype", (const char*)cmd, 5) == 0)
	{
		if(param != -1)
			MTK_RF_TEST_CMD_set_packet_type_length(param);
		return 0;
	}
	else if(strncmp("cmdgo", (const char*)cmd, 5) == 0)
	{
		MTK_RF_TEST_CMD_go();
		return 0;
	}
	else
	{
		FuartSend((uint8_t *)unknow_cmd_msg, sizeof(unknow_cmd_msg));
		print_usage();
	}
	return -1;
}
