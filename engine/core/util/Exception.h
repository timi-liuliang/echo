#pragma once

#include "engine/core/Util/StringUtil.h"
#include "engine/core/log/LogManager.h"

namespace Echo
{
	class Exception
	{
	public:
		Exception();
		Exception(const String& msg);
		Exception(const String& msg, const String& filename, ui32 lineNum);
		virtual ~Exception();

		//Methods
		virtual const String&	getMessage() const;
		virtual void			setMessage(const String &msg);

		virtual const String&	getFilename() const;
		virtual ui32			getLineNum() const;

	protected:
		String		m_msg;
		String		m_filename;
		int			m_lineNum;
	};

	INLINE void __EchoThrowException(const String& msg, const char* filename, ui32 lineNum)
	{
		EchoLogError( "EchoThrowException[%s] file[%s] line[%d]", msg.c_str(), filename, lineNum);
		throw Exception(msg, filename, lineNum);
	};
}