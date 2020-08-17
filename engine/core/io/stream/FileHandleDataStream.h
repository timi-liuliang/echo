#pragma once

#include "DataStream.h"

namespace Echo
{
	class FileHandleDataStream : public DataStream
	{
	public:
		FileHandleDataStream(const String& name, ui32 accessMode = READ);
		~FileHandleDataStream();

		size_t read(void* buf, size_t count);

		size_t write(const void* buf, size_t count);

		void skip(long count);

		void seek(size_t pos, int origin = SEEK_SET);

		size_t tell(void) const;

		bool eof(void) const;

		void close(void);

		// is fail
		bool fail() { return m_fileHandle ? false : true; }

    protected:
        FILE*       m_fileHandle;
	};
}
