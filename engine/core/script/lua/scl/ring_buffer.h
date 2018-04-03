////////////////////////////////////////////////////////////
//	ring_buffer
//  caolei 2010.05.27
////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

namespace scl {

class ring_buffer
{
public:
	ring_buffer();
	virtual ~ring_buffer();

	//alloc is not thread safe, don't call this in any thread functions
	void	alloc	(int size);

	int		write	(const void* pBuffer, int length);
	int		read	(void* pOut, int length);
	int		peek	(void* pOut, int length);
	void	drop	(int length);

	//set the head = 0 and tail = 0
	//thread safe : not thread safe.
	void	clear	();
	void	clear_safe(); //thread safe clear (only safe as drop and read)

	//whether the buffer body crossed the end. 
	//thread safe : if there is only one reader and one writer, the function is thread safe.
	bool	is_wrapped() volatile;		

	//whether the free space crossed the end. //notice, if head == 0 or tail == 0, this function returns false
	//thread safe : yes.
	bool	is_free_wrapped() volatile;

	//thread safe : if there is only one reader and one writer, the function is thread safe.
	//warning, call this function once, then cache the value, don't call this function many times.
	//warning, please use "volatile int x = used()" to makesure the compiler will not optimalize used() to a const;
	int		used() volatile;

	//thread safe : if there is only one reader and one writer, the function is thread safe.
	//warning, call this function once, then cache the value, don't call this function many times.
	int		free() volatile;

	//thread unsafe, use function head() or tail() only when you are familar with the projects thread model.
	int 	head() volatile { return m_head; }
	int 	tail() volatile { return m_tail; }
	int		max_size() const { return m_maxSize; }

	//just for test
	byte* get_buffer() { return m_buffer; }

protected:
	byte*			m_buffer;
	int				m_head;
	int				m_tail;
	int				m_maxSize;

	////_init is not thread safe, don't call this in any thread functions
	//void	_init(const int size);	

	void	_moveHead(const int length);
	void	_moveTail(const int length);

	//static functions are for thread safe usage
	static bool	static_isWrapped	(const int head, const int tail) { return head > tail; }
	static bool	static_isFreeWrapped(const int head, const int tail);
	static int	static_free			(const int head, const int tail, const int maxSize);
	static int	static_used			(const int head, const int tail, const int maxSize);
};


} //namespace scl
