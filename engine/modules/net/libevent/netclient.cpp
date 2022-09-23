#include "netclient.h"

namespace network
{
	// 读回调
	static void socket_read_cb(evutil_socket_t fd, short events, void* arg)
	{
		client* ct = (client*)arg;

		char msg[1024];
		int len = ::recv(fd, msg, 1024, 0);
		while (len > 0)
		{
			// 由服务器类处理消息
			ct->on_accept_msg(fd, msg, len);

			// 继续接收
			len = ::recv(fd, msg, 1024, 0);
		}
	}

	// 构造函数
	client::client()
		: m_base(nullptr)
		, m_thread(nullptr)
		, m_ev_sockfd(nullptr)
		, m_packet(nullptr)
	{
		network_init();
	}

	// 析构函数(退出函数有问题)
	client::~client()
	{
// 		event_base_loopexit(m_base, nullptr);
// 		event_base_free(m_base);
// 
// 		network_uninit();
// 
// 		// 等待线程结束
// 		m_thread->join();
	}

	// 联接服务器
	bool client::connect(const char* ip, int port)
	{
		evutil_socket_t sockfd;
		int status;

		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(ip);
		sin.sin_port = htons(port);

		// 新建套接字
		sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
		if (sockfd == -1)
			return false;

		// 联接服务器
		status = ::connect(sockfd, (sockaddr*)&sin, sizeof(sin));
		if (status == -1)
		{
			::closesocket(sockfd);
			return false;
		}

		// 设置套接字不阻塞
		evutil_make_socket_nonblocking(sockfd);

		// 添加事件
		m_base = event_base_new();
		m_ev_sockfd = event_new(m_base, sockfd, EV_READ | EV_PERSIST, socket_read_cb, this);
		event_add(m_ev_sockfd, NULL);

		// 启动主循环
		auto function = std::bind(&client::connect_internel, this, ip, port);
		m_thread = new std::thread(function, this);

		return true;
	}

	// 链接
	void client::connect_internel(const char* ip, int port)
	{
		// 循环处理事件
		event_base_dispatch(m_base);
	}

	// 发送数据
	void client::send( const void* data, int size)
	{
		if (m_ev_sockfd == NULL)
		{
			return;
		}
		evutil_socket_t sockefd = event_get_fd(m_ev_sockfd);
		
		::send(sockefd, (const char*)data, size, 0);
	}

	// 发送数据
	void client::send(const message_lite& msg_lite)
	{
		message_packet packget(0);
		packget.m_header.m_body_len = msg_lite.ByteSize();
		packget.m_header.m_pack_id = bkdr_hash(msg_lite.GetTypeName().c_str());

		// 打包数据
		if (msg_lite.SerializeToArray(packget.get_body(), packget.get_body_space()))
		{
			send( packget.get_data(), packget.get_data_len());
		}
	}

	// 接受到信息
	void client::on_accept_msg(evutil_socket_t fd, char* msg, int len)
	{
		// 创建新包，或获取未组装完包
		message_packet* packet = m_packet;

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
			}
		}

		// 包未组装完成，记录之
		m_packet = packet;

		// 继续处理数据
		if (msg_processed < len)
		{
			on_accept_msg(fd, msg + msg_processed, len - msg_processed);
		}
	}

	// 获取下一条要处理的消息
	message_packet* client::get_next_msg()
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
	void client::process()
	{
		while (message_packet* packet = get_next_msg())
		{
			m_dispatcher.on_accept_msg(packet->m_fd, *packet);
			delete packet;
		}
	}
}