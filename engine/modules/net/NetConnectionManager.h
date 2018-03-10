#pragma once

#include "engine/core/Memory/MemAllocDef.h"

#ifdef ECHO_USE_LIBEVENT
extern "C"
{
	struct event_base;
	struct evhttp_uri;
	struct evhttp_connection;
	struct evhttp_request;
}

namespace Echo
{
	class NetErrCode
	{
	public: 
		enum ErrCode
		{
			HA_REQUEST_TIME_OUT,  // 超时
			HA_REQUEST_CONN_LOST, // 连接丢失
			HA_REQUEST_OK = 0
		};
	};

	typedef void(*onRequestCompleteCb)(void* arg, const Echo::String& fileName, void* data, size_t len, NetErrCode::ErrCode err);
	
	class NetConnection : public ObjectAlloc
	{
		NetConnection(NetConnection& rhs); 
		NetConnection& operator=(NetConnection& rhs); 

	public: 
		NetConnection(); 
		~NetConnection(); 

	public:
		// 构造连接信息
		void createConnection(const Echo::String& url); 

		// 设置完成时回调
		void setRequestCompleteCb(onRequestCompleteCb cb);

		// 请求文件
		void requestFile(const String& url, void* arg, const String& fileName);

		// 请求完成回调
		void onRequestComplete(evhttp_request* request); 

	private:
		struct NetContext
		{
			String name; 
			void* pThis; 
		};

	private: 
		onRequestCompleteCb m_onRequestCompleteCallback; 

		struct evhttp_uri*				m_uri;
		struct evhttp_connection*		m_con;

		map<String, NetContext>::type   m_contexts; 
	};


	class NetConnectionManager : public ObjectAlloc
	{
		NetConnectionManager(NetConnectionManager& rhs); 
		NetConnectionManager& operator=(NetConnectionManager& rhs); 

	public: 
		NetConnectionManager(); 
		~NetConnectionManager(); 

	public: 
		// 新建网络连接
		void createNetConnection(const Echo::String& url); 

		// 移除网络连接
		void removeNetConnection(const Echo::String& url); 

		// 获取网络连接
		NetConnection* getNetConnection(const Echo::String& url); 

		// 获取Http模块
		event_base* getNetConnectionBase() const { return m_eventBase; }
	
	private:
		event_base*						  m_eventBase; 
		map<String, NetConnection*>::type m_netConnectinos;
		
	};
}

#else
namespace Echo
{
	class NetConnectionManager {};
}
#endif
