#pragma once

#include "ResourcePath.h"

namespace Echo
{
	class Res
	{
	public:
		// release
		virtual void release();

		// get res
		static Res* get(const ResourcePath& path);

	protected:
		Res(const ResourcePath& path);
		virtual ~Res();

	protected:
		int				m_refCount;
		ResourcePath	m_path;
	};
}