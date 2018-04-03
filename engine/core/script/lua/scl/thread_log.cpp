#include "scl/thread_log.h"

#include "scl/time.h"
#include "scl/thread.h"
#include "scl/log.h"

namespace scl {

thread_log::thread_log() : 
	m_using			(0),
	m_thread_id		(INVALID_THREAD_ID), 
	m_close_timer	(0),
	m_log_handler	(NULL)
{
	
}

thread_log::~thread_log()
{
	m_log.free();
}

bool thread_log::init(log_handler_t log_handler, log_level* levels, const int levelCount, const int buffer_size) // buffer_size = MAX_THREAD_LOG_BUFFER_SIZE
{
	m_log_handler = log_handler;
	m_buffer.alloc(buffer_size);
	m_log.alloc(MAX_LOG_STRING_LENGTH);
	m_frequent_logs.reserve(FREQUENT_LOG_MAX_COUNT);
	m_frequent_clear_timer.start(FREQUENT_LOG_CLEAR_INTERVAL);
	m_frequent_clear_timer.set_update_to_now(true);
	m_levels.assign(levels, levelCount);
	m_check_timer.start(THREAD_LOG_CHECK_INTERVAL);
	return true;
}


void thread_log::write(log_header& header, const char* const s)
{
	if (m_thread_id == INVALID_THREAD_ID)
		return;

	//检查日志是否重复
	if (m_levels[header.level].prevent_frequent 
		&& _is_frequent_log(header))
		return;

	_write_log(header, s);
}

void thread_log::_write_log(log_header& header, const char* const s)
{
	header.begin_mark	= BEGIN_MARK;
	header.len			= static_cast<int>(::strlen(s));
	assert(header.len > 0 && header.len < MAX_LOG_STRING_LENGTH);

	const int total_length = sizeof(header) + header.len + sizeof(END_MARK);
	if (m_buffer.free() < total_length)
	{
		//缓冲区已满
		string1024 urgency_msg;
		urgency_msg.format("thread_log buffer is full! thread id[%d] m_buffer.free = %d, total_log_length = %d\n", m_thread_id, m_buffer.free(), total_length);
		urgency_log(urgency_msg.c_str(), "logfull.log");
		return;
	}

	m_buffer.write(&header, sizeof(header));
	m_buffer.write(s, header.len);
	m_buffer.write(&END_MARK, sizeof(END_MARK));
}

void thread_log::flush()
{
	if (m_thread_id == INVALID_THREAD_ID)
		return;

	_check_close();

	log_header header;
	if (_has_a_complete_log(header))
	{	
		m_buffer.drop(sizeof(header));

		//读取log内容
		m_log.clear();
		m_buffer.read(m_log.c_str(), header.len);

		//检查log结束标志
		byte check_mark = 0;
		m_buffer.read(&check_mark, sizeof(check_mark));
		if (check_mark != END_MARK)
		{
			assertf(false, "invalid end_mark 0x%02x", check_mark);
			return;
		}

		//打印日志
		if (NULL != m_log_handler)
			m_log_handler(header, m_log.c_str(), m_thread_id);
	}
}


bool thread_log::ready_to_work()
{
	return compare_and_swap(&m_using, 0, 1);
}

bool thread_log::is_thread(const int thread_id) const
{
	return m_thread_id == thread_id && m_close_timer == 0;
}

void thread_log::_close()
{
	printf("thread[%d] log closed\n", m_thread_id);
	m_thread_id		= INVALID_THREAD_ID;
	m_close_timer	= 0;
	m_frequent_clear_timer.pause();
	m_buffer.clear();
	m_using			= 0; //这里务必最后置using为0
}

void thread_log::_check_close()
{
	if (m_close_timer != 0) //if is wait closing
	{
		uint64 now = SCL_TICK;
		if (now - m_close_timer > THREAD_LOG_CLOSE_DELAY)
			_close();
	}
	else
	{
		if (m_check_timer.ring())
		{
			if (!thread::exists(m_thread_id) && thread::main_thread_id() != m_thread_id)
				m_close_timer = SCL_TICK; //start close
		}
	}
}

bool thread_log::_has_a_complete_log(log_header& header)
{
	const int buffer_len = m_buffer.used();
	if (buffer_len < static_cast<int>(sizeof(log_header)))
		return false;

	//检查header
	m_buffer.peek(&header, sizeof(header));
	if (header.begin_mark != BEGIN_MARK)
	{
		assertf(false, "invalid begin_mark 0x%02x", header.begin_mark);
		return false;
	}
	assert(header.len < MAX_LOG_STRING_LENGTH);

	if (buffer_len < static_cast<int>(sizeof(log_header) + header.len + sizeof(END_MARK)))
		return false;

	return true;
}

bool thread_log::_is_frequent_log(log_header& header)
{
	//如果频繁日志清空计时器到时了
	if (m_frequent_clear_timer.ring())
	{
		log_header frequent_header;
		frequent_header.time		= SCL_TIME;
		frequent_header.format		= LOG_FORMAT_TIME | LOG_FORMAT_MILLISECOND | LOG_FORMAT_LINE_FEED | LOG_FORMAT_LEVEL;
		frequent_header.level		= LOG_LEVEL_INFO;
		for (int i = 0; i < m_frequent_logs.size(); ++i)
		{
			frequent_log& l = m_frequent_logs[i];
			if (l.times > FREQUENT_LOG_TIMES_LIMIT)
			{
				string256 strlog;
				strlog.format("clearing frequent logs! file[%s:%d] times[%d]", l.filename.c_str(), l.line, l.times);
				_write_log(frequent_header, strlog.c_str());
			}
		}
		m_frequent_logs.clear();
		return false;
	}

	//查找是否已经记录该日志
	for (int i = 0; i < m_frequent_logs.size(); ++i)
	{
		frequent_log& l = m_frequent_logs[i];
		if (l.filename == header.filename && l.line == header.line)
			return ++l.times > FREQUENT_LOG_TIMES_LIMIT;
	}

	//没找到，将当前日志信息加入
	if (m_frequent_logs.size() < m_frequent_logs.capacity())
	{
		_add_frequent_log(header);
	}
	else //频繁日志队列已满，需要清除一个，然后将当前加入
	{
		//找到打印次数最少的日志的index
		int least_index	= 0;
		int least		= 0x7FFFFFFF;
		for (int i = 0; i < m_frequent_logs.size(); ++i)
		{
			frequent_log& l = m_frequent_logs[i];
			if (l.times < least)
			{
				least_index = i;
				least		= l.times;
			}
		}
		m_frequent_logs.erase(least_index);
		_add_frequent_log(header);
	}

	return false;
}

void thread_log::_add_frequent_log(log_header& header)
{
	if (m_frequent_logs.size() == m_frequent_logs.capacity())
	{
		assert(false);
		return;
	}
	frequent_log& l = m_frequent_logs.push_back_fast();
	l.clear();
	l.filename	= header.filename;
	l.line		= header.line;
	l.times		= 0;
}




}	// namespace scl

