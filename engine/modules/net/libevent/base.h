#pragma once

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <functional>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <google/protobuf/message_lite.h>

#define  gp google::protobuf

namespace network
{
	typedef google::protobuf::MessageLite message_lite;

	/**
	 * 消息包
	 */
	struct message_packet
	{
		struct header
		{
			gp::uint32  m_body_len;		// 数据长度(不包含头)
			gp::uint32	m_pack_id;		// 包ID
		};

		evutil_socket_t m_fd;			// 包来源
		gp::uint32		m_idx;			// 数据游标

		union
		{
			header	m_header;
			char	m_data[4096];		// 包整数据
		};

		// 构造函数
		message_packet(evutil_socket_t fd);

		// 获取数据
		char* get_data() { return m_data; }

		// 获取容量
		int get_data_capacity() { return sizeof(m_data); }

		// 获取包长度
		gp::uint32 get_data_len() { return m_header.m_body_len + sizeof(header); }

		// 获取主体数据
		char* get_body() { return m_data + sizeof(header); }

		// 获取主体数据空间
		gp::uint32 get_body_space() { return 4096 - sizeof(header); }

		// 获取主体数据长度
		gp::uint32 get_body_len() { return m_header.m_body_len; }

		// 压缩
		void compress(){}

		// 解压缩
		void decompress(){}

		// 接收消息
		bool on_accept_data(char c);
	};

	/**
	 * 网络连接
	 */
	struct connection
	{
		evutil_socket_t		m_fd;		// 套接字句柄
		message_packet*		m_packet;	// 正在处理的数据包

		// 构造函数
		connection()
			: m_fd(-1)
			, m_packet(nullptr)
		{}

		// 析构函数
		~connection()
		{
		}
	};

	// 初始化
	void network_init();

	// 卸载
	void network_uninit();

	// BKDR 哈希函数
	unsigned int bkdr_hash(const char* str);
}