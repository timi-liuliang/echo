////////////////////////////////////////////////////////////////////////////////
//	allocator
//	2010.05.02 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/alloc_pool.h"

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	DefaultAllocator
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class default_allocator
{
public:
	static T*		alloc()					{ return ::new T(); }
	static  void	free(T* const pElem)	{ ::delete pElem;	}
};

////////////////////////////////////////////////////////////////////////////////
//	single_allocator
//	仅用于单个对象的分配
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class single_allocator
{
public:
	//static single_allocator<T>& Singleton() { static single_allocator<T> s_memory; return s_memory; }
	static void				init		(int maxSize)		{ m_memory.init(maxSize); }
	static void				release		()					{ m_memory.release(); }
	static bool				is_full		()					{ return m_memory.is_full(); }
	static bool				has_init	()					{ return m_memory.has_init(); }
	static int				free_size	()					{ return m_memory.free_size(); }

	static T*				alloc		()					{ return m_memory.alloc(); }
	static void				free		(T* const pElem)	{ m_memory.free(pElem); }

private:
	static alloc_pool<T>	m_memory;
};

template<typename T>
alloc_pool<T> single_allocator<T>::m_memory;


////////////////////////////////////////////////////////////////////////////////
//	grow_allocator
//	会增长的allocator
//	init设定一个增长的最大size数，即最大可能分配的class T的object的数量	
//	grow_allocator会将这个数量除以MAX_PAGE，作为每个pool的大小。
//	只有当所有当前已创建pool都用光的时候，才会new一个新的pool
////////////////////////////////////////////////////////////////////////////////
const int MAX_PAGE = 16;

template<typename T>
class grow_pool
{
public:
	grow_pool() : m_page_count(0), m_current(0), m_page_size(0) {}
	void init(uint maxsize)		
	{ 
		m_page_size = maxsize / MAX_PAGE + 1;
		for (int i = 0; i < MAX_PAGE; ++i)
		{
			m_pages[i] = NULL;
		}
	}

	void release()					
	{ 
		for (int i = 0; i < m_page_count; ++i)
		{
			assert(m_pages[i]);
			m_pages[i]->release();
			delete m_pages[i];
			m_pages[i] = NULL;
		}
	}

	bool is_full()					
	{	
		for (int i = 0; i < m_page_count; ++i)
		{
			assert(m_pages[i]);
			if (!m_pages[i]->is_full())
				return false;
		}
		return true;
	}

	bool has_init()					
	{
		return m_page_size > 0;
	}

	int free_size()					
	{
		int total = 0;
		for (int i = 0; i < m_page_count; ++i)
		{
			assert(m_pages[i]);
			total += m_pages[i]->free_size();
		}
		return total;
	}

	T* alloc()					
	{
		alloc_pool<T>* page = m_pages[m_current];
		if (NULL == page)
		{
			assert(m_page_count < MAX_PAGE);
			page = new alloc_pool<T>;
			page->init(m_page_size);
			m_pages[m_current] = page;
			++m_page_count;
			return page->alloc();
		}
		else
		{
			if (page->is_full()) 
			{
				//从前面开始找所有分配器
				for (int i = 0; i < m_page_count; ++i)
				{
					assert(m_pages[i]);
					if (!m_pages[i]->is_full())
					{
						m_current = i;
						return m_pages[i]->alloc();
					}
				}

				//没找到能用的，创建一个新的
				assert(m_page_count < MAX_PAGE);
				page = new alloc_pool<T>;
				page->init(m_page_size);
				m_pages[m_page_count] = page;
				m_current = m_page_count;
				++m_page_count;
				return page->alloc();
			}
			else
			{
				return page->alloc();
			}
		}
		return NULL;
	}

	void free(T* const pElem)	
	{
		for (int i = 0; i < m_page_count; ++i)
		{
			assert(m_pages[i]);	
			if (!m_pages[i]->in_pool(pElem))
				continue;
			m_pages[i]->free(pElem);
			break;
		}
	}

	int page_size	() const { return m_page_size; }
	int page_count	() const { return m_page_count; }
	int total_memory() const { return m_page_count * m_page_size; }

private:
	alloc_pool<T>*		m_pages[MAX_PAGE];	//实际的内存页,每页是一个内存池
	int					m_page_count;		//当前已用页数
	int					m_current;			//当前正在使用的页
	int					m_page_size;		//每个页的大小
};

////////////////////////////////////////////////////////////////////////////////
//	grow_allocator
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class grow_allocator
{
public:
	static void				init		(int maxSize)		{ m_memory.init(maxSize); }
	static void				release		()					{ m_memory.release(); }
	static bool				is_full		()					{ return m_memory.is_full(); }
	static bool				has_init	()					{ return m_memory.has_init(); }
	static int				free_size	()					{ return m_memory.free_size(); }
	static int				page_size	()					{ return m_memory.page_size(); }
	static int				page_count	()					{ return m_memory.page_count(); }
	static int				total_memory()					{ return m_memory.total_memory(); }

	static T*				alloc		()					{ return m_memory.alloc(); }
	static void				free		(T* const pElem)	{ m_memory.free(pElem); }

private:
	static grow_pool<T>	m_memory;
};

template<typename T>
grow_pool<T> grow_allocator<T>::m_memory;

} //namespace scl

