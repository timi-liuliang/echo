#pragma once

#include "engine/core/base/type_def.h"

namespace Echo
{
	class Buffer
	{
	public:
        Buffer();
		Buffer(ui32 size, void* pData = nullptr, bool isAutoFree = false);
		~Buffer();

        // allocate|clear
		void allocate(ui32 size);
		void clear();

        // data operate
        void set(const Buffer &buff);
		void set(ui32 size, void *pData, bool bAutoFree = false);
		Byte* getData() const;

        // size in bytes
		ui32 getSize() const;
		ui32 takeData(Byte* &pData);

	protected:
		Byte*		m_data = nullptr;
		ui32		m_size = 0;
		bool		m_isAutoFree = false;
	};
}
