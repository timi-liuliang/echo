#pragma once

#include "scl/type.h"

namespace scl {

class file_mapping
{
public:
	file_mapping();
	~file_mapping();

	bool		open			(const char* const filename);
	void*		map				();
	void		close			();
	int			size			() const { return m_size; }
	void*		buffer			() { return m_buffer; }
	const void*	buffer_const	() const { return m_buffer; }
	void		set_auto_close	(bool v) { m_auto_close = v; }
	bool		is_auto_close	() const { return m_auto_close; }
	bool		is_open			() const;
	int			read			(void* data, const int len) const;

	enum SEEK
	{
		SET = 0,
		CUR = 1,
		END = 2,
	};
	void		seek			(const SEEK start, const int offset);

private:
	int			_get_file_size	() const;

private:
	void*		m_buffer;
	bool		m_auto_close;
	int			m_size;
	int			m_seek;		
#ifdef SCL_WIN
	void*		m_file_handle;
	void*		m_mapping_handle;
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	int			m_file;
#endif
};

} //namespace scl

