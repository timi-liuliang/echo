#pragma once

#include "scl/varray.h"

namespace scl {

template <typename T>
class page_array
{
public:
	page_array();
	~page_array();

	void			reserve			(const int max_page, const int page_size);
	void			push_back		(const T& t);
	T&				push_back_fast	();
	int				size			() const { return m_size; }
	int				capacity		() const { return m_page_size * m_page_capacity; }

	const T&		operator[]		(int index) const;
	T&				operator[]		(int index);

private:
	varray<T>&		_page			(const int index);

private:
	varray<T>*		m_pages;
	int				m_page_size;
	int				m_page_capacity;
	int				m_size;
};

template <typename T>
page_array<T>::page_array() :
	m_pages			(NULL),
	m_page_size		(0),
	m_page_capacity	(0),
	m_size			(0)
{
}

template <typename T>
page_array<T>::~page_array()
{
	if (NULL != m_pages)
	{
		delete[] m_pages;
		m_pages = NULL;
	}
}

template <typename T>
void page_array<T>::reserve(const int max_page, const int page_size)
{
	if (NULL != m_pages)
		return;
	m_pages			= new varray<T>[max_page];
	m_page_capacity = max_page;
	m_page_size		= page_size;
}


template <typename T>
void page_array<T>::push_back(const T& t)
{
	varray<T>& page = _page(m_size);
	++m_size;
	page.push_back(t);
}

template <typename T>
T& page_array<T>::push_back_fast()
{
	varray<T>& page = _page(m_size);
	++m_size;
	return page.push_back_fast();
}

template <typename T>
varray<T>& page_array<T>::_page(const int index)
{
	const int page_index = index / m_page_size;
	if (page_index >= m_page_capacity)
	{
		assertf(false, "pages are full");
		throw 1;
	}
	varray<T>& page = m_pages[page_index]; 
	if (page.capacity() == 0)
		page.reserve(m_page_size);
	return page;
}

template <typename T>
const T& page_array<T>::operator[](int index) const
{
	return const_cast<page_array<T>*>(this)->operator[](index);
}

template <typename T>
T& page_array<T>::operator[](int index)
{
	varray<T>& page = _page(index);
	int inner_index = index % m_page_size;
	return page[inner_index];
}

} // namespace scl

