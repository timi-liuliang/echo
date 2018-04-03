////////////////////////////////////////////////////////////////////////////////
//	stack
//	2010.05.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/array.h"
#include "scl/varray.h"

namespace scl{

////////////////////////////////////////////////////////////////////////////////
//	class stack
////////////////////////////////////////////////////////////////////////////////
template<typename T, int MAX_COUNT>
class stack
{
public:
	void	push	(const T& elem);
	T		pop		();
	int		size	() const { return m_stack.size(); }
	int		empty	() const { return size() == 0; }
	T		top		() const;

private:
	array<T, MAX_COUNT> m_stack;
};

////////////////////////////////////////////////////////////////////////////////
//	class HStack
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class vstack
{
public:
	
	void	push	(const T& elem);
	T		pop		();
	int		size	() const { return m_stack.size(); }
	int		empty	() const { return size() == 0; }
	T		top		() const;
	void	reserve	(const int maxSize);
	int		capacity() const { return m_stack.capacity(); }

private:
	varray<T> m_stack;
};


////////////////////////////////////////////////////////////////////////////////
//	class stack functions
////////////////////////////////////////////////////////////////////////////////
template<typename T, int MAX_COUNT>
void stack<T, MAX_COUNT>::push(const T& elem)
{
	m_stack.push_back(elem);
}

template<typename T, int MAX_COUNT>
T stack<T, MAX_COUNT>::pop()
{
	const T& elem = m_stack[m_stack.size() - 1];
	m_stack.erase_fast(m_stack.size() - 1);
	return elem;
}

template<typename T, int MAX_COUNT>
T stack<T, MAX_COUNT>::top() const
{
	return m_stack[m_stack.size() - 1];
}


////////////////////////////////////////////////////////////////////////////////
//	class pstack functions
////////////////////////////////////////////////////////////////////////////////
template<typename T>
void vstack<T>::reserve(const int maxSize)
{
	m_stack.reserve(maxSize);
}

template<typename T>
void vstack<T>::push(const T& elem)
{
	m_stack.push_back(elem);
}

template<typename T>
T vstack<T>::pop()
{
	const T& elem = m_stack[m_stack.size() - 1];
	m_stack.erase_fast(m_stack.size() - 1);
	return elem;
}

template<typename T>
T vstack<T>::top() const
{
	return m_stack[m_stack.size() - 1];
}


}// namespace scl

