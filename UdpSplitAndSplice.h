#ifndef _UDP_TEST_H_
#define _UDP_TEST_H_

#define USING_IN_STM32 0

#if USING_IN_STM32
	#include "stm32f4xx.h"
	#include "Console.h"
#else
	
#endif

#include "string.h"

namespace
{
	const uint16_t SPLIT_PACKET_SIZE = 304;
	const uint16_t SPLIT_DATA_SIZE = 297;
	const uint16_t STORE_DATA_MAX_SIZE = 2048;
	const uint16_t SPLIT_DATA_MAX_SIZE = 2048;
	const uint8_t SPLIT_PACKET_MAX_NUM = 10;			//70
	struct udp_little_packet			//one struct 304 bytes
	{
		uint8_t udp_head;
		uint8_t udp_index;
		uint16_t udp_data_size;
		uint16_t udp_size;
		uint8_t udp_packet_num;		
		uint8_t udp_data[SPLIT_DATA_SIZE];	
	};
}

class CUdpSplitAndSplice
{
private:
	uint8_t _store_data[STORE_DATA_MAX_SIZE];
	uint16_t _recieve_data_size;
	bool _is_recieve_whole_packet;
	uint16_t _send_packet_num;
	udp_little_packet little_packet[SPLIT_PACKET_MAX_NUM];
public:
	CUdpSplitAndSplice();
	~CUdpSplitAndSplice(){}
	bool spliceData(uint8_t* rec_data, uint16_t rec_len);
	bool splitData(uint8_t* send_data, uint16_t data_size);
	void clearStoreData(){memset(_store_data,0x00,STORE_DATA_MAX_SIZE);}
	uint16_t getSendDataSize(){return 0;}
	uint16_t getSendPacketNum(){return _send_packet_num;}
	uint8_t* getSendData(uint8_t packet_index){return (uint8_t*)(&(little_packet[packet_index]));}
	bool isSpliceFinshed(){return _is_recieve_whole_packet;}
	uint8_t* getStoreData()
	{	
		if(_is_recieve_whole_packet)
			return _store_data;
		else
			return NULL;
	}
	uint16_t getStoreDataSize()
	{
		if(_is_recieve_whole_packet)
			return _recieve_data_size;
		else
			return 0;
	}	
};

CUdpSplitAndSplice::CUdpSplitAndSplice()
{
	_send_packet_num = 0;
	_is_recieve_whole_packet = false;
	_recieve_data_size = 0;
	memset(_store_data,0x00,STORE_DATA_MAX_SIZE);
}

bool CUdpSplitAndSplice::splitData(uint8_t* send_data, uint16_t data_size)
{
	if(data_size > SPLIT_DATA_MAX_SIZE)
	{
		#if USING_IN_STM32
		Console::Instance()->printf("data_size is over SPLIT_DATA_MAX_SIZE:2048\r\n");
		#else
		printf("data_size is over SPLIT_DATA_MAX_SIZE:2048\r\n");
		#endif
		return false;
	}
	uint16_t packet_data_max_size = SPLIT_DATA_SIZE;
	bool is_exact_division = (data_size % packet_data_max_size) > 0 ? false : true;
	_send_packet_num = is_exact_division ? (data_size/packet_data_max_size) : (data_size/packet_data_max_size+1);
	//Console::Instance()->printf("split data packet num is %d\r\n",_send_packet_num);
	for (int i = 0; i<_send_packet_num; i++)
	{
		little_packet[i].udp_head = 0x5A;
		little_packet[i].udp_index = i;
		little_packet[i].udp_packet_num = (uint8_t)_send_packet_num;
		
		if (i != _send_packet_num - 1)			//²»ÊÇÄ©±¨
		{
			little_packet[i].udp_data_size = (uint16_t)packet_data_max_size;
		}
		else
		{
			if (is_exact_division)
			{
				little_packet[i].udp_data_size = (uint16_t)packet_data_max_size;
			}	
			else
			{
				little_packet[i].udp_data_size = (uint16_t)(data_size - (_send_packet_num-1) * packet_data_max_size);
				//Console::Instance()->printf("last packet size is %d\r\n",little_packet[i].udp_data_size);
			}
		}
		little_packet[i].udp_size = data_size;		
		memcpy(little_packet[i].udp_data, send_data + i * packet_data_max_size, little_packet[i].udp_data_size);
		
	}
	return true;
}

bool CUdpSplitAndSplice::spliceData(uint8_t* rec_data, uint16_t rec_len)
{
	if(rec_len > SPLIT_PACKET_SIZE)
	{
		#if USING_IN_STM32
		Console::Instance()->printf("rec_len is over SPLIT_PACKET_SIZE:304\r\n");
		#else
		printf("rec_len is over SPLIT_PACKET_SIZE:304\r\n");
		#endif
		return false;
	}
	_is_recieve_whole_packet = false;
	if(rec_data[0] == 0x5A)
	{
		uint16_t udp_packet_size = ((uint16_t)rec_data[3] << 8) + (uint16_t)rec_data[2];
		//Console::Instance()->printf("udp_packet_size is %d \r\n", udp_packet_size);
		uint16_t udp_total_size = ((uint16_t)rec_data[5] << 8) + (uint16_t)rec_data[4];
		//Console::Instance()->printf("udp_total_size is %d \r\n", udp_total_size);
		uint8_t udp_packet_index = rec_data[1];
		//Console::Instance()->printf("packet index is %d \r\n", udp_packet_index);
		uint8_t udp_packet_num_max = rec_data[6];
		//Console::Instance()->printf("packet num is %d \r\n", udp_packet_num_max);
		if(udp_packet_index == 0)
		{
			clearStoreData();
			_recieve_data_size = 0;
		}
		_recieve_data_size += udp_packet_size;
		if(udp_packet_size == rec_len-7 || udp_packet_index == udp_packet_num_max-1)
		{
			memcpy(_store_data+(udp_packet_index)*SPLIT_DATA_SIZE, rec_data+7, udp_packet_size);
			if(udp_packet_index == (udp_packet_num_max-1))
			{
				_is_recieve_whole_packet = true;
				//Console::Instance()->printf("this is the last index packet \r\n");
			}
		}
		else
		{
			#if USING_IN_STM32
			Console::Instance()->printf("packet size unequal with recieve size, rec is %d, but packet size is %d\r\n", rec_len, udp_packet_size);
			#else
			printf("packet size unequal with recieve size, rec is %d, but packet size is %d\r\n", rec_len, udp_packet_size);
			#endif
			return false;
		}
		return true;
	}
	else 
	{
		#if USING_IN_STM32
		Console::Instance()->printf("packet head error");
		#else
		printf("packet head error");
		#endif
		return false;
	}
}

#endif //_UDP_TEST_H_