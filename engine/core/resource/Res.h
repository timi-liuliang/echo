#pragma once

#include "ResourcePath.h"

namespace Echo
{
	class Res
	{
	public:
		// get res
		static Res* get(const ResourcePath& path);

		// add ref count
		void addRefCount() { m_refCount++; }

		// release
		void subRefCount();

	protected:
		Res(const ResourcePath& path);
		virtual ~Res();

	protected:
		int				m_refCount;
		ResourcePath	m_path;
	};

	template<typename T>
	class ResRef
	{
	public:
		ResRef()
			: m_ptr(nullptr)
		{}

		~ResRef()
		{
			reset();
		}

		void reset()
		{
			if (m_ptr)
			{
				m_ptr->subRefCount();
				m_ptr = nullptr;
			}
		}

		void operator = (T* ptr)
		{
			if (m_ptr)
			{
				m_ptr->subRefCount();
				m_ptr = nullptr;
			}

			if (ptr)
			{
				ptr->addRefCount();
				m_ptr = ptr;
			}
		}

		T* operator -> ()
		{
			return m_ptr;
		}

	private:
		T*		m_ptr;
	};
}