#pragma once
#include <memory.h>
#include "scl/type.h"
#include "scl/assert.h"

namespace scl {

template <typename T>
class heap_ptr
{
public:
	heap_ptr();
	~heap_ptr()
	{
		if (NULL != m_heap)
		{
			delete[] m_heap;
			m_heap = NULL;
		}
	}
	void		reserve		(const int size)	
	{ 
		if (NULL != m_heap)
		{
			delete[] m_heap;
			m_heap = NULL;
		}
		m_heap = new T[size];
		m_capacity = size;
		m_size = 0;
	}
	void		zero_memory	()					{ memset(m_heap, 0, sizeof(m_heap[0]) * m_capacity); }
	void		add			(const T& a);		//添加一个元素，并在添加后调整为堆
	void		add_direct	(const T& a);		//仅在数组尾部添加一个元素，不进行调整，所以不保证添加后仍然是堆，一般和build一起用
	void		remove		(const T& a)		{ remove_at(_find(a)); }//删除一个元素，需要查找，时间复杂度为O(n)
	void		remove_at	(int index);		//删除指定索引处的元素 时间复杂度为O(lg n)
	bool		is_heap		();					//返回当前heap是否满足堆的定义
	void		build		();					//将当前heap内元素整理成堆
	void		clear		()					{ m_size = 0; }
	int			size		() const			{ return m_size; }
	int			count		() const			{ return m_size; }
	bool		empty		() const			{ return size() == 0; }
	T*			c_array		()					{ return m_heap; }
	const T*	c_array		() const			{ return m_heap; }
	const T&	operator[]	(const int i) const { assert(_is_valid_index(i)); return m_heap[i]; }
	T&			operator[]	(const int i)		{ assert(_is_valid_index(i)); return m_heap[i]; }

private:
	int			_find			(const T& a);
	bool		_is_valid_index	(const int i) const	{ return i >= 0 && i < m_size; } 
	int			_parent_index	(const int i) const	{ return (i - 1) / 2; }
	void		_swap			(const int isrc, const int idest);	//交换src和dest索引的元素值
	void		_sift_up		(const int i);	//将索引为i的值向上转移
	void		_sift_down		(const int i);	//将索引为i的值向下转移

private:
	T*	m_heap;
	int m_size;
	int m_capacity;
};

#define HEAP_IS_VALID_INDEX(i) (i >= 0 && i < m_size)

#define HEAP_SWAP(a, b) if (HEAP_IS_VALID_INDEX(a) && HEAP_IS_VALID_INDEX(b)) \
	{ T t = m_heap[a]; m_heap[a] = m_heap[b]; m_heap[b] = t; } else { assert(false); }
		

template <typename T>
heap_ptr<T>::heap_ptr() : m_heap(NULL)
{
	m_size = 0;
}

template <typename T>
void heap_ptr<T>::add_direct(const T& a)
{
	if (m_size < m_capacity)
	{
		m_heap[m_size++] = a;
	}
	else
		assert(false);
}

template <typename T>
void heap_ptr<T>::add(const T& a)
{
	if (m_size >= m_capacity)
	{
		assert(false);
		return;
	}
	m_heap[m_size++] = a;
	_sift_up(m_size - 1);
}

template <typename T>
void heap_ptr<T>::remove_at(int i)
{
	if (!HEAP_IS_VALID_INDEX(i))
		return;
	m_heap[i] = m_heap[--m_size];
	if (m_size <= 1)
		return;
	_sift_up(i);
	_sift_down(i);
}

template <typename T>
void heap_ptr<T>::build()
{
	if (m_size <= 1)
		return;
	for (int i = (m_size - 2) / 2; i >= 0 ; --i)
		_sift_down(i);
}

template <typename T>
void heap_ptr<T>::_swap(const int src, const int dest)
{
	if (!HEAP_IS_VALID_INDEX(src) || !HEAP_IS_VALID_INDEX(dest))
	{
		assert(false);
		return;
	}
	T t = m_heap[src];
	m_heap[src] = m_heap[dest];
	m_heap[dest] = t;
}

template <typename T>
void heap_ptr<T>::_sift_up(const int nodeIndex)
{
	int i = nodeIndex;
	int parent = _parent_index(i);
	while (i > 0 && m_heap[i] < m_heap[parent])
	{
		_swap(i, parent);
		i = parent;
		parent = _parent_index(i);
	}
}

template <typename T>
void heap_ptr<T>::_sift_down(const int nodeIndex)
{
	int i = nodeIndex;
	const int lastNodeWithChild = (m_size - 2) / 2; //最后一个包含子节点的node
	while (i <= lastNodeWithChild)
	{
		const int left	= 2 * i + 1;
		const int right	= 2 * i + 2;

		assert(left < m_size);
		if (right >= m_size) //如果只有左子节点
		{	
			if (m_heap[left] < m_heap[i])
			{
				_swap(i, left);
			}
			break;
		}

		if (m_heap[left] < m_heap[i] || m_heap[right] < m_heap[i])
		{	
			//当前节点比某个子节点大，所以不满足堆的条件，需要移动	
			const int min = m_heap[left] < m_heap[right] ? left : right;
			if (!HEAP_IS_VALID_INDEX(i) || !HEAP_IS_VALID_INDEX(min))
			{ 
				assert(false); 
			}
			HEAP_SWAP(i, min);
			i = min;
		}
		else //当前节点比两个子节点都小，已经是堆了
			break;
	}
}



template <typename T>
bool heap_ptr<T>::is_heap()
{
	for (int i = 0; i < (m_size - 1) / 2; ++i)
	{
		const int left = 2 * i + 1;
		const int right = 2 * i + 2;
		if (left < m_size && m_heap[left] < m_heap[i])
			return false;
		if (right < m_size && m_heap[left] < m_heap[i])
			return false;
	}
	return true;
}

template <typename T>
int	heap_ptr<T>::_find(const T& a)	
{
	for (int i = 0; i < m_size; ++i)
		if (m_heap[i] == a) return i;
	return -1;
}


} //namespace scl




