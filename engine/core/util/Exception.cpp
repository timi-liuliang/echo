#include "Exception.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	Exception::Exception()
		: m_lineNum(0)
	{
	}

	Exception::Exception(const String& msg)
		: m_msg(msg)
		, m_lineNum(0)
	{
	}

	Exception::Exception(const String &msg, const String &filename, ui32 lineNum)
		: m_msg(msg)
		, m_filename(filename)
		, m_lineNum(0)
	{
	}

	Exception::~Exception()
	{
	}

	const String& Exception::getMessage() const
	{
		return m_msg;
	}

	void Exception::setMessage(const String& msg)
	{
		m_msg = msg;
	}

	const String& Exception::getFilename() const
	{
		return m_filename;
	}

	ui32 Exception::getLineNum() const
	{
		return m_lineNum;
	}

	void __EchoThrowException(const String& msg, const char* filename, ui32 lineNum)
	{
		EchoLogError("EchoThrowException[%s] file[%s] line[%d]", msg.c_str(), filename, lineNum);
		throw Exception(msg, filename, lineNum);
	};
}