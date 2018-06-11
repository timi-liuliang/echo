#pragma once

#include <map>
#include "engine/core/Base/EchoDef.h"
#include "engine/core/log/LogManager.h"

namespace Echo
{
	template <typename T>
	class HashNode
	{
	public:
		HashNode( )
		: m_key( 0 )
		, m_depth( 0 )
		, m_ptr( NULL )
		, m_next( NULL )
		{
		
		}

		~HashNode( )
		{
			m_key = 0;
		}

	public:
		int m_key;
		int m_depth;
		T*	m_ptr;
		HashNode<T>* m_next;
	};

	template<typename T>
	class PtrMonitor
	{
	#define CalcHash( x ) (x) % m_maxhash

	public:
		PtrMonitor( )
		{
			if ( !m_ptrs )
			{
				m_ptrs = EchoAlloc(HashNode<T>*,m_maxhash);

				memset( m_ptrs, 0, sizeof(HashNode<T>*) * m_maxhash );
			}

			static unsigned int identifier = 1;	
			m_identifier = (identifier % 10) != 0 ? identifier : ++identifier;
			insertPtr( identifier );

			++m_hashcount;
			++identifier;
		}

		virtual ~PtrMonitor( )
		{
			deletePtr( m_identifier );
			--m_hashcount;
			if ( m_hashcount <= 0 )
			{
				EchoSafeFree(m_ptrs);
				m_maxhash = 10;
			}
		}

	public:
		static T* getPtr(const int AiKey, const char* filename, int line)
		{
			if ( !AiKey || !m_ptrs )
			{
				return NULL;
			}

			unsigned int liHashCode = CalcHash( AiKey );
			HashNode<T>* pNode = m_ptrs[liHashCode];

			while ( pNode != NULL )
			{
				if ( pNode->m_key == AiKey )
				{
					return pNode->m_ptr;
				}
				else
				{
					pNode = pNode->m_next;
				}
			}

			EchoLogError("PtrMonitor getPtr[%d],[file:%s][line:%d] failed", AiKey, filename, line);

			return NULL;
		}

		// 获取唯一标识符
		int getIdentifier() const { return m_identifier; }

		// 是否可用
		bool isValid(const int AiKey) const
		{ 
			if (!AiKey || !m_ptrs)
			{
				return NULL;
			}

			unsigned int liHashCode = CalcHash(AiKey);
			HashNode<T>* pNode = m_ptrs[liHashCode];

			while (pNode != NULL)
			{
				if (pNode->m_key == AiKey)
				{
					return true;
				}
				else
				{
					pNode = pNode->m_next;
				}
			}

			return false;
		}

	private:
		void insertPtr( const int AiKey )
		{
			if ( !isValid( AiKey))
			{
				unsigned int iHashKey = CalcHash( AiKey );
				HashNode<T>* pHead = m_ptrs[iHashKey];
				if ( pHead && pHead->m_depth > 2 )
				{
					resetPtr();
					insertPtr( AiKey );
					return;
				}
				m_ptrs[iHashKey] = EchoNew(HashNode<T>);
				m_ptrs[iHashKey]->m_key = AiKey;	
				m_ptrs[iHashKey]->m_ptr = static_cast<T*>(this);
				m_ptrs[iHashKey]->m_next = pHead;
				if ( pHead )
				{
					m_ptrs[iHashKey]->m_depth = pHead->m_depth + 1;
				}
			}
		}
	
		bool deletePtr( const int AiKey )
		{
			unsigned int iHashCode = CalcHash( AiKey );
			HashNode<T>* pNode = m_ptrs[iHashCode];
			HashNode<T>* pPreNode = NULL;
			if ( pNode == NULL )
			{
				return false;
			}

			while ( pNode )
			{
				if ( pNode->m_key == AiKey )
				{
					if ( pPreNode )
					{
						pPreNode->m_next = pNode->m_next;
						EchoSafeDelete(pNode, HashNode<T>);
					}
					else
					{
						m_ptrs[iHashCode] = pNode->m_next;
						EchoSafeDelete(pNode, HashNode<T>);
					}

					return true;
				}
				else
				{
					pPreNode = pNode;
					pNode = pNode->m_next;
				}
			}

			return false;
		}

		// 重新分配指针数组上限，并将原来的数据插入新的数组中
		void resetPtr( )
		{
			// 当某一个槽的数量超过3个时重新分配大小（每次*2）
			HashNode<T>** pTemp = m_ptrs;
			m_maxhash *= 2;
			m_ptrs = EchoAlloc( HashNode<T>*, m_maxhash);
			memset( m_ptrs, 0, sizeof(HashNode<T>*)*m_maxhash );

			Echo::ui32 iSize = m_maxhash >> 1;
			for ( Echo::ui32 i = 0; i < iSize; ++i )
			{
				HashNode<T>* pHead = pTemp[i]; 
				while ( pHead )
				{
					HashNode<T>* pNext = pHead->m_next;

					reinsertPtr( pHead->m_key, pHead );
					pHead = pNext;
				}
			}

			EchoSafeFree( pTemp);
		}

		void reinsertPtr( const int AiKey, HashNode<T>* ApPtr )
		{
			if ( !isValid( AiKey))
			{
				unsigned int iHashKey = CalcHash( AiKey );
				HashNode<T>* pHead = m_ptrs[iHashKey];

				m_ptrs[iHashKey] = ApPtr;
				m_ptrs[iHashKey]->m_next = pHead;
				if ( pHead )
				{
					m_ptrs[iHashKey]->m_depth = pHead->m_depth + 1;
				}
				else
				{
					m_ptrs[iHashKey]->m_depth = 0;
				}
			}
		}

	private:
		int m_identifier;	// 标示符

	public:
		static HashNode<T>** m_ptrs;		// 所有指针

	protected:
		static int m_hashcount;				// 当前指针数量
		static unsigned int m_maxhash;		// 当前Hash最大值
	};
}

// ID转指针
#define IdToPtr( PtrType, id) (PtrType::getPtr( id, __FILE__, __LINE__))
