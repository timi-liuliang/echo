//#pragma once
///*
//	1 目前只能追踪F8及FF15方式的调用，这两种占绝大多数
//	2 目前每次追踪上线为65536，需要调用Tick来输出和重置，否则会溢出，因为效率原因，没有对溢出做限制，要自己注意.
//	3 基本原理是调用TrackFunctions之后，会扫描代码段，然后记录所有的调用备用，然后用户请求追踪某个函数时，就在记录中查找指定的函数调用点，修改为跳转到CallTracker_CallSheel,CallTracker_CallSheel进行记录后，再调用该函数.
//	4 调用示范
//		Echo::CallTrackManager::instance().Initialize();
//		Echo::CallTrackManager::instance().Tick();
//		Echo::CallTrackManager::instance().Release();
//	5 目前只有WINDOW下追踪可以用，以后增加其他平台的追踪.
//	6 CallTracker_CallSheel中加有锁，所以异步调用不会有问题，但是在CallTracker_CallSheel中加锁也就是给追踪函数加锁，可能会遇到的问题，要自己考虑清楚。
//*/
//
//#include "Foundation/Base/EchoDef.h"
//#include <map>
//#include <vector>
//
//namespace Echo
//{
//#ifdef ECHO_PLATFORM_WINDOWS
//	Dword __stdcall GetOpCodeSize(unsigned char* iptr0);
//#endif
//
//	class ECHO_EXPORT_FOUNDATION CallTrackModule
//	{
//		friend 	void libEvent_Read(struct bufferevent*,void*);
//	public:
//		CallTrackModule(const char* pathName,const char* fileName);
//		~CallTrackModule(void);
//		
//		// 输出追踪信息
//		void OutputTrackerInfo(FILE* fp);
//		// 获取名字
//		const std::string& GetPDBPathName(){ return m_sPDBPathName; }
//		// 获取名字
//		const std::string& GetModuleName(){ return m_sFileName; }
//		// 获取全部符号的名字
//		const std::map<std::string,void*>& GetAllSymload(){ return m_SymloadList; };
//		// 获取查询地址
//		void* ToSymloadAddress(void* address);
//		// 加载
//		bool Load(const std::vector<unsigned int>& e8Call,const std::vector<unsigned int>& ff15Call);
//		// 卸载
//		void Unload();
//		// 追踪
//		int Track(char* funName);
//		// 取消追踪
//		int CancelTrack(char* funName);
//
//	private:
//		void TrackCall(const std::string& callName,unsigned int codeAddress,unsigned int jumpToAddress);
//		void* GetFunctionAddress(const char* sName);
//		bool CheckAddress(void* address);
//		size_t GetPageSize(){ return 4096; }
//		bool EnablePageReadWrite(void* address,size_t size);
//		bool EnablePageReadWriteByte(void* address);
//		const std::map<std::string,void*> GetFunctionDist(const std::vector<std::string>& vsFunction);
//
//	private:
//		unsigned long	m_BaseOfDll;
//		void*			m_BaseAddress;
//		std::string		m_sPDBPathName;
//		std::string		m_sFileName;
//		//std::map<std::string,std::vector<void*>> m_TrackRecordList;
//
//		std::map<std::string,void*>	m_SymloadList;//所有的符号
//		std::map<void*,std::string>	m_QueryList;//通过这个表快速找到当前位置对应的函数名字
//		std::map<void*,void*> m_AddrToSymAddrList; //实际代码地址到模块文件地址之间的转换。
//		typedef struct
//		{
//			unsigned int address;
//			unsigned int jumpTo;
//			unsigned int queryPos;
//		}CallPos;
//		std::multimap<std::string,CallPos> m_e8CallPos;//调用位置
//		std::multimap<std::string,CallPos> m_ff15CallPos;//调用位置
//		typedef struct 
//		{
//			unsigned int  callAddress;//代码中的位置
//			unsigned int  allocAddress;//分配地址
//			unsigned char oldCode[6];//原始代码
//		}TrackRecord;
//		std::multimap<std::string,TrackRecord> m_callRecord;//追踪记录，用于恢复
//	};
//
//	class ECHO_EXPORT_FOUNDATION CallTrackManager
//	{
//		friend struct bufferevent*& getClientEvent();
//		friend void libEvent_Read(struct bufferevent* bev,void* ctx);
//		friend void libEvent_Event(struct bufferevent* bev,short events,void* ctx);
//
//		std::vector<CallTrackModule*> ma_Modules;
//		struct event_base* m_base;
//		std::vector<unsigned int> ma_e8Call;
//		std::vector<unsigned int> ma_ff15Call;
//	public:
//		CallTrackManager();
//		~CallTrackManager();
//		static CallTrackManager& instance();				// 单件
//		bool Initialize();									// 初始化
//		void Tick();										// 每帧进行处理
//		void Release();										// 关闭
//		static std::vector<std::string> GetModuleList();	// 获取所有模块的名字
//	private:
//		int ListenCall(void* callAddress);
//		int ListenCall(const char* callList);				// 监听某个调用
//		int CancelListenCall(void* callAddress);			// 取消监听
//		int CancelListenCall(const char* callList);			// 取消监听
//		std::vector<std::string> ToVectorString(const char* str);	//转化串为数组串
//		bool CheckAddress(void* address);					// 检查一个地址是否有效
//	private:
//		struct evconnlistener*		m_listener_event;
//		struct bufferevent*			m_client_event;
//	public:
//		void OnMsg(struct bufferevent* bev,void* ctx);	//处理消息
//		void OnMsg_GetModuleList(struct bufferevent* bev,void* ctx);//请求模块表
//		void OnMsg_LoadModule(struct bufferevent* bev,void* ctx);//加载模块表
//		void OnMsg_UnloadModule(struct bufferevent* bev,void* ctx);//释放模块表
//		void OnMsg_GetSymloadList(struct bufferevent* bev,void* ctx);
//		void OnMsg_Track(struct bufferevent* bev,void* ctx); //追踪
//		void OnMsg_CancelTrack(struct bufferevent* bev,void* ctx); //取消追踪
//	};
//}
