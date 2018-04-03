////////////////////////////////////////////////////////////////////////////////
//	array and parray
//	2010.04.24 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/math.h"

//#include <stdlib.h> //for qsort
#include <algorithm> //for std sort
#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <string.h>
#endif

#ifdef min
#undef min
#endif
#ifdef max 
#undef max 
#endif
namespace scl{

////////////////////////////////////////////////////////////////////////////////
//	class array
////////////////////////////////////////////////////////////////////////////////
template<typename T, int MAX_SIZE>
class array
{
public:

	array(): m_size(0) {}

	//stl兼容接口。iterator暂时简化为T*
	typedef T*			iterator;
	typedef const T*	const_iterator;
	typedef T			value_type;
	typedef int			size_type;
	void	zero_memory	()						{ memset(m_array, 0, sizeof(m_array)); }
	void	push_back	(const T& elem);
	void	erase		(const int index);
	void	clear		()						{ m_size = 0; }
	int		size		() const 				{ return m_size; };
	bool	full		() const				{ return m_size == MAX_SIZE; }
	int&	size_ref	()  					{ return m_size; };
	int		max_size	() const 				{ return MAX_SIZE; }
	T*		begin		()						{ return m_array; }
	T*		end			()						{ return &m_array[m_size]; }
	const T* begin		() const				{ return m_array; }
	const T* end		() const				{ return &(m_array[m_size]); }
	T*		rbegin		()						{ return &(m_array[m_size - 1]); }
	T*		rend		()						{ return &m_array[-1]; }
	void	resize		(const int size)		{ assert(size <= MAX_SIZE); m_size = size; };
	void	reserve		(const int capacity)	{ assert(0); } //静态数组不能reserve
	bool	empty		() const				{ return m_size == 0; }

	template<class _Pr>
	inline void	sort (_Pr _Pred)
	{ 
		std::sort(m_array, m_array + m_size, _Pred);
	}

	inline void	sort ()
	{ 
		std::sort(m_array, m_array + m_size);
	}

	template <class _Pr>
	inline int min(_Pr _Pred) const
	{
		int minIndex = -1;
		int minValue = scl::MAX_INT;
		for (int i = 0; i < size(); ++i)
		{
			int v = _Pred(m_array[i]);
			if (v >= minValue)
				continue;
			minValue = v;
			minIndex = i;
		}
		return minIndex;
	}

	template <class _Pr>
	inline int max(_Pr _Pred) const
	{
		int maxIndex = -1;
		int maxValue = scl::MIN_INT;
		for (int i = 0; i < size(); ++i)
		{
			int v = _Pred(m_array[i]);
			if (v <= maxValue)
				continue;
			maxValue = v;
			maxIndex = i;
		}
		return maxIndex;
	}

	//stl中没有相应的功能接口
	T&		push_back_fast		();
	void	erase_fast			(const int index);
	void	erase_element		(const T& elem);
	void	erase_element_fast	(const T& elem);
	int		find				(const T& elem) const;
	bool	contains			(const T& elem) const { return find(elem) != -1; }
	void	assign				(const T* other, const int count);
	int		capacity			() const { return MAX_SIZE; }

	const T& 	operator[]	(int index) const { if (index >= MAX_SIZE || index < 0) { assert(0);throw 1; } return m_array[index]; }
	T&		 	operator[]	(int index)		 { if (index >= MAX_SIZE || index < 0) { assert(0);throw 1; } return m_array[index]; }
	T*		 	c_array		()			{ return m_array; }
	const T* 	c_array		() const	{ return m_array; }
	bool		operator==	(const array<T, MAX_SIZE>& a) const;
	bool		operator!=	(const array<T, MAX_SIZE>& a) const;
	void	reverse		()
	{
		for (int i = 0; i < m_size / 2; ++i)
		{
			swapmem2(m_array[i], m_array[m_size - 1 - i]);
		}
	}

	template <typename StreamerT>
	void map(StreamerT& s)
	{
		s << m_size;
		if (m_size < 0 || m_size > MAX_SIZE)
			return;
		for (int i = 0; i < size(); ++i)
		{
			s << m_array[i];
		}
	}

protected:
	T	m_array[MAX_SIZE];
	int m_size;
};

template<typename T, int MAX_SIZE>
bool array<T, MAX_SIZE>::operator==(const array<T, MAX_SIZE>& other) const
{
	if (this->size() != other.size())
		return false;
	for (int i = 0; i < size(); ++i)
	{
		if (m_array[i] != other[i])
			return false;
	}
	return true;
}

template<typename T, int MAX_SIZE>
bool array<T, MAX_SIZE>::operator!=(const array<T, MAX_SIZE>& other) const
{
	return !(this->operator==(other));
}

template<typename T>
inline void swapmem(T& a, T& b)
{
	if (&a == &b)
		return;
	//if (a == b)  //operator==只提供了逻辑上的相等性判定
	//	return;
	int* pIntA = reinterpret_cast<int*>(&a);
	int* pIntB = reinterpret_cast<int*>(&b);
	for (uint i = 0; i < sizeof(T) / sizeof(int); ++i)
	{
		int t = *pIntA;
		*pIntA = *pIntB;
		*pIntB = t;
		++pIntA;
		++pIntB;
	}

	byte* pByteA = reinterpret_cast<byte*>(pIntA);
	byte* pByteB = reinterpret_cast<byte*>(pIntB);
	for (uint j = 0; j < sizeof(T) % sizeof(int); ++j)
	{
		byte t = *pByteA;
		*pByteA = *pByteB;
		*pByteB = t;
		++pByteA;
		++pByteB;
	}
}

template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::push_back(const T& elem)
{
	if (m_size >= MAX_SIZE)
	{
		assert(0);
		return;
	}
	m_array[m_size++] = elem;
}

template<typename T, int MAX_SIZE>
T& array<T, MAX_SIZE>::push_back_fast()
{
	if (m_size >= MAX_SIZE)
	{
		assert(0);
		return m_array[MAX_SIZE - 1];
	}
	m_size++;
	return m_array[m_size - 1];
}


template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::erase_element(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase(delIdx);
}

template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::erase(const int index)
{
	if (index < 0 || index >= m_size)
		return;

	for (int i = index; i < m_size - 1; ++i)
	{
		//m_array[i] = m_array[i + 1];
		swapmem(m_array[i], m_array[i + 1]);
	}
	m_size--;
}

template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::erase_element_fast(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase_fast(delIdx);
}

template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::erase_fast(const int index)
{
	if (index < 0 || index >= m_size)
	{
		assert(0);
		return;
	}

	//这里要做交换，否则当T中包含指针的时候，
	//会导致m_array[m_count - 1]的析构中delete同一个指针两次（m_array[index]中还有一个相同的指针）
	//同时，不能用一个新的T temp来做交换中间变量，因为这样会导致temp析构，
	//使得同样有同一个指针被两次delete的问题（temp内的指针和被temp保存过的elem内的指针）
	//2013.10.15 当T的类型为指针时，swapmem在多线程下不安全，直接交换安全。上面提到的问题，交给T本身的拷贝构造函数来解决深拷贝潜拷贝的问提
	//m_array[index] = m_array[m_count - 1];
	if (index != m_size - 1)
	{
		//m_array[index] = m_array[m_size - 1];
		swapmem(m_array[index], m_array[m_size - 1]);
	}
	m_size--;
}

template<typename T, int MAX_SIZE>
int array<T, MAX_SIZE>::find(const T& elem) const
{
	int foundIdx = -1;
	for (int i = 0; i < m_size; ++i)
	{
		if (m_array[i] == elem)
		{
			foundIdx = i;
			break;
		}
	}
	return foundIdx;
}

template<typename T, int MAX_SIZE>
void array<T, MAX_SIZE>::assign(const T* other, const int count)
{
	if (count == 0)
	{
		m_size = 0;
		return;
	}
	if (count > MAX_SIZE)
	{
		return;
	}

	//防止内存重合
	assert(m_array != other);
	if (m_array < other)
		assert(m_array + count < other);
	if (other < m_array)
		assert(other + count < m_array);

	memcpy(m_array, other, count * sizeof(T));
	m_size = count;
}

//template<typename T, int MAX_SIZE>
//void array<T, MAX_SIZE>::assign(const T* other, const int count)
//{
//	if (count > m_maxSize)
//	{
//		assert(false);
//		return;
//	}
//	memcpy(m_array, other, count * sizeof(T));
//	m_size = count;
//}

template <typename T>
class parray
{
public:
	parray() { m_array = NULL; m_size = 0; m_capacity = 0; }

	template <int N>
	parray(array<T, N>& a) { m_array = a.c_array(); m_size = a.size(); m_capacity = a.capacity(); }

	void		assign	(T* arr, const int size, const int capacity) { m_array = arr; m_size = size; m_capacity = capacity; }
	int			capacity() const	{ return m_capacity;	}
	int			size	() const	{ return m_size;		}
	T*			c_array	()			{ return m_array;		}
	const T*	c_array	() const	{ return m_array;		}

	const T&	operator[]	(int index) const	{ if (index >= m_capacity || index < 0) { assert(0);throw 1; } return m_array[index]; }
	T&		 	operator[]	(int index)			{ if (index >= m_capacity || index < 0) { assert(0);throw 1; } return m_array[index]; }

private:
	int			m_size;
	int			m_capacity;
	T*			m_array;
};

template <typename T>
class parray_const
{
public:
	parray_const() { m_array = NULL; m_size = 0; m_capacity = 0; }

	template <int N>
	parray_const(const array<T, N>& a) { m_array = a.c_array(); m_size = a.size(); m_capacity = a.capacity(); }

	void		assign	(T* arr, const int size, const int capacity) { m_array = arr; m_size = size; m_capacity = capacity; }
	int			capacity() const	{ return m_capacity;	}
	int			size	() const	{ return m_size;		}
	const T*	c_array	() const	{ return m_array;		}

	const T&	operator[]	(int index) const	{ if (index >= m_capacity || index < 0) { assert(0);throw 1; } return m_array[index]; }

private:
	int			m_size;
	int			m_capacity;
	const T*	m_array;
};



} //namespace scl

