#pragma once

#include "DataStream.h"

namespace Echo
{
	class MemoryDataStream : public DataStream
	{		
	public:
		MemoryDataStream(void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false);
		MemoryDataStream(const String& name, void* pMem, size_t size, bool freeOnClose = false, bool readOnly = false);
		MemoryDataStream(size_t size, bool freeOnClose = true, bool readOnly = false);
		MemoryDataStream(const String& name, size_t size, bool freeOnClose = true, bool readOnly = false);
		~MemoryDataStream();

		// get buffer
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
}
