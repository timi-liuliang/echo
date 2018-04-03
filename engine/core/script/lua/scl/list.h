////////////////////////////////////////////////////////////////////////////////
//	list
//	2010.05.02 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/allocator.h"

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class list_node
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class list_node
{
public:
	T			elem;
	list_node*	next;
	list_node*	prev;
};

////////////////////////////////////////////////////////////////////////////////
//	class list
////////////////////////////////////////////////////////////////////////////////
template<typename T, typename Alloc = default_allocator<list_node<T> > >
class list
{
public:
	typedef list_node<T>		Node;
	class	iterator;

public:
	list();
	virtual ~list();
	list(const list& other) { *this = other; };
	//void SetAllocator(IAllocator<Node>& allocator) { m_pAllocator = &allocator; }

	//stl兼容接口
	void		push_back	(const T& elem) 					{ _insert(m_pTail, elem); }
	void		push_front	(const T& elem) 					{ _insert(NULL, elem); }
	void		pop_back	()									{ _remove(m_pTail); }
	void		pop_front	()									{ _remove(m_pHead); }
	void		remove		(const T& elem) 					{ _remove(_find(elem)); }
	iterator	insert		(iterator _where, const T& elem)	{ return iterator(_insert(_where.p, elem)); };
	iterator	erase		(iterator _where)					{ return iterator(_remove(_where.p)); }
	void		clear		();
	bool		empty		() const							{ return m_pHead == NULL; }
	int			size		() const							{ return m_size; }

	iterator	begin	() const	{ return iterator(m_pHead);	}
	iterator	end		() const	{ return iterator(NULL);	}
	iterator	rbegin	() const	{ return iterator(m_pTail);	}
	iterator	rend	() const	{ return iterator(NULL);	}

	T&			front	()			{ return m_pHead->elem; }
	const T&	front	() const	{ return m_pHead->elem; }
	T&			back	()			{ return m_pTail->elem; }
	const T&	back	() const	{ return m_pTail->elem; }

	//stl中没有的接口
	void		insert_before	(iterator _where, const T& elem);
	T&			push_back_fast	() { Node* p = _insert(m_pTail); return p->elem;		}
	iterator	find			(const T& elem) { return iterator(_find(elem));		}
	iterator	find_last		(const T& elem) { return iterator(_findLast(elem)); }
	bool		contains		(const T& elem) { return _find(elem) != NULL;		}

	//List层面的深拷贝，list_node中的list_node::elem内如果包含指针，不在该拷贝过程中。因此对于list_node::elem是浅拷贝
	list& operator=(const list& other);

public:
	class iterator
	{
	public:
		Node* p;
		
		iterator() { p = NULL; }
		iterator(Node* pNode) { p = pNode; }
		iterator&	operator++() { assert(NULL != p); p = p->next; return *this; }
		iterator&	operator++(int) { assert(NULL != p); p = p->next; return *this; }
		iterator&	operator--() { assert(NULL != p); p = p->prev; return *this; }
		iterator&	operator--(int) { assert(NULL != p); p = p->prev; return *this; }
		bool		operator==(const iterator& other) const { return this->p == other.p; }
		bool		operator!=(const iterator& other) const { return this->p != other.p; }
		T&			operator*() { assert(NULL != p); return p->elem; }
		const T&	operator*() const { assert(NULL != p); return p->elem; }
	};

private:
	Node*	_insert		(Node* pPosition, const T& elem);
	Node*	_insert		(Node* pPosition);
	Node*	_find		(const T& elem) const;
	Node*	_findLast	(const T& elem) const;
	Node*	_remove		(const T& elem);
	Node*	_remove		(Node* pElem);

private:
	Node*	m_pHead;
	Node*	m_pTail;
	int		m_size;

	//IAllocator<Node>* m_pAllocator;
};

template<typename T, typename Alloc>
list<T, Alloc>::list()
{
	m_pHead	= NULL;
	m_pTail	= NULL;
	//m_pAllocator = &DefaultAllocator< Node >::Singleton();
	m_size = 0;
}

template<typename T, typename Alloc>
list<T, Alloc>::~list()
{
	clear();
}

//template<typename T, typename Alloc>
//void list<T, Alloc>::Add(const T& elem)
//{
//	_insert(m_pTail, elem);
//}

//template<typename T, typename Alloc>
//void list<T, Alloc>::AddAtHead(const T& elem)
//{
//	_insert(NULL, elem);
//}

//template<typename T, typename Alloc>
//T& list<T, Alloc>::AddFast()
//{
//	Node* pNewNode = _insert(m_pTail);
//	return pNewNode->elem;
//}

template<typename T, typename Alloc>
void list<T, Alloc>::insert_before(iterator _where, const T& elem) 
{ 
	Node* pPosition = _where.p;
	if (NULL != pPosition->prev)
	{ 
		//pPosition不是头结点，直接插入即可
		_insert(pPosition->prev, elem); 
	} 
	else
	{
		//pPosition是头结点
		Node* pNewNode = Alloc::alloc();
		pNewNode->elem = elem;
		pNewNode->prev = NULL;
		pNewNode->next = pPosition;
		pPosition->prev = pNewNode;
		m_pHead = pNewNode;
	}
}

template<typename T, typename Alloc>
void list<T, Alloc>::clear()
{
	Node* removeNode = m_pHead;
	while (removeNode)
	{
		Node* next = removeNode->next;
		_remove(removeNode);
		removeNode = next;
	}
	m_pHead = NULL;
	m_pTail = NULL;
	m_size = 0;
}

//List层面的深拷贝，list_node中的list_node::elem内如果包含指针，不在该拷贝过程中。因此对于list_node::elem是浅拷贝
template<typename T, typename Alloc>
list<T, Alloc>& list<T, Alloc>::operator=(const list<T, Alloc>& other)
{
	clear();
	Node* p = other.m_pHead;
	while (p)
	{
		push_back(p->elem);
		p = p->next;
	}
	return *this;
}

//将elem元素插入到pPosition后面
template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_insert(Node* pPosition, const T& elem)
{
	Node* pNewNode = _insert(pPosition);
	pNewNode->elem = elem;
	return pNewNode;
}

//将elem元素插入到pPosition后面
template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_insert(Node* pPosition)
{
	Node* pNewNode = Alloc::alloc();
	if (NULL == pPosition)
	{
		pNewNode->next = m_pHead;
		pNewNode->prev = NULL;
		if (NULL != m_pHead)
		{
			m_pHead->prev = pNewNode;
		}
	}
	else
	{	
		pNewNode->prev = pPosition;
		pNewNode->next = pPosition->next;
		if (pPosition->next)
		{
			pPosition->next->prev = pNewNode;
		}
		pPosition->next = pNewNode;
	}
	if (NULL == pNewNode->prev)
		m_pHead = pNewNode;
	if (NULL == pNewNode->next)
		m_pTail = pNewNode;

	++m_size;
	return pNewNode;
}

template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_find(const T& elem) const
{
	Node* pFind = m_pHead;
	while(NULL != pFind)
	{
		if (pFind->elem == elem)
			return pFind;
		pFind = pFind->next;
	}
	return NULL;
}

template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_findLast(const T& elem) const
{
	Node* pFind = m_pTail;
	while(NULL != pFind)
	{
		if (pFind->elem == elem)
		{
			return pFind;
		}
		pFind = pFind->prev;
	}
	return NULL;
}

template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_remove(const T& elem)
{
	return _remove(_find(elem));
}

template<typename T, typename Alloc>
list_node<T>* list<T, Alloc>::_remove(Node* pElem)
{
	if (NULL == pElem)
		return NULL;
	Node* pNext = pElem->next;
	if (m_pHead == pElem)
	{
		m_pHead = pElem->next;
	}
	if (m_pTail == pElem)
	{
		m_pTail = pElem->prev;
	}
	if (NULL != pElem->prev)
	{
		pElem->prev->next = pElem->next;
	}
	if (NULL != pElem->next)
	{
		pElem->next->prev = pElem->prev;
	}
	Alloc::free(pElem);
	--m_size;
	return pNext;
}

} //namespace scl
