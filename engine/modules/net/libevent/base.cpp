#include "base.h"
#include <stdio.h>

namespace network
{
	// 初始化
	void network_init()
	{
#if defined(WIN32) || defined(WIN64)
		// Initialize Winsock.
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR)
			printf("Error at WSAStartup()\n");
#endif
	}

	// 卸载
	void network_uninit()
	{
#if defined(WIN32) || defined(WIN64)
		WSACleanup();
#endif
	}

	// BKDR 哈希函数
	unsigned int bkdr_hash(const char* str)
	{
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
		unsigned int hash = 0;

		while (*str)
			hash = hash * seed + (*str++);

		return (hash & 0x7FFFFFFF);
	}

	// 构造函数
	message_packet::message_packet(evutil_socket_t fd)
		: m_fd( fd)
		, m_idx( 0)
	{

	}

	// 接收消息
	bool message_packet::on_accept_data(char c)
	{
		if (m_idx < sizeof(header))
		{
			m_data[m_idx] = c;
			m_idx++;

			// 检测是否为空数据包
			if (m_idx == sizeof(header) && m_header.m_body_len==0)
				return true;
			else
				return false;
		}
		else if (m_idx < get_data_len())
		{
			m_data[m_idx] = c;
			m_idx++;

			if (m_idx == get_data_len())
				return true;
			else
				return false;
		}
		else
		{
			assert(false);
			return false;
		}
	}
}