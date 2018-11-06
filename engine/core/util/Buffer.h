#pragma once

#include "engine/core/base/type_def.h"

namespace Echo
{
	class Buffer
	{
	public:
		Buffer();
		Buffer(ui32 size, void* pData = nullptr, bool bAutoFree = false);
		virtual ~Buffer();

		virtual void allocate(ui32 size);
		virtual void clear();
		virtual void set(ui32 size, void *pData, bool bAutoFree = false);
		virtual void copyBuffer(const Buffer &buff);
		virtual Byte* getData() const;
		virtual ui32 getSize() const;
		virtual ui32 takeData(Byte* &pData);

	protected:
		Byte*		m_pData;
		ui32		m_size;
		bool		m_bAutoFree;
	};
}
