////////////////////////////////////////////////////////////
//	buffer.h
//  caolei 2012.05.27
////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

namespace scl {

class buffer
{
public:
	buffer();
	buffer(void* p, const int max_length, const int start = 0, const int end = 0);
	virtual ~buffer();

	//初始化buffer
	void	alloc	(const int maxLength);				//分配一块长度为maxLength的内存,该内存在类析构的时候会被delete掉
	void	init	(void* pData, const int maxLength, const int start = 0, const int end = 0);	//将pData指向的内存作为数据区，最大使用长度为maxLength，该内存不会被类delete

	uint8*	data	()					{ return &(m_buffer[m_start]); }
	int		length	() const			{ return m_end - m_start; }			//end占一个位置
	int		read	(void* p, const int length);
	int		peek	(void* p, const int length);
	void	write	(const void* const p, const int length);
	int		free	() const			{ return m_maxLength - m_end;  }
	int		used	() const			{ return length(); }
	void	drop	(const int count) 	{ m_start += count; }
	int		max_length() const			{ return m_maxLength; }
	void	set_length(const int v)		{ m_end = m_start + v; }

private:
	uint8*			m_buffer;		//数据缓冲区
	int				m_start;		//起始位置，这个位置和数据的第一个byte重合
	int				m_end;			//结束位置，这个位置位于数据的最后一个byte后面
	int				m_maxLength;	//整个缓冲区所能容纳的数据长度，为m_buffer的长度减去m_end占用的1
	bool			m_autoDelete;	//是否需要在BinaryStreamer析构时delete[] m_buffer
};

} //namespace scl 

