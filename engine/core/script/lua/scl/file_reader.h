#pragma once

namespace scl {

#ifndef NULL
#define NULL 0
#endif

// there is 2 buffers in file_read, read buffer and line buffer
#define DEFAULT_MAX_BUFFER_SIZE (64 * 1024)	// max size of read buffer by bytes
#define DEFAULT_LINE_CACHE_SIZE (64 * 1024)	// max size of line buffer by bytes
#define RESERVE_LINEBREAK_SIZE	3			// see comment of "file_reader::reserve_linebreak"

//	NOTICE! this class used "new" to malloc memory, so do NOT use it on a frequently called function stack.

////////////////////////////////////
//	read file line by line, used cache to SPEED UP contrast to "fgets"
//		NOTICE! this class used "new" to malloc memory, so do NOT use it on a frequently called function stack.
//	
////////////////////////////////////
class file_reader
{
public:
	file_reader(const int maxBufferSize = DEFAULT_MAX_BUFFER_SIZE, const int m_maxLineCacheSize = DEFAULT_LINE_CACHE_SIZE);
	~file_reader();

	bool		open							(const char* const filename);	//打开一个文件
	bool		open_buffer						(const char* const buffer, const int len);		//打开一个缓冲区
	bool		nextline						();		//进入下一行
	char*		currentline						();		//获取当前行的内容
	void		clear							();		//清空所有缓冲区和游标状态，不会导致缓冲区内存释放
	void		reserve_linebreak				(char* c);	//设置解析每行后，保留的换行符格式。如果不设置，currentline() 获得的行就不包括换行符

private:
	bool		_append_string_in_next_block	();		//当m_buffer中无法包含整行内容时，使用m_line缓存当前行
	bool		_find_next_start				();		//找到下一行的开始位置
	bool		_find_end						(bool useLineCache = false);		//找到下当前行结束的地方,useLineCache参数用来指示当m_buff无法完整读入一行时，是否尝试使用扩展m_line缓冲区
	bool		_load_block						();		//从文件中读取一个新的byte块
	int			_block_end						() { return m_readByteCount; }	//当前读取块的结束位置
	bool		_is_line_end					(const char c) { return c == '\r' || c == '\n' || c == 0; } //返回当前字符是否为一行的结束

private:

	////////////////////////////////////
	//
	//	file_read可以通过两种不同的方式来读取文件
	//
	//	1.当使用open(filename)时，使用m_file文件指针和fread()来读取文件内容
	//	2.当使用open_buffer(buffer, len)，就不再使用fread来读取文件，而是使用m_filebuffer_len和m_filebuffer_pos来读取文件内容
	//
	////////////////////////////////////
	void*		m_file;							//fopen返回的文件句柄
	const char*	m_filebuffer;					//文件缓冲区
	int			m_filebuffer_len;				//文件缓冲区长度
	int			m_filebuffer_pos;				//文件缓冲区当前指针


	//用于处理文件的临时缓冲区
	char*		m_buffer;						//缓冲区
	char*		m_line;							//临时用行缓冲区，当m_buffer中无法包含整行内容时（中间包含了换行，导致无法容纳一个完整行），使用m_line缓存当前行

	int			m_start;						//m_buffer的起始点游标，标记一行的开始位置
	int			m_end;							//m_buffer的结束点游标，标记一行的结束位置
	int			m_readByteCount;				//某次fread返回的字节数，用于_block_end()函数标识当前读取块的结束为止，注意，该位置与换行符没有任何逻辑关系
	bool		m_usingLineCache;				//标明当前是否正在使用m_line缓冲区
	char		m_linebreak[RESERVE_LINEBREAK_SIZE]; //解析每行后，保存的换行符格式

	//配置
	const int	m_maxBufferSize;				//一般缓冲区的最大容量
	const int	m_maxLineCacheSize;				//临时用的“行缓冲区”的最大容量
};

} //namespace scl


