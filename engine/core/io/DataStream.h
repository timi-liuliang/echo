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
		DataStream(ui16 accessMode = READ) : mSize(0), m_buffer(nullptr), mAccess(accessMode) {}
		DataStream(const String& name, ui16 accessMode = READ) : mName(name), mSize(0), m_buffer(nullptr), mAccess(accessMode) {}
		virtual ~DataStream();

		const String& getName(void) { return mName; }
		ui16 getAccessMode() const { return mAccess; }
		virtual bool isReadable() const { return (mAccess & READ) != 0; }
		virtual bool isWriteable() const { return (mAccess & WRITE) != 0; }
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

		size_t size(void) const { return mSize; }

		virtual const void* getBuffer();

		virtual void close(void) {}

        void readAll( void* buffer );

        // lua test
        void test(const char* msg);

	protected:
		String	mName;
		size_t	mSize;
		char*	m_buffer;
		ui16	mAccess;
	};

	class MemoryDataStream : public DataStream
	{		
	public:
		MemoryDataStream(void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false);
		MemoryDataStream(const String& name, void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false);
		MemoryDataStream(DataStream& sourceStream, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(DataStream* sourceStream, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(const String& name, DataStream& sourceStream, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(const String& name, DataStream* sourceStream, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(size_t size, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(const String& name, size_t size, bool freeOnClose = true, bool readOnly = false);
		~MemoryDataStream();

		// 获取数据流
		virtual const void* getBuffer() { return mData; }

		ui8* getPtr(void) { return mData; }

		ui8* getCurrentPtr(void) { return mPos; }

		size_t read(void* buf, size_t count);

		size_t write(const void* buf, size_t count);

		size_t readLine(char* buf, size_t maxCount, const String& delim = "\n");

		size_t skipLine(const String& delim = "\n");

		void skip(long count);

		void seek(size_t pos, int origin = SEEK_SET);

		size_t tell(void) const;

		bool eof(void) const;

		void close(void);

		void setFreeOnClose(bool free) { mFreeOnClose = free; }

        virtual bool isMemoryDS() const { return true; }

	protected:
		ui8*	mData;
		ui8*	mPos;
		ui8*	mEnd;
		bool	mFreeOnClose;
	};

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
