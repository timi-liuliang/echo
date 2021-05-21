#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/base/object.h"

namespace Echo
{
	#define ECHO_STREAM_TEMP_SIZE 128

	class DataStream : public Object
	{
        ECHO_CLASS(DataStream, Object);

	public:
		enum AccessMode
		{
			READ = 1, 
			WRITE = 2
		};

	public:
		DataStream(ui16 accessMode = READ) : m_size(0), m_buffer(nullptr), m_access(accessMode) {}
		DataStream(const String& name, ui16 accessMode = READ) : m_name(name), m_size(0), m_buffer(nullptr), m_access(accessMode) {}
		virtual ~DataStream();

		const String& getName(void) { return m_name; }
		ui16 getAccessMode() const { return m_access; }
		virtual bool isReadable() const { return (m_access & READ) != 0; }
		virtual bool isWriteable() const { return (m_access & WRITE) != 0; }
        virtual bool isMemoryDS() const { return false; }
		template<typename T> DataStream& operator>>(T& val);
        virtual size_t read(void* buf, size_t count) { return 0; }
		virtual size_t write(const void* buf, size_t count);

		virtual size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");

		virtual String getLine( bool trimAfter = true );

		virtual String getAsString(void);

		virtual size_t skipLine(const String& delim = "\n");

		virtual void skip(long count) {}

		virtual void seek(size_t pos, int origin=SEEK_SET) {}

        virtual size_t tell(void) const { return 0; }

        virtual bool eof(void) const { return true; }

		size_t size(void) const { return m_size; }

		virtual const void* getBuffer();

		virtual void close(void) {}

        void readAll( void* buffer );

	protected:
		String	m_name;
		size_t	m_size;
		char*	m_buffer;
		ui16	m_access;
	};

	LUA_PUSH_VALUE(DataStream)
}
