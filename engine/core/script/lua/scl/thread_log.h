#pragma once

#include "scl/ring_buffer.h"
#include "scl/string.h"
#include "scl/type.h"
#include "scl/time.h"
#include "scl/varray.h"
#include "scl/log_define.h"

namespace scl {

class thread_log
{
public:
	typedef void (*log_handler_t)(const log_header& header, const char* const s, int thread_id);

	thread_log						();
	~thread_log						();
	bool		init				(log_handler_t log_handler, log_level* levels, const int levelCount, const int buffer_size = MAX_THREAD_LOG_BUFFER_SIZE);
	void		flush				();
	void		write				(log_header& header, const char* const s);

	bool		has_init			() const	{ return m_buffer.max_size() > 0;  }
	bool		has_data			() volatile { return m_buffer.used() > 0; }
	int			thread_id			() const { return m_thread_id; }
	void		set_thread_id		(const int thread_id) { assert(m_using); m_thread_id = thread_id; }
	bool		is_thread			(const int thread_id) const;
	bool		ready_to_work		();

private:
	void		_check_close		();
	void		_close				();
	void		_write_log			(log_header& header, const char* const s);
	bool		_has_a_complete_log	(log_header& header);
	bool		_is_frequent_log	(log_header& header);
	void		_add_frequent_log	(log_header& header);

private:
	volatile uint			m_using;
	int						m_thread_id;
	ring_buffer				m_buffer;
	pstring					m_log;
	uint64					m_close_timer;
	log_handler_t			m_log_handler;
	varray<frequent_log>	m_frequent_logs;
	timer					m_frequent_clear_timer;
	varray<log_level>		m_levels;
	timer					m_check_timer;
};

} //namespace scl

