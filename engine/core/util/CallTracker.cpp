//#include "Foundation/Util/LogManager.h"
//#include "Foundation/Thread/Threading.h"
//#include "CallTracker.h"
//#include <set>
//#ifdef ECHO_PLATFORM_WINDOWS
//	#pragma warning(disable:4996)
//	#pragma warning(disable:4731)
//#include <windows.h>
//#include <assert.h>
//#include <DbgHelp.h>
//	#pragma comment(lib,"Dbghelp.lib")
//#include <psapi.h>
//#endif
//
//namespace Echo
//{
//	typedef struct
//	{
//		void*				fun;
//		unsigned long		high1;
//		unsigned long		low1;
//		unsigned long		high2;
//		unsigned long		low2;
//		unsigned short		level;//栈深度
//	}TrackInfo;
//
//	typedef struct  
//	{
//		int					currentEBP;
//		int					currentRetAddress;
//		int					currentParam;
//		TrackInfo*			currentTrackInfo;
//	}TrackStackInfo;
//
//	static TrackInfo		s_ti[0x10000];		//追踪信息
//	static TrackStackInfo	s_tiStack[0x10000];	//栈信息
//
//	TrackInfo*				s_tiPtr=s_ti;
//	static TrackStackInfo*	s_tiStackPtr=s_tiStack;
//	unsigned int			s_tiIndex=0;
//	unsigned int			s_tiStackLevel=0;
//
//	static int				s_EBP;//栈地址
//	static void*			s_ECX;//THIS指针
//	static int				s_EAX;//返回值
//	static int				s_RetAddress;
//	static int				s_Param;
//
//	static unsigned long	s_high;
//	static unsigned long	s_low;
//
//	static const unsigned int s_jmpLength=5;	//长跳转指令长度
//	static const unsigned int s_e8InstructLength=5;	//E8调用指令长度
//	static const unsigned int es_ff15InstructLength=6;//FF15调用指令长度
//	static const unsigned int s_WindowUserAddressSize=0x80000000;
//
//	static Echo::Mutex	s_lock;
//	static int			s_lockBuf1;
//	static int			s_lockBuf2;
//	static int			s_lockBuf3;
//	static int			s_lockBuf4;
//	static int			s_lockBuf5;
//	static int			s_lockBuf6;
//	static int			s_lockBuf7;
//	static int			s_lockBuf8;
//	static int			s_lockBuf9;
//	static int			s_lockBuf10;
//
//#ifdef ECHO_PLATFORM_WINDOWS
//	static LARGE_INTEGER	s_CPUFrequency;
//#endif
//
//	// 封套函数。
//	void CallTracker_CallSheel(void* funAddress)
//	{
//#if defined(ECHO_PLATFORM_WINDOWS) && !defined(_WIN64)
//
//		_asm push ecx;
//		_asm push eax;
//		s_lock.Lock();
//		_asm pop eax;
//		_asm pop ecx;
//
//		_asm pop s_EBP;
//		_asm pop s_RetAddress;
//		_asm pop s_Param;
//		_asm mov s_ECX,ecx;
//		_asm mov s_EAX,eax;
//
//		_asm RDTSC;
//		_asm mov s_high,edx;
//		_asm mov s_low,eax;
//
//		s_tiPtr->high1=s_high;
//		s_tiPtr->low1=s_low;
//
//		s_tiStackPtr->currentEBP=s_EBP;
//		s_tiStackPtr->currentRetAddress=s_RetAddress;
//		s_tiStackPtr->currentParam=s_Param;
//		s_tiStackPtr->currentTrackInfo=s_tiPtr;
//		s_tiPtr->fun=(void*)s_Param;
//		s_tiPtr->level=s_tiStackLevel;
//		++s_tiStackLevel;
//		++s_tiStackPtr;
//		++s_tiPtr;
//		++s_tiIndex;
//
//		_asm mov ecx,s_ECX;
//		_asm mov eax,s_EAX;
//		_asm call s_Param;
//		_asm mov s_EAX,eax;
//
//		--s_tiStackPtr;
//		--s_tiStackLevel;
//		s_EBP=s_tiStackPtr->currentEBP;
//		s_RetAddress=s_tiStackPtr->currentRetAddress;
//		s_Param=s_tiStackPtr->currentParam;
//
//		_asm push s_Param;
//		_asm push s_RetAddress;
//		_asm push s_EBP;
//		_asm RDTSC;
//		_asm mov s_high,edx;
//		_asm mov s_low,eax;
//
//		s_tiStackPtr->currentTrackInfo->high2=s_high;
//		s_tiStackPtr->currentTrackInfo->low2=s_low;
//
//		_asm pop s_lockBuf1;
//		_asm pop s_lockBuf2;
//		_asm pop s_lockBuf3;
//		_asm pop s_lockBuf4;
//		_asm pop s_lockBuf5;
//		_asm pop s_lockBuf6;
//		_asm pop s_lockBuf7;
//		_asm pop s_lockBuf8;
//		_asm pop s_lockBuf9;
//		_asm pop s_lockBuf10;
//		s_lock.Unlock();
//		_asm push s_lockBuf10;
//		_asm push s_lockBuf9;
//		_asm push s_lockBuf8;
//		_asm push s_lockBuf7;
//		_asm push s_lockBuf6;
//		_asm push s_lockBuf5;
//		_asm push s_lockBuf4;
//		_asm push s_lockBuf3;
//		_asm push s_lockBuf2;
//		_asm push s_lockBuf1;
//
//		_asm mov eax,s_EAX;
//
//		_asm pop ebp;
//		_asm ret;
//#endif
//	}
//
//#ifdef ECHO_PLATFORM_WINDOWS
//	// key=name data=address;
//	static std::map<std::string,void*> s_symList;
//#ifdef _WIN64
//	BOOL __stdcall enumSymbolsCallback(PCSTR SymbolName,DWORD64 SymbolAddress,ULONG SymbolSize,PVOID UserContext)
//#else
//	BOOL __stdcall enumSymbolsCallback(PCSTR SymbolName, ULONG SymbolAddress, ULONG SymbolSize, PVOID UserContext)
//#endif
//	{
//		s_symList.insert(std::pair<std::string,void*>(SymbolName,(void*)SymbolAddress));
//		//EchoLogInfo("enumSymbolsCallback name=%s addr=%d",SymbolName,SymbolAddress);
//		return true;
//	}
//
//	static void* s_symFunction;
//#ifdef _WIN64
//	BOOL __stdcall enumFindFunctionCallback(PCSTR SymbolName,DWORD64 SymbolAddress,ULONG SymbolSize,PVOID UserContext)
//#else
//	BOOL __stdcall enumFindFunctionCallback(PCSTR SymbolName, ULONG SymbolAddress, ULONG SymbolSize, PVOID UserContext)
//#endif
//	{
//		PCSTR sName=(PCSTR)UserContext;
//		if(!strcmp(sName,SymbolName))
//		{
//			s_symFunction=(void*)SymbolAddress;
//			return false;
//		}
//		return true;
//	}
//#endif
//
//	const std::map<std::string,void*> CallTrackModule::GetFunctionDist(const std::vector<std::string>& vsFunction)
//	{
//		std::map<std::string,void*>	results;
//#ifdef ECHO_PLATFORM_WINDOWS
//		for(size_t i=0;i<vsFunction.size();i++)
//		{
//			results.insert(std::pair<std::string,void*>(vsFunction[i].c_str(),(void*)NULL));
//		}
//
//		std::map<std::string,void*>::iterator it=m_SymloadList.begin();
//
//		while(it!=m_SymloadList.end())
//		{
//			std::map<std::string,void*>::iterator itResult=results.find(it->first);
//			if(itResult!=results.end())
//			{
//				itResult->second=it->second;
//			}
//			++it;
//		}
//#endif
//		return results;
//	}
//
//	void* CallTrackModule::GetFunctionAddress(const char* sName)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		assert(m_BaseOfDll);
//		HANDLE hProcess=GetCurrentProcess();
//		s_symFunction=NULL;
//		SymEnumerateSymbols(hProcess,m_BaseOfDll,enumFindFunctionCallback,const_cast<char*>(sName));
//
//		return s_symFunction;
//#endif
//    
//    return NULL;
//	}
//
//	CallTrackModule::CallTrackModule(const char* pathName,const char* fileName)
//		:m_BaseOfDll(0),
//		m_BaseAddress(0),
//		m_sPDBPathName(pathName),
//		m_sFileName(fileName)
//	{
//	}
//
//	CallTrackModule::~CallTrackModule(void)
//	{
//	}
//
//	void* CallTrackModule::ToSymloadAddress(void* address)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		std::map<void*,void*>::iterator it=this->m_AddrToSymAddrList.find(address);
//		if(it!=m_AddrToSymAddrList.end())
//			return it->second;
//#endif
//		return NULL;
//	}
//	void CallTrackModule::OutputTrackerInfo(FILE* fp)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		LARGE_INTEGER time;
//		for(TrackInfo* ti=s_ti;ti<s_tiPtr;++ti)
//		{
//			std::map<void*,std::string>::iterator it=m_QueryList.find(ti->fun);
//
//			if(ti->low2<ti->low1)
//			{
//				Echo::i64 lowPart=ti->low2+0x100000000-ti->low1;
//
//				--ti->high2;
//				time.LowPart=(DWORD)lowPart;
//			}
//			else
//				time.LowPart=ti->low2-ti->low1;
//
//			time.HighPart=ti->high2-ti->high1;
//
//			LONGLONG us=(time.QuadPart*1000000)/s_CPUFrequency.QuadPart;//百万分之一秒
//
//			if(it!=m_QueryList.end())
//			{
//				for(int i=0;i<ti->level;i++)
//				{
//					if(fp)
//						fprintf(fp,"	");
//				}
//
//				if(fp)
//					fprintf(fp,"call %s us=%d\n",it->second.c_str(),us);
//			}
//			else
//			{
//				if(fp)
//					fprintf(fp,"call %p us=%d\n",ti->fun,us);
//			}
//		}
//
//		// 清0
//		s_tiPtr=s_ti;
//		s_tiStackPtr=s_tiStack;
//#endif
//	}
//
//	CallTrackManager& CallTrackManager::instance()
//	{
//		static CallTrackManager ctm;
//		return ctm;
//	}
//
//	CallTrackManager::CallTrackManager()
//		:m_base(NULL),
//		m_listener_event(NULL)
//	{
//	}
//
//	CallTrackManager::~CallTrackManager()
//	{
//	}
//
//#include "TrackMsgDef.h"
//#ifdef ECHO_PLATFORM_WINDOWS
//#include <event2/event.h>
//#include <event2/buffer.h>
//#include <event2/listener.h>
//#include <event2/bufferevent.h>
//
//	static void libEvent_Read(struct bufferevent* bev,void* ctx)
//	{
//		Echo::CallTrackManager::instance().OnMsg(bev,ctx);
//	}
//
//	static void libEvent_Event(struct bufferevent* bev,short events,void* ctx)
//	{
//		if(events&BEV_EVENT_ERROR)
//			return;
//		if(events&(BEV_EVENT_EOF|BEV_EVENT_ERROR))
//		{
//			bufferevent_free(bev);
//			CallTrackManager::instance().m_client_event=NULL;
//		}
//	}
//
//	struct bufferevent*& getClientEvent()
//	{
//		return CallTrackManager::instance().m_client_event;
//	}
//
//	static void libEvent_AcceptConnect(struct evconnlistener *listener, evutil_socket_t sock, struct sockaddr * addr, int socklen, void *)
//	{
//		struct event_base* base=evconnlistener_get_base(listener);
//		struct bufferevent*& bev=getClientEvent();
//		bev=bufferevent_socket_new(base,sock,BEV_OPT_CLOSE_ON_FREE);//创建新的SOCKET.
//		
//		bufferevent_setcb(bev,libEvent_Read,NULL,libEvent_Event,NULL);
//		bufferevent_enable(bev,EV_READ|EV_WRITE);
//
//		int error=EVUTIL_SOCKET_ERROR();
//		event_base_loopexit(base,NULL);
//	}
//	
//	static void libEvent_AcceptError(struct evconnlistener* listener, void* ctx)
//	{
//		struct event_base* base=evconnlistener_get_base(listener);
//		int error=EVUTIL_SOCKET_ERROR();
//		event_base_loopexit(base,NULL);
//	}
//
//	bool CallTrackManager::Initialize()
//	{
//#ifndef USECALLTRACK
//		return true;
//#endif
//
//#ifdef ECHO_PLATFORM_WINDOWS
//		// 初始化计时器
//		QueryPerformanceFrequency(&s_CPUFrequency);
//
//		// 寻找所有的模块
//		HANDLE hProc=GetCurrentProcess();
//		HMODULE hMod[1024];
//		DWORD cbNeeded;
//		char pathName[MAX_PATH];
//		char fileName[MAX_PATH];
//		if(EnumProcessModules(hProc,hMod,sizeof(hMod),&cbNeeded))
//		{
//			for(size_t i=0;i<cbNeeded/sizeof(HMODULE);i++)
//			{
//				if(GetModuleFileNameExA(hProc,hMod[i],pathName,MAX_PATH))
//				{
//					char* pos = strrchr(pathName,'\\');
//					if(pos==0)
//						pos=pathName;
//					strcpy(fileName,pos+1);
//					
//					pos = strstr(pathName,".");
//					if(pos)
//					{
//						memcpy(pos,".pdb",4);
//						pos[4]=0;
//						FILE* fp=fopen(pathName,"rb");
//						if(fp)
//						{
//							fclose(fp);
//							ma_Modules.push_back(new CallTrackModule(pathName,fileName));
//						}
//					}
//
//					//EchoLogInfo("enum module %d name=%s base addr=%d",i,pathName,hMod[i]);
//				}
//			}
//		}
//		CloseHandle(hProc);
//
//		// 扫描用户地址空间，记录所有的调用地址
//		unsigned int index=0;
//		unsigned int pageSize=4096;
//		int page=-1;
//		while(index<s_WindowUserAddressSize)
//		{
//			// 如果不是有效页，就跳过
//			if(int(index/pageSize)>page)
//			{
//				bool enableCode=false;
//				MEMORY_BASIC_INFORMATION info;
//				if(VirtualQuery((void*)index,&info,sizeof(MEMORY_BASIC_INFORMATION)))
//				{
//					if(info.State==MEM_COMMIT)
//					{
//						if(info.Protect==PAGE_EXECUTE||info.Protect==PAGE_EXECUTE_WRITECOPY||info.Protect==PAGE_EXECUTE_READ)
//						{
//							DWORD oldProtect;
//							if(VirtualProtect((void*)index,pageSize,PAGE_EXECUTE_READWRITE,&oldProtect))
//								enableCode=true;
//						}
//						else if(info.Protect==PAGE_EXECUTE_READWRITE)
//						{
//							enableCode=true;
//						}
//					}
//				}
//
//				page=int(index/pageSize);
//
//				if(!enableCode)	// 如果是非代码页，则跳到下一页。
//				{
//					index/=pageSize;
//					++index;
//					index*=pageSize;
//					continue;
//				}
//			}
//
//			// 记录调用
//			if(((Byte*)index)[0]==0xe8)
//			{
//				ma_e8Call.push_back(index);
//			}
//			else if(index+6<s_WindowUserAddressSize&&((Byte*)index)[0]==0xff&&((Byte*)index)[1]==0x15)
//			{
//				ma_ff15Call.push_back(index);
//			}
//
//			// 跳到下条指令
//			DWORD dwCodeSize=1;
//			if(CheckAddress((void*)(index+1)))
//			{
//				dwCodeSize=GetOpCodeSize((unsigned char*)index);
//				if(dwCodeSize==0xffffffff)
//					dwCodeSize=1;
//			}
//
//			//
//			index+=dwCodeSize;
//		}
//		//EchoLogInfo("e8 call count=%d ff15 call count=%d\n",ma_e8Call.size(),ma_ff15Call.size());
//
//		// 初始化服务器
//		m_base=event_base_new();
//		if(!m_base)
//			return false;
//
//		struct sockaddr_in address;
//		memset(&address,0,sizeof(address));
//		address.sin_family=AF_INET;
//		address.sin_addr.s_addr=htonl(0);
//		address.sin_port=htons(TRACKLISTENPORT);
//
//		m_listener_event=evconnlistener_new_bind(m_base,libEvent_AcceptConnect,NULL,LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,1,(struct sockaddr*)&address,sizeof(address));
//		if(!m_listener_event)
//		{
//			EchoLogWarning("libEvent bind error!\n");
//			return false;
//		}
//
//		evconnlistener_set_error_cb(m_listener_event,libEvent_AcceptError);
//#endif
//		return true;
//	}
//
//	void CallTrackManager::Tick()
//	{
//#ifndef USECALLTRACK
//		return ;
//#endif
//
//#if defined(ECHO_PLATFORM_WINDOWS) && !defined(_WIN64)
//		// 消息循环
//		event_base_loop(m_base,EVLOOP_NONBLOCK);
//		
//		if(m_client_event)//&&bufferevent_enable(m_client_event,EV_WRITE))
//		{
//			_asm RDTSC;
//			_asm mov s_high,edx;
//			_asm mov s_low,eax;
//			s_ti->high2=s_high;
//			s_ti->low2=s_low;
//
//			// 发送帧消息
//			struct evbuffer* output=bufferevent_get_output(m_client_event);
//
//			WORD msgId=RMSG_FRAMEINFO;
//			bufferevent_write(m_client_event,&msgId,2);
//
//			static DWORD dwFrameId=0;
//			++dwFrameId;
//			bufferevent_write(m_client_event,&dwFrameId,4);
//
//			WORD wItemCount=(WORD)s_tiIndex;
//			bufferevent_write(m_client_event,&wItemCount,2);
//
//			for(WORD i=0;i<wItemCount;++i)
//			{
//				unsigned int fun=NULL;
//				for(size_t j=0;j<ma_Modules.size();j++)
//				{
//					fun=(unsigned int)ma_Modules[j]->ToSymloadAddress(s_ti[i].fun);
//					if(fun)
//						break;
//				}
//				bufferevent_write(m_client_event,&fun,4);
//				++s_ti[i].level;
//				bufferevent_write(m_client_event,&s_ti[i].level,2);
//				bufferevent_write(m_client_event,&s_ti[i].high1,4);
//				bufferevent_write(m_client_event,&s_ti[i].high2,4);
//				bufferevent_write(m_client_event,&s_ti[i].low1,4);
//				bufferevent_write(m_client_event,&s_ti[i].low2,4);
//			}
//			
//			bufferevent_flush(m_client_event,EV_WRITE,BEV_FLUSH);
//		}
//
//		// 清除记录
//		s_tiPtr=s_ti;
//		s_tiStackPtr=s_tiStack;
//		s_tiIndex=0;
//
//		s_ti->fun=NULL;
//		s_ti->level=-1;
//		
//		_asm RDTSC;
//		_asm mov s_high,edx;
//		_asm mov s_low,eax;
//		s_ti->high1=s_high;
//		s_ti->low1=s_low;
//		++s_tiPtr;
//		++s_tiIndex;
//#endif
//	}
//
//	void CallTrackManager::Release()
//	{
//#ifndef USECALLTRACK
//		return;
//#endif
//
//#ifdef ECHO_PLATFORM_WINDOWS
//		evconnlistener_free(m_listener_event);
//		WSACleanup();
//#endif
//	}
//
//	std::vector<std::string> CallTrackManager::ToVectorString(const char* str)
//	{
//		std::vector<std::string> vsCall;
//		std::string callName;
//		while(*str)
//		{
//			if(*str==' ')
//			{
//				vsCall.push_back(callName);
//				callName.clear();
//			}
//			else
//				callName+=*str;
//			++str;
//		}
//		return vsCall;
//	}
//
//	// 加载
//	bool CallTrackModule::Load(const std::vector<unsigned int>& e8Call,const std::vector<unsigned int>& ff15Call)
//	{
//		if(m_SymloadList.size()==0)
//		{
//#ifdef ECHO_PLATFORM_WINDOWS
//			assert(!m_BaseOfDll);
//
//			HANDLE hProcess=GetCurrentProcess();
//			if(!SymInitialize(hProcess,0,false))
//				return false;
//
//			m_BaseOfDll=SymLoadModule(hProcess,NULL,m_sFileName.c_str(),NULL,0,0);
//			if(!m_BaseOfDll)
//			{
//				DWORD dwError=GetLastError();
//				return false;
//			}
//
//			m_BaseAddress=GetModuleHandleA(m_sFileName.c_str());
//			if(!m_BaseAddress)
//			{
//				DWORD dwError=GetLastError();
//				return false;
//			}
//
//			// 获取全部符号的名字
//			assert(m_BaseOfDll);
//
//			s_symList.clear();
//			SymEnumerateSymbols(hProcess,m_BaseOfDll,enumSymbolsCallback,NULL);
//			m_SymloadList=s_symList;
//
//			std::map<std::string,void*>::iterator it=m_SymloadList.begin();
//			while(it!=m_SymloadList.end())
//			{
//				m_QueryList.insert(std::pair<void*,std::string>(it->second,it->first));
//				//EchoLogInfo("query list add addr=%d name=%s",it->second,it->first.c_str());
//				++it;
//			}
//
//			// 记录本地函数的所有调入点.
//			for(size_t i=0;i<e8Call.size();i++)
//			{
//				Byte* address=(Byte*)e8Call[i];
//				unsigned int jmpAddress=(unsigned int)address+*(unsigned int*)(address+1)+s_e8InstructLength;
//			
//				if(CheckAddress((void*)jmpAddress))
//				{
//					if(((Byte*)jmpAddress)[0]==0xe9)//这条应该是个长跳转
//					{
//						unsigned int jumpTo=*(unsigned int*)(jmpAddress+1)+jmpAddress+s_jmpLength;
//						unsigned int queryPos=(jumpTo-(unsigned int)m_BaseAddress)+m_BaseOfDll;
//						std::map<void*,std::string>::iterator itQuery=m_QueryList.find((void*)queryPos);
//						if(itQuery!=m_QueryList.end())
//						{
//							CallPos cp;
//							cp.address=(unsigned int)address;
//							cp.jumpTo=jumpTo;
//							cp.queryPos=queryPos;
//							this->m_e8CallPos.insert(std::pair<std::string,CallPos>(itQuery->second.c_str(),cp));
//							m_AddrToSymAddrList.insert(std::pair<void*,void*>((void*)jumpTo,(void*)queryPos));
//							//EchoLogInfo("e8 call add %s",itQuery->second.c_str());
//						}
//					}
//					else if(((Byte*)jmpAddress)[0]==0xff&&((Byte*)jmpAddress)[1]==0x25)//后面4位是变量地址
//					{
//						EchoLogInfo("e8 call 3 ff25 address=%d",jmpAddress);
//					}
//					else// if(((Byte*)jmpAddress)[0]==0x55||((Byte*)jmpAddress)[0]==0x56)//0x55表示push ebp 表示一个函数开始执行了
//					{
//						unsigned int jumpTo=jmpAddress;
//						unsigned int queryPos=(jumpTo-(unsigned int)m_BaseAddress)+m_BaseOfDll;
//						std::map<void*,std::string>::iterator itQuery=m_QueryList.find((void*)queryPos);
//						if(itQuery!=m_QueryList.end())
//						{
//							CallPos cp;
//							cp.address=(unsigned int)address;
//							cp.jumpTo=(unsigned int)jumpTo;
//							cp.queryPos=queryPos;
//							this->m_e8CallPos.insert(std::pair<std::string,CallPos>(itQuery->second.c_str(),cp));
//							m_AddrToSymAddrList.insert(std::pair<void*,void*>((void*)jumpTo,(void*)queryPos));
//							//EchoLogInfo("e8 2 call add %s",itQuery->second.c_str());
//						}
//						//else
//						//	EchoLogInfo("address %d queryAddress=%d code bytes = 0x%x%x%x%x",address,queryPos,((Byte*)jmpAddress)[0],((Byte*)jmpAddress)[1],((Byte*)jmpAddress)[2],((Byte*)jmpAddress)[3]);
//					}
//				}
//			}
//			for(size_t i=0;i<ff15Call.size();i++)
//			{
//				Byte* address=(Byte*)ff15Call[i];
//				if(CheckAddress((void*)(address+2)))
//				{
//					unsigned int callAddress=*(unsigned int*)(address+2);//ff 15 address
//					if(CheckAddress((void*)callAddress))
//					{
//						unsigned int jmpAddress=*(unsigned int*)callAddress;
//						if(CheckAddress((void*)jmpAddress)&&((Byte*)jmpAddress)[0]==0xe9)//如果不是长跳转指令，跳过
//						{
//							unsigned int jumpTo=*(unsigned int*)(jmpAddress+1)+jmpAddress+s_jmpLength;
//							unsigned int queryPos=(jumpTo-(unsigned int)m_BaseAddress)+m_BaseOfDll;
//							std::map<void*,std::string>::iterator itQuery=m_QueryList.find((void*)queryPos);
//							if(itQuery!=m_QueryList.end())
//							{
//								CallPos cp;
//								cp.address=(unsigned int)address;
//								cp.jumpTo=jumpTo;
//								cp.queryPos=queryPos;
//								this->m_ff15CallPos.insert(std::pair<std::string,CallPos>(itQuery->second,cp));
//								m_AddrToSymAddrList.insert(std::pair<void*,void*>((void*)jumpTo,(void*)queryPos));
//							}
//						}
//					}
//				}
//
//			}
//		}
//#endif
//		return true;
//	}
//
//	// 卸载
//	void CallTrackModule::Unload()
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//			assert(m_BaseOfDll);
//
//			// 释放所有的追踪项
//			std::multimap<std::string,TrackRecord>::iterator it=m_callRecord.begin();
//			while(it!=m_callRecord.end())
//			{
//				void* dst=(void*)it->second.callAddress;
//				void* src=it->second.oldCode;
//				memcpy(dst,src,6);
//				VirtualFree((void*)it->second.allocAddress,20,MEM_RELEASE);
//				++it;
//			}
//			m_callRecord.clear();
//
//			//
//			HANDLE hProcess=GetCurrentProcess();
//			SymUnloadModule(hProcess,m_BaseOfDll);
//			SymCleanup(hProcess);
//			m_BaseOfDll=NULL;
//#endif
//	}
//
//	// 追踪
//	int CallTrackModule::Track(char* funName)
//	{
//		//EchoLogInfo("model e8Call Count=%d.",m_e8CallPos.size());
//		int count=0;
//		std::multimap<std::string,CallPos>::iterator it=m_e8CallPos.find(funName);
//		if(it==m_e8CallPos.end())
//		{
//			it=m_e8CallPos.begin();
//			int i=0;
//			while(it!=m_e8CallPos.end())
//			{
//				//EchoLogInfo("%d. %s!=%s",++i,it->first.c_str(),funName);
//				++it;
//			}
//		}
//		while(it!=m_e8CallPos.end())
//		{
//			//EchoLogInfo("find %s e8 call success! call address=%d",funName,it->second.address);
//			if(it->first!=funName)
//				break;
//
//			TrackCall(funName,it->second.address,it->second.jumpTo);
//
//			++count;
//			++it;
//		}
//		
//		it=m_ff15CallPos.find(funName);
//		while(it!=m_ff15CallPos.end())
//		{
//			//EchoLogInfo("find %s ff15 call success! call address=%d",funName,it->second.address);
//			if(it->first!=funName)
//				break;
//
//			TrackCall(funName,it->second.address,it->second.jumpTo);
//
//			++count;
//			++it;
//		}
//
//		return count;
//	}
//
//	// 取消追踪
//	int CallTrackModule::CancelTrack(char* funName)
//	{
//		int freeCount=0;
//		std::multimap<std::string,TrackRecord>::iterator it=m_callRecord.lower_bound(funName);
//		while(it!=m_callRecord.end())
//		{
//			if(!it->first.compare(funName))
//			{
//				void* dst=(void*)it->second.callAddress;
//				void* src=it->second.oldCode;
//				memcpy(dst,src,6);
//				VirtualFree((void*)it->second.allocAddress,20,MEM_RELEASE);
//				m_callRecord.erase(it++);
//				++freeCount;
//			}
//			else
//				break;
//		}
//		return freeCount;
//	}
//
//	bool CallTrackModule::CheckAddress(void* address)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		MEMORY_BASIC_INFORMATION info;
//		if(!VirtualQuery((void*)address,&info,sizeof(MEMORY_BASIC_INFORMATION)))
//		{
//			return false;
//		}
//
//		if(info.State!=MEM_COMMIT)
//		{
//			//EchoLogInfo("address %d page state=%d",(unsigned int)address,info.State);
//			return false;
//		}
//
//		return true;
//
//#endif
//		return true;
//	}
//
//	bool CallTrackModule::EnablePageReadWriteByte(void* address)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		// 改首字节
//		MEMORY_BASIC_INFORMATION info;
//		if(!VirtualQuery((void*)address,&info,sizeof(MEMORY_BASIC_INFORMATION)))
//			return false;
//
//		DWORD oldProtect;
//		if(info.Protect==PAGE_EXECUTE||info.Protect==PAGE_EXECUTE_READ||info.Protect==PAGE_EXECUTE_WRITECOPY)
//		{
//			if(!VirtualProtect(address,1,PAGE_EXECUTE_READWRITE,&oldProtect))
//				return false;
//		}
//		else if(info.Protect==PAGE_READONLY||info.Protect==PAGE_WRITECOPY)
//		{
//			if(!VirtualProtect(address,1,PAGE_READWRITE,&oldProtect))
//				return false;
//		}
//		else if(info.Protect==PAGE_READWRITE||info.Protect==PAGE_EXECUTE_READWRITE)
//			return true;
//		else
//		{
//			assert(0);
//			return false;
//		}
//
//		return true;
//#endif
//		return true;
//	}
//
//	bool CallTrackModule::EnablePageReadWrite(void* address,size_t size)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		// 改首字节
//		if(!EnablePageReadWriteByte(address))
//			return false;
//
//		if(size>1)
//		{
//			// 改尾字节
//			if(!EnablePageReadWriteByte(&(((Byte*)address)[size-1])))
//				return false;
//		}
//
//		return true;
//#endif
//	}
//
//	void CallTrackModule::TrackCall(const std::string& callName,unsigned int codeAddress,unsigned int jumpToAddress)
//	{
//		// 在记录中检查一下，如果已经有了，就跳过
//		if(*((Byte*)codeAddress)==0xe9)
//		{
//			EchoLogInfo("track call address %d, code=jmp 0xe9, continue!\n",codeAddress);
//			return;
//		}
//
//#ifdef ECHO_PLATFORM_WINDOWS
//		// 分配虚拟内存
//		unsigned char* codeBuffer=(unsigned char*)VirtualAlloc(NULL,20,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
//		if(codeBuffer)
//		{
//			// 记录
//			TrackRecord tr;
//			tr.allocAddress=(unsigned int)codeBuffer;
//			tr.callAddress=codeAddress;
//			memcpy(tr.oldCode,(void*)codeAddress,6);
//			m_callRecord.insert(std::pair<std::string,TrackRecord>(callName,tr));
//
//			// 修改调用为长转移到预分配代码段 jmp codeAddress 机器码 = ea 00 00 00 00
//			unsigned char* buffer=(unsigned char*)codeAddress;
//			unsigned int instructLength=0;
//			if(((Byte*)buffer)[0]==0xe8)
//			{
//				instructLength=s_e8InstructLength;
//			}
//			else if(((Byte*)buffer)[0]==0xff&&((Byte*)buffer)[1]==0x15)
//			{
//				EnablePageReadWrite(&(buffer[5]),1);
//				buffer[5]=0xcc;//int3
//				instructLength=es_ff15InstructLength;
//			}
//			else
//			{
//				assert(0);
//				return;
//			}
//			buffer[0]=0xe9;//调用代码改写成jmp
//			unsigned int srcAddress=codeAddress;
//			unsigned int dstAddress=unsigned int(codeBuffer);
//			unsigned int* jmpAddress=(unsigned int*)&(buffer[1]);
//			EnablePageReadWrite(jmpAddress,sizeof(unsigned int));
//			*jmpAddress=dstAddress-(srcAddress+s_jmpLength);
//
//			// 把目标函数地址作为参数入栈		push	function address	机器码 = 68 00 00 00 00
//			*codeBuffer=0x68;
//			unsigned int* address=(unsigned int*)++codeBuffer;
//			EnablePageReadWrite(address,sizeof(unsigned int));
//			*address=jumpToAddress;
//			codeBuffer+=4;
//
//			// 调用记录功能 call sheel function     机器码 = e8 00 00 00 00
//			*codeBuffer=0xe8;
//			address=(unsigned int*)++codeBuffer;
//			*address=(unsigned int)CallTracker_CallSheel-(unsigned int)codeBuffer-4;
//			codeBuffer+=4;
//
//			*codeBuffer=0x83;// 堆栈减12.
//			++codeBuffer;
//			*codeBuffer=0xC4;
//			++codeBuffer;
//			*codeBuffer=0x4;
//			++codeBuffer;
//
//			// 跳转到调用位置的下一条指令位置	jmp		call address		机器码 = ea 00 00 00 00
//			*codeBuffer=0xe9;
//			++codeBuffer;
//			srcAddress=unsigned int(codeBuffer);
//			dstAddress=codeAddress;
//			address=(unsigned int*)codeBuffer;
//			*address=dstAddress-srcAddress;
//			++*address;
//			if(instructLength==es_ff15InstructLength)
//				++*address;
//		}
//#endif
//	}
//
//	void CallTrackManager::OnMsg_GetModuleList(struct bufferevent* bev,void* ctx)//请求模块表
//	{
//		struct evbuffer* input=bufferevent_get_input(bev);
//		struct evbuffer* output=bufferevent_get_output(bev);
//
//		WORD msgId=RMSG_MODULELIST;
//		bufferevent_write(bev,&msgId,2);
//		bufferevent_write(bev,&s_CPUFrequency.HighPart,4);
//		bufferevent_write(bev,&s_CPUFrequency.LowPart,4);
//		WORD moduleCount=ma_Modules.size();
//		bufferevent_write(bev,&moduleCount,2);
//		for(size_t i=0;i<ma_Modules.size();i++)
//		{
//			// 是否加载成功
//			WORD loaded=ma_Modules[i]->GetAllSymload().size();
//			bufferevent_write(bev,&loaded,2);
//			// 模块符号文件名字
//			WORD nameLength=ma_Modules[i]->GetPDBPathName().length();
//			bufferevent_write(bev,&nameLength,2);
//			bufferevent_write(bev,ma_Modules[i]->GetPDBPathName().c_str(),nameLength);
//			// 模块文件名字
//			nameLength=ma_Modules[i]->GetModuleName().length();
//			bufferevent_write(bev,&nameLength,2);
//			bufferevent_write(bev,ma_Modules[i]->GetModuleName().c_str(),nameLength);
//		}
//		event_base_loop(CallTrackManager::instance().m_base,EVLOOP_ONCE);
//	}
//	void CallTrackManager::OnMsg_UnloadModule(struct bufferevent* bev,void* ctx)
//	{
//		//assert(0);
//	}
//	void CallTrackManager::OnMsg_LoadModule(struct bufferevent* bev,void* ctx)//请求加载模块
//	{
//		struct evbuffer* input=bufferevent_get_input(bev);
//		struct evbuffer* output=bufferevent_get_output(bev);
//		
//		WORD moduleNameLength;
//		evbuffer_remove(input,&moduleNameLength,2);
//		char* buffer=(char*)malloc(moduleNameLength+1);
//		evbuffer_remove(input,buffer,moduleNameLength);
//		buffer[moduleNameLength]=0;
//
//		for(size_t i=0;i<ma_Modules.size();i++)
//		{
//			if(!ma_Modules[i]->GetPDBPathName().compare(buffer))
//			{
//				ma_Modules[i]->Load(ma_e8Call,ma_ff15Call);//加载模块
//
//				//发送符号表
//				WORD msgId=RMSG_SYMLOADLIST;
//				bufferevent_write(bev,&msgId,2);
//				bufferevent_write(bev,&moduleNameLength,2);
//				bufferevent_write(bev,buffer,moduleNameLength);
//				const std::map<std::string,void*>& syms=Echo::CallTrackManager::instance().ma_Modules[i]->GetAllSymload();
//				std::map<std::string,void*>::const_iterator it=syms.begin();
//				while(it!=syms.end())
//				{
//					if(it->second!=NULL)
//					{
//						if(!strstr(it->first.c_str(),"<"))//过滤掉模板函数
//						{
//							bufferevent_write(bev,&it->second,4);//函数地址
//							WORD wLen=it->first.length();
//							bufferevent_write(bev,&wLen,2);//名字长度
//							bufferevent_write(bev,it->first.c_str(),(size_t)wLen);//名字
//						}
//					}
//					++it;
//				}
//				DWORD nullAddress=NULL;
//				bufferevent_write(bev,&nullAddress,4);//返回空表示结束了
//				event_base_loop(CallTrackManager::instance().m_base,EVLOOP_ONCE);
//
//				return;
//			}
//		}
//	}
//	void CallTrackManager::OnMsg_GetSymloadList(struct bufferevent* bev,void* ctx)
//	{
//	}
//	void CallTrackManager::OnMsg_Track(struct bufferevent* bev,void* ctx)
//	{
//		struct evbuffer* input=bufferevent_get_input(bev);
//		struct evbuffer* output=bufferevent_get_output(bev);
//
//		// 取模块名字
//		WORD modNameLength;
//		evbuffer_remove(input,&modNameLength,2);
//		char* bufModName=(char*)malloc(modNameLength+1);
//		evbuffer_remove(input,bufModName,modNameLength);
//		bufModName[modNameLength]=0;
//
//		// 取函数名字
//		WORD funNameLength;
//		evbuffer_remove(input,&funNameLength,2);
//		char* bufFunName=(char*)malloc(funNameLength+1);
//		evbuffer_remove(input,bufFunName,funNameLength);
//		bufFunName[funNameLength]=0;
//
//		//
//		for(size_t i=0;i<ma_Modules.size();i++)
//		{
//			if(ma_Modules[i]->GetModuleName()==bufModName)
//			{
//				int result=ma_Modules[i]->Track(bufFunName);
//
//				WORD msgId=RMSG_LISTENRESULT;
//				bufferevent_write(bev,&msgId,2);
//				bufferevent_write(bev,&funNameLength,2);
//				bufferevent_write(bev,bufFunName,funNameLength);
//				bufferevent_write(bev,&result,4);
//			}
//		}
//
//		free(bufModName);
//		free(bufFunName);
//	}
//
//	void CallTrackManager::OnMsg_CancelTrack(struct bufferevent* bev,void* ctx)
//	{
//		struct evbuffer* input=bufferevent_get_input(bev);
//		struct evbuffer* output=bufferevent_get_output(bev);
//
//		// 取模块名字
//		WORD modNameLength;
//		evbuffer_remove(input,&modNameLength,2);
//		char* bufModName=(char*)malloc(modNameLength+1);
//		evbuffer_remove(input,bufModName,modNameLength);
//		bufModName[modNameLength]=0;
//
//		// 取函数名字
//		WORD funNameLength;
//		evbuffer_remove(input,&funNameLength,2);
//		char* bufFunName=(char*)malloc(funNameLength+1);
//		evbuffer_remove(input,bufFunName,funNameLength);
//		bufFunName[funNameLength]=0;
//
//		//
//		for(size_t i=0;i<ma_Modules.size();i++)
//		{
//			if(ma_Modules[i]->GetModuleName()==bufModName)
//			{
//				ma_Modules[i]->CancelTrack(bufFunName);
//
//				WORD msgId=RMSG_UNDO_LISTENRESULT;
//				bufferevent_write(bev,&msgId,2);
//				bufferevent_write(bev,&funNameLength,2);
//				bufferevent_write(bev,bufFunName,funNameLength);
//			}
//		}
//
//		free(bufModName);
//		free(bufFunName);
//	}
//
//	void CallTrackManager::OnMsg(struct bufferevent* bev,void* ctx)	//处理消息
//	{
//		struct evbuffer* input=bufferevent_get_input(bev);
//		struct evbuffer* output=bufferevent_get_output(bev);
//
//		size_t len=evbuffer_get_length(input);
//		while(len>=sizeof(WORD))
//		{
//			WORD msgLength=0;
//			evbuffer_copyout(input,&msgLength,sizeof(WORD));
//			if(len>=msgLength)
//			{
//				evbuffer_remove(input,&msgLength,sizeof(WORD));
//				WORD msgId;
//				evbuffer_remove(input,&msgId,sizeof(WORD));
//				if(msgId==TMSG_GETMODULELIST)//获取模块表
//				{
//					OnMsg_GetModuleList(bev,ctx);
//				}
//				else if(msgId==TMSG_LOADMODULE)
//				{
//					OnMsg_LoadModule(bev,ctx);
//				}
//				else if(msgId==TMSG_UNLOADMODULE)
//				{
//					OnMsg_UnloadModule(bev,ctx);
//				}
//				else if(msgId==TMSG_GETSYMLOADLIST)//获取符号表
//				{
//					OnMsg_GetSymloadList(bev,ctx);
//				}
//				else if(msgId==TMSG_TRACKCALL)//追踪
//				{
//					OnMsg_Track(bev,ctx);
//				}
//				else if(msgId==TMSG_UNDO_TRACKCALL)//取消追踪
//				{
//					OnMsg_CancelTrack(bev,ctx);
//				}
//				else
//					assert(0);
//			}
//			else
//				break;
//
//			len=evbuffer_get_length(input);
//		}
//	}
//
//	bool CallTrackManager::CheckAddress(void* address)
//	{
//#ifdef ECHO_PLATFORM_WINDOWS
//		MEMORY_BASIC_INFORMATION info;
//		if(!VirtualQuery((void*)address,&info,sizeof(MEMORY_BASIC_INFORMATION)))
//			return false;
//
//		if(info.State!=MEM_COMMIT)
//			return false;
//
//		return true;
//
//#endif
//		return true;
//	}
//#endif
//}
