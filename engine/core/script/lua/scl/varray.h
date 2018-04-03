////////////////////////////////////////////////////////////////////////////////
//	array and varray
//	2010.04.24 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include <new>	//placement new
		
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <stdlib.h>	
#include <string.h>	
#endif

#include <algorithm>

namespace scl{

////////////////////////////////////////////////////////////////////////////////
//	class varray
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class varray
{
public:
	varray(): m_array(NULL), m_size(0), m_maxSize(0), m_option(0) {}
	~varray();

	//stl兼容接口。iterator暂时简化为T*
	typedef T			value_type;
	typedef int			size_type;
	void	push_back	(const T& elem);
	void	erase		(const int index);
	void	clear		();
	int		size		() const 				{ return m_size; };
	int		max_size	() const 				{ return m_maxSize; }
	T*		begin		()						{ return m_array; }
	T*		end			()						{ return &(m_array[m_size]); }
	const T* begin		() const				{ return m_array; }
	const T* end		() const				{ return &(m_array[m_size]); }
	T*		rbegin		()						{ return &(m_array[m_size - 1]); }
	T*		rend		()						{ return &(m_array[-1]); }
	void	resize		(const int size);		
	void	reserve		(const int capacity)	{ _grow_to(capacity); }
	bool	empty		() const				{ return m_size == 0; }
	int		capacity	() const		{ return m_maxSize; }
	void	assign		(const T* other, const int count, const int start = 0);
	void	reverse		() { for (int i = 0; i < m_size / 2; ++i) swapmem2(m_array[i], m_array[m_size - 1 - i]); }
	template <class _Pr>
	inline void sort(_Pr _Pred) { std::sort(m_array, m_array + m_size, _Pred);	}
	inline void sort()			{ std::sort(m_array, m_array + m_size);			}

	template <class _Pr>
	int binary_search	(const T& elem, _Pr compare);

	//template <class SearchT, class _Pr>
	//int binary_search_type(const SearchT& elem, _Pr customCompare);

	//stl中没有相应的功能接口
	//T&		push_back_fast		();		//当T中存在浅拷贝指针时，或者想要避免不必要的临时对象拷贝时，使用push_back_fast接口
	T&		push_back_fast		(const int count = 1);		//当T中存在浅拷贝指针时，或者想要避免不必要的临时对象拷贝时，使用push_back_fast接口
	void	erase_fast			(const int index);
	void	erase_element		(const T& elem);
	void	erase_element_fast	(const T& elem);	//当对数组内元素的顺序不关心的时候，使用RemoveFast接口提高效率
	int		find				(const T& elem) const;
	bool	contains			(const T& elem) const { return find(elem) != -1; }
	void	zero_memory			(const int start = 0, const int len = -1);
	void	disable_constructor	() { m_option |= OPTION_DISABLE_CONSTRUCTOR; }
	void	disable_destructor	() { m_option |= OPTION_DISABLE_DESTRUCTOR; }
	void	insert				(const int index, const T& elem);
	
	const T& operator[](int index) const { if (index >= m_maxSize || index < 0) { assert(0); throw 1; } return m_array[index]; }
	T&		 operator[](int index)		 { if (index >= m_maxSize || index < 0) { assert(0); throw 1; } return m_array[index]; }
	T*		 c_array			()			{ return m_array; }
	const T* c_array			() const	{ return m_array; }

	template <typename StreamerT>
	void map(StreamerT& s)
	{
		s << m_size;
		for (int i = 0; i < size(); ++i)
			s << m_array[i];
	}

	enum OPTION
	{
		OPTION_DISABLE_CONSTRUCTOR	= 0x01, //禁用构造函数
		OPTION_DISABLE_DESTRUCTOR	= 0x02, //禁用析构函数

	};

private:
	void _grow_to	(const int count);		//增长到指定大小，增长后，和_grow函数每次增长到STEP的某个倍数不同，_grow_to会使增长后的capacity严格为传入的参数count
	void _grow		(const int count = -1); //自动增长。如果不指定count，则增长后为增长前STEP倍。如果指定count，增长后肯定可以容纳count，但是capacity不一定是count
	bool _disable_constructor	() const { return (m_option & OPTION_DISABLE_CONSTRUCTOR) != 0; } //是否禁用构造函数
	bool _disable_destructor	() const { return (m_option & OPTION_DISABLE_DESTRUCTOR	) != 0; } //是否禁用析构函数

protected:
	T*		m_array;
	int 	m_size;
	int 	m_maxSize;
	char	m_option;
};

template<typename T>
inline void swapmem2(T& a, T& b)
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


////////////////////////////////////////////////////////////////////////////////
//	varray实现
////////////////////////////////////////////////////////////////////////////////
template<typename T> 
varray<T>::~varray()
{ 
	//当整个varray被析构的时候，不应该再考虑_disable_destructor选项，否则会造成大多数array内的element的析构函数没有被调用的情况
//	if (!_disable_destructor())
	{
		for (int i = 0; i < size(); ++i)
			m_array[i].~T(); //!!!注意，如果要使这里能调用到class T的析构函数，那么varray的声明处必须包含class T的完整定义，而不能因为成员变量只有一个T*指针，就仅仅使用一个T的前置声明
	}
	::free(m_array);
	m_array		= NULL;
	m_maxSize	= 0;
	m_size		= 0;
}

template<typename T> 
void varray<T>::push_back(const T& elem)
{
	if (m_size >= m_maxSize)
	{
		_grow();
	}

	if (!_disable_constructor())
		::new (&m_array[m_size]) T;

	m_array[m_size] = elem;
	++m_size;
}

//template<typename T> 
//T& varray<T>::push_back_fast()
//{
//	if (m_size >= m_maxSize)
//	{
//		_grow();
//	}
//	T* p = NULL;
//	if (!_disable_constructor())
//		p = ::new (&m_array[m_size]) T;
//	else
//		p = &m_array[m_size];
//
//	++m_size;
//	if (NULL == p)
//	{
//		assert(0);
//		throw 1;
//	}
//	return *p;
//}
//

template<typename T> 
T& varray<T>::push_back_fast(const int count)
{
	if (m_size + count > m_maxSize)
	{
		_grow(m_size + count);
	}
	T* p = &m_array[m_size];
	if (!_disable_constructor())
	{
		for (int i = 0; i < count; ++i)
			::new (&m_array[m_size + i]) T;
	}

	m_size += count;
	if (NULL == p)
	{
		assert(0);
		throw 1;
	}
	return *p;
}


template<typename T> 
void varray<T>::erase_element(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase(delIdx);
}

template<typename T> 
void varray<T>::erase(const int index)
{
	if (index < 0 || index >= m_size)
		return;

	//调用析构函数
	if (!_disable_destructor())
		m_array[index].~T();

	if (index < m_size - 1)
		memmove(&m_array[index], &m_array[index + 1], sizeof(T) * (m_size - index));

	--m_size;
}

template<typename T> 
void varray<T>::clear()
{
	//调用析构函数
	if (!_disable_destructor())
	{
		for (int i = 0; i < m_size; ++i)
			m_array[i].~T();
	}
	m_size = 0;
}

template<typename T>
void varray<T>::erase_element_fast(const T& elem)
{
	int delIdx = find(elem);
	if (-1 == delIdx)
		return;

	erase_fast(delIdx);
}

template<typename T>
void varray<T>::erase_fast(const int index)
{
	if (index < 0 || index >= m_size)
	{
		assert(0);
		return;
	}

	//调用析构函数
	if (!_disable_destructor())
		m_array[index].~T();

	//这里要做交换，否则当T中包含指针的时候，
	//会导致m_array[m_count - 1]的析构中delete同一个指针两次（m_array[index]中还有一个相同的指针）
	//同时，不能用一个新的T temp来做交换中间变量，因为这样会导致temp析构，
	//使得同样有同一个指针被两次delete的问题（temp内的指针和被temp保存过的elem内的指针）
	//2013.10.15 当T的类型为指针时，swapmem2在多线程下不安全，直接交换安全。上面提到的问题，交给T本身的拷贝构造函数来解决深拷贝潜拷贝的问提
	//m_array[index] = m_array[m_count - 1];
	if (index != m_size - 1)
	{
		//m_array[index] = m_array[m_size - 1];
		swapmem2(m_array[index], m_array[m_size - 1]);
	}
	m_size--;
}

template<typename T>
int varray<T>::find(const T& elem) const
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


template<typename T>
void varray<T>::assign(const T* other, const int count, const int start)
{
	if (count == 0)
		return;
	if (start + count > m_maxSize)
	{
		_grow(start + count);	//这里之所以调用_grow而不是_grow_to，是因为assign后如果push_back新元素，会导致再次_grow
	}

	//检查内存是否重叠
	assert(m_array != other);
	if (m_array < other)
		assert(m_array + start + count < other);
	if (other < m_array)
		assert(other + count < m_array);

	memcpy(m_array + start, other, count * sizeof(T));
	m_size = start + count;
}

template<typename T>
void varray<T>::_grow(const int count)
{
#ifdef SCL_DEBUG
	static int grow_c = 0;
	static int total_mem = 0;
	grow_c++;
#endif

	const int STEP = 8;
	//const int oldMaxSize = m_maxSize;

	//确定要增长到的大小
	int target = count;
	if (target == -1)
		target = m_maxSize + 1;
	assert(target > 0 && target > m_maxSize);
	if (m_maxSize == 0)
		m_maxSize = 16;
	while (m_maxSize < target)
		m_maxSize *= STEP;

	//分配新空间
	T* p = static_cast<T*>(::malloc(sizeof(T) * m_maxSize)); //new T[m_maxSize];
	if (NULL == p)
		return;
	if (_disable_constructor()) //禁用了构造函数，所以这里要对新创建的内存做简单的清空工作	//TODO 是否考虑增加选项禁用这一功能?
		memset((void*)(void*)(void*)p, 0, sizeof(T) * m_maxSize);
	
#ifdef SCL_DEBUG
	total_mem += m_maxSize * sizeof(T);
#endif

	//拷贝老数据到新空间
	if (m_array != NULL)
	{
		if (m_size > 0)
		{
			assert(m_size <= m_maxSize);
			memcpy((void*)p, (void*)m_array, sizeof(T) * m_size);
		}
		::free(m_array);
#ifdef SCL_DEBUG
		total_mem -= m_size * sizeof(T);
		printf("_grow = %d, count = %d, total mem = %dk, sizeof(T) = %d\n", m_maxSize, grow_c, total_mem / 1024, sizeof(T));
#endif
	}
	m_array = p;
}

//增长到指定大小，增长后，array的capacity严格为count
template <typename T>
void varray<T>::_grow_to(const int count)
{
	if (m_maxSize >= count)
		return;

	m_maxSize = count;

	//分配新空间
	T* p = static_cast<T*>(::malloc(sizeof(T) * m_maxSize)); //new T[m_maxSize];
	if (NULL == p)
		return;
	if (_disable_constructor()) //禁用了构造函数，所以这里要对新创建的内存做简单的清空工作	//TODO 是否考虑增加选项禁用这一功能?
		memset((void*)(void*)(void*)p, 0, sizeof(T) * m_maxSize);

	//拷贝老数据到新空间
	if (m_array != NULL)
	{
		if (m_size > 0)
			memcpy((void*)p, (void*)m_array, sizeof(T)* m_size);
		::free(m_array);
	}
	m_array = p;
}
template <typename T>
void varray<T>::resize(const int size)
{
	if (size > m_maxSize)
	{
		assert(false);
		return;
	}
	if (size > m_size)
	for (int i = m_size; i < size; ++i)
		push_back_fast();

	if (size < m_size)
	for (int i = m_size - 1; i >= size; --i)
		erase(i);
};

template <typename T>
void varray<T>::zero_memory(const int start, const int len) 
{ 
	int l = len >= 0 ? len : capacity();
	if (l > 0)
		memset(m_array + start, 0, sizeof(T) * l); 
}

template <typename T>
void varray<T>::insert(const int index, const T& elem)
{
	if (m_size >= m_maxSize)
		_grow();

	if (index > m_size || index < 0)
		return;
	if (index < m_size)
		memmove(&m_array[index + 1], &m_array[index], sizeof(T) * (m_size - index));
	m_array[index] = elem;
	++m_size;
}

template <typename T>
template <class _Pr>
int varray<T>::binary_search(const T& elem, _Pr compare)
{
	int		left	= 0;
	int		right	= size() - 1;
	int		mid		= -1;
	while (left <= right)
	{
		mid = (left + right) / 2; // mid = left + (right - left) / 2 ==>  mid = (left + right) / 2
		T& cur = m_array[mid];
		if (compare(cur, elem))
			right = mid - 1;
		else if (compare(elem, cur))
			left = mid + 1;
		else
			break;
	}
	return mid;
}

} //namespace scl
