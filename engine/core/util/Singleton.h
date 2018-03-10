#ifndef __ECHO_SINGLETON_H__
#define __ECHO_SINGLETON_H__

#include "engine/core/Util/AssertX.h"

//------------------------------------------------------------------------
#define __DeclareSingleton(type) \
public: \
    static type * m_pSingleton; \
    static type * instance() { EchoAssert(NULL != m_pSingleton); return m_pSingleton; }; \
    static bool hasInstance() { return NULL != m_pSingleton; }; \
	static void replaceInstance(type* instance) { EchoAssert(instance); m_pSingleton = instance; }\
private:

#define __ImplementSingleton(type) \
    type * type::m_pSingleton = NULL;

#define __ConstructSingleton \
    EchoAssert(NULL == m_pSingleton); m_pSingleton = this;

#define __DestructSingleton \
	EchoAssert(m_pSingleton); m_pSingleton = NULL;
//------------------------------------------------------------------------------

namespace Echo
{
	template<class T>
	class Singleton
	{
	public:
		Singleton()
		{
			EchoAssert(!ms_pSingleton);
			ms_pSingleton = static_cast<T*>(this);
		}

		~Singleton()
		{
			EchoAssert(ms_pSingleton);
			ms_pSingleton = NULL;
		}

	private:
		/** \brief Explicit private copy constructor. This is a forbidden operation.*/
		Singleton(const Singleton<T>&);

		/** \brief Private operator= . This is a forbidden operation. */
		Singleton& operator = (const Singleton<T>&);

	public:
		static bool HasInstance()
		{
			return ms_pSingleton != nullptr;
		}

		static T* Instance()
		{
			EchoAssert(ms_pSingleton);
			return ms_pSingleton;
		}

	protected:
		static T* ms_pSingleton;
	};

	template<class T>
	T* Singleton<T>::ms_pSingleton = NULL;
}

#endif