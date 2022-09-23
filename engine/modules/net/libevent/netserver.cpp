#include "netserver.h"
#include "base.h"
#include <assert.h>

namespace network
{
	// 读事件回调
	static void socket_read_cb( evutil_socket_t fd, short events, void* arg)
	{
		server* sev = (server*)arg;

		char msg[1024];
		int len = ::recv(fd, msg, 1024, 0);
		while (len>0)
		{
			// 由服务器类处理消息
			sev->on_accept_msg(fd, msg, len);

			// 继续接收
			len = ::recv(fd, msg, 1024, 0);
		}
	}

	// 接收到联接回调
	static void accept_cb(evutil_socket_t fd, short events, void* arg)
	{
		server* sev = (server*)arg;
		event_base* base = (event_base*)sev->get_base();

		evutil_socket_t sockfd;
		sockaddr_in sin_client;
		socklen_t len = sizeof(sin_client);			// len必须有初值

		sockfd = ::accept( fd, (sockaddr*)&sin_client, &len);
		if (sockfd == -1)
		{
			return;
		}

		// 创建事件
		event* ev = event_new(NULL, -1, 0, NULL, NULL);

		// 创建事件
		event_assign(ev, base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, (void*)sev);
		event_add(ev, NULL);

		sev->on_accept_connect(sockfd);
	}


	// 构造函数
	server::server()
		: m_base(NULL)
		, m_ev_listener(NULL)
		, m_port(-1)
		, m_thread(NULL)
		, m_max_conn_num(0)
	{
		network_init();
	}

	// 析构函数 (目前退出处理不正确)
	server::~server()
	{
// 		event_base_loopexit(m_base, nullptr);
// 		event_base_free(m_base);
// 
// 		network_uninit();
// 
// 		// 等待线程结束
// 		m_thread->join();
	}

	// 启动服务
	bool server::start( int port, int listen_num)
	{
		m_port = port;
		m_max_conn_num = listen_num;

		evutil_socket_t listener;

		listener = ::socket(AF_INET, SOCK_STREAM, 0);
		if (listener == -1)
			return false;

		evutil_make_listen_socket_reuseable(listener);

		// 初始化网络地址
		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = 0;
		sin.sin_port = htons(m_port);

		// 绑定
		if (::bind(listener, (sockaddr*)&sin, sizeof(sin)) < 0)
			return false;

		// 开始监听
		if (::listen(listener, listen_num) < 0)
			return false;

		evutil_make_socket_nonblocking(listener);

		// 新建运行基础
		m_base = event_base_new();
		if (!m_base)
			return false;

		// 新建监听事件
		event* ev_listen = event_new( m_base, listener, EV_READ | EV_PERSIST, accept_cb, this);
		event_add(ev_listen, NULL);
		
		// 启动主循环
		auto function = std::bind(&server::listen_internel, this);
		m_thread = new std::thread(function, this);

		return true;
	}

	// 链接
	void server::listen_internel()
	{
		// 循环处理事件
		event_base_dispatch(m_base);
	}

	// 发送数据到客户端
	void server::send(evutil_socket_t fd, void* data, int size)
	{
		::send(fd, (const char*)data, size, 0);
	}

	// 发送数据
	void server::send(evutil_socket_t fd, const message_lite& msg_lite)
	{
		message_packet packget(fd);
		packget.m_header.m_body_len = msg_lite.ByteSize();
		packget.m_header.m_pack_id = bkdr_hash(msg_lite.GetTypeName().c_str());

		// 打包数据
		if (msg_lite.SerializeToArray(packget.get_body(), packget.get_body_space()))
		{
			send( fd, packget.get_data(), packget.get_data_len());
		}
	}

	// 接受新的链接
	void server::on_accept_connect(evutil_socket_t fd)
	{
		connection conn;
		conn.m_fd = fd;
		m_conns[fd] = conn;
	}

	// 停止链接
	void server::on_stop_connect(evutil_socket_t fd)
	{
		connection_map::iterator it = m_conns.find(fd);
		if (it != m_conns.end())
		{
			assert(it->second.m_fd == fd);
			::closesocket(fd);
			m_conns.erase(it);
		}
	}

	// 接受到信息
	void server::on_accept_msg(evutil_socket_t fd, char* msg, int len)
	{
		// 创建新包，或获取未组装完包
		message_packet* packet = m_conns[fd].m_packet;

		// 组装包
		int msg_processed = 0;
		for (int i = 0; i < len; i++)
		{
			// 新建包
			if (!packet)
			{
				packet = new message_packet(fd);
			}

			if (packet->on_accept_data(msg[i]))
			{
				// 组装完成
				std::lock_guard<std::mutex> lock(m_mutex);
				m_accepted_msgs.push(packet);
				
				packet = nullptr;
				msg_processed = i + 1;

				break;
			}
		}

		// 包未组装完成，记录之
		m_conns[fd].m_packet = packet;

		// 继续处理数据
		if (msg_processed < len)
		{
			on_accept_msg(fd, msg + msg_processed, len - msg_processed);
		}
	}

	// 获取下一条要处理的消息
	message_packet* server::get_next_msg()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_accepted_msgs.empty())
		{
			message_packet* packet = m_accepted_msgs.front();
			m_accepted_msgs.pop();

			return packet;
		}

		return nullptr;
	}

	// 处理已接收消息
	void server::process()
	{
		while (message_packet* packet = get_next_msg())
		{
			m_dispatcher.on_accept_msg(packet->m_fd, *packet);
			delete packet;
		}
	}
}