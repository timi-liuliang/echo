////////////////////////////////////////////////////////////////////////////////
//	pool
//	2010.11.17 caolei
//	note:
//		The different between a pool and a single_allocator is : 
//		whether called the constructor and destructor of class T.
//		a pool won't call the constructor and destructor,
//		but a single_allocator will always call the constructor destructor.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/stack.h"

namespace scl {

template<typename T>
class pool
{
public:
	pool();
	virtual ~pool();

	void	init			(int maxSize);

	T*		alloc			();
	void	free			(T* const pElem);
	int		count			() { return m_maxSize; };
	bool	is_full			(){ return m_allocCount + m_free.size() == 0; }
	bool	has_init		() const { return m_hasInit; }
	T&		operator[]		(const int index) { return m_pool[index]; }

	//以下两个方法需要T有clear成员函数
	T*		alloc_clear		() { T* p = this->alloc(); p->clear(); }	
	void	free_clear		(T* const pElem) { pElem->clear(); this->free(pElem); }	

private:
	varray<T>	m_pool;
	vstack<int>	m_free;
	int			m_allocCount;
	int			m_maxSize;
	bool		m_hasInit;
};

template<typename T>
pool<T>::pool()
{
	m_allocCount = 0;
	m_hasInit = false;
}

template<typename T>
pool<T>::~pool()
{
	m_allocCount = 0;
	m_hasInit = false;
}

template<typename T>
void pool<T>::init(const int maxSize)
{
	if (m_hasInit)
	{
		assert(0);
		return;
	}
	m_pool.reserve(maxSize);
	m_pool.resize(maxSize);
	m_free.reserve(maxSize);
	m_allocCount = maxSize; 
	m_maxSize	= maxSize;
	m_hasInit = true;
}

template<typename T>
T* pool<T>::alloc()
{
	if (!m_hasInit)
	{
		assert(0);
		return NULL;
	}
	if (m_allocCount > 0)
	{
		--m_allocCount;
		T* p = &m_pool[m_allocCount];
		return p;
	}
	else if (m_free.size() > 0)
	{
		int index = m_free.pop();
 		T* p = &m_pool[index];
 		return p;
	}
	else
	{
		//throw(1);	//TODO 定义对应异常
		assert(0);
		return NULL;
	}
}

template<typename T>
void pool<T>::free(T* const pElem)
{
	if (!m_hasInit)
	{
		assert(0);
		return;
	}
	if (NULL != pElem)
	{
		int index = pElem - &(m_pool[0]);
		m_free.push(index);
	}
}

} //namespace scl
