#include "Exception.h"

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
}