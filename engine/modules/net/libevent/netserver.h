#pragma once

#include "base.h"
#include "dispatcher.h"

namespace network
{
	/**
	 * 网络服务
	 */
	class server
	{
		typedef std::map<evutil_socket_t, connection> connection_map;
	public:
		server();
		~server();

		// 启动服务
		bool start( int port, int listen_num=2048);

		// 发送数据到客户端
		void send( evutil_socket_t fd, void* data, int size);

		// 发送数据
		void send( evutil_socket_t fd, const message_lite& msg_lite);

		// 获取连接数量
		int get_conn_num() const { return (int)m_conns.size(); }

		// 设置消息回调
		template<typename T> bool register_msg_cb(const typename msg_call_back_t<T>::recv_msg_cb& cb) { return m_dispatcher.register_msg_cb<T>(cb); }

		// 处理已接收消息
		void process();

	public:
		// 链接
		void listen_internel();

		// 获取base
		event_base* get_base() { return m_base; }

		// 接受新的链接
		void on_accept_connect(evutil_socket_t fd);

		// 停止链接
		void on_stop_connect(evutil_socket_t fd);

		// 接受到信息
		void on_accept_msg(evutil_socket_t fd, char* msg, int len);

		// 获取下一条要处理的消息
		message_packet* get_next_msg();

	private:
		std::thread*				m_thread;		// 线程句柄
		int							m_port;			// 被监听的端口号
		int							m_max_conn_num;	// 最大客户端联接数量
		event_base*					m_base;			// 基础(所有libevent操作必须有一个base)
		event*						m_ev_listener;	// 监听器
		connection_map				m_conns;		// 所有链接
		dispatcher					m_dispatcher;	// 消息分发器
		std::mutex					m_mutex;		// 互斥量
		std::queue<message_packet*>	m_accepted_msgs;// 接收到的消息
	};
}