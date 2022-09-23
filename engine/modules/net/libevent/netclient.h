#pragma once

#include "base.h"
#include "dispatcher.h"

namespace network
{
	/**
	 * 网络客户端
	 */
	class client
	{
	public:
		client();
		~client();

		// 联接服务器
		bool connect(const char* ip, int port);

		// 发送数据
		void send(const void* data, int size);

		// 发送数据
		void send(const message_lite& msg_lite);

		// 设置消息回调
		template<typename T> bool register_msg_cb(const typename msg_call_back_t<T>::recv_msg_cb& cb) { return m_dispatcher.register_msg_cb<T>(cb);}

		// 处理已接收消息
		void client::process();

		event* get_ev_sockfd(){ return m_ev_sockfd; }

	private:
		// 链接
		void connect_internel(const char* ip, int port);

	public:
		// 接受到信息
		void on_accept_msg(evutil_socket_t fd, char* msg, int len);

		// 获取下一条要处理的消息
		message_packet* get_next_msg();

	private:
		std::thread*					m_thread;		// 线程句柄
		event_base*						m_base;			// 事件基础
		event*							m_ev_sockfd;	// 套接字事件
		dispatcher						m_dispatcher;	// 消息分发
		std::mutex						m_mutex;		// 互斥量
		message_packet*					m_packet;		// 组装中的数据包
		std::queue<message_packet*>		m_accepted_msgs;// 接收到的消息
	};
}