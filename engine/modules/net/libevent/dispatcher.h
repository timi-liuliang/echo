#pragma once

#include "base.h"

namespace network
{
	/**
	 * 消息回调基类
	 */
	class msg_call_back
	{
	public:
		virtual ~msg_call_back() {}
		
		// 消息处理
		virtual void on_accept_msg( evutil_socket_t fd, message_lite* msg)=0;
	};

	/**
	 * 消息回调模板继承类
	 */
	template <typename T>
	class msg_call_back_t : public msg_call_back
	{
	public:
		typedef std::function<void(evutil_socket_t, const T*)> recv_msg_cb;

		// 构造函数
		msg_call_back_t(const recv_msg_cb& cb)
			: m_cb(cb)
		{}

		// 消息处理
		virtual void on_accept_msg( evutil_socket_t fd, message_lite* msg)
		{
			T* t = dynamic_cast<T*>(msg);
			if (t)
			{
				m_cb(fd, t);
			}
		}

	private:
		recv_msg_cb		m_cb;
	};


	/**
	 * 消息分发
	 */
	class dispatcher
	{
	public:
		// 消息处理回调
		struct message_process
		{
			message_lite*	m_msg;
			msg_call_back*	m_cb;
		};
		typedef std::map<unsigned int, message_process> message_template;

	public:
		dispatcher() {}
		~dispatcher();

		// 设置消息回调
		template<typename T> bool register_msg_cb( const typename msg_call_back_t<T>::recv_msg_cb& cb);

		// 接受到信息
		void on_accept_msg(evutil_socket_t fd, message_packet& packet);

	private:
		message_template			m_msg_template;	// 消息模板ss
	};

	// 析构函数
	inline dispatcher::~dispatcher()
	{
		for (message_template::iterator it = m_msg_template.begin(); it != m_msg_template.end(); it++)
		{
			delete it->second.m_cb;
			delete it->second.m_msg;
		}

		m_msg_template.clear();
	}

	// 注册消息回调
	template<typename T>
	inline bool dispatcher::register_msg_cb( const typename msg_call_back_t<T>::recv_msg_cb& cb)
	{
		T* tmsg = new T;
		msg_call_back* msg_cb = new msg_call_back_t<T>(cb);
		unsigned int hash_key = bkdr_hash(tmsg->GetTypeName().c_str());
		if (m_msg_template.find(hash_key) == m_msg_template.end())
		{
			message_process process;
			process.m_msg = tmsg;
			process.m_cb = msg_cb;
			m_msg_template[hash_key] = process;

			return true;
		}
		else
		{
			// 同一消息类型多次注册
			assert(false);

			return false;
		}
	}

	// 接受到信息
	inline void dispatcher::on_accept_msg(evutil_socket_t fd, message_packet& packet)
	{
		message_template::iterator it = m_msg_template.find(packet.m_header.m_pack_id);
		if (it != m_msg_template.end())
		{
			const message_process& mp = it->second;
			message_lite* msg = mp.m_msg->New();
			if (msg->ParseFromArray(packet.get_body(), packet.get_body_len()))
			{
				// 调用处理函数      
				mp.m_cb->on_accept_msg(fd, msg);
			}
			else
			{
				// 消息反序列化失败
			}

			delete(msg);
		}
		else
		{
			// 接收到未知消息
		}
	}
}