#pragma once

#include "ResourcePath.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class Res : public Object
	{
		ECHO_CLASS(Res, Object)

	public:
		Res();
		Res(const ResourcePath& path);
		virtual ~Res();

		// get res
		static Res* get(const ResourcePath& path);

		// bind methods to script
		static void bindMethods() {}

		// add ref count
		void addRefCount() { m_refCount++; }

		// release
		void subRefCount();

		// is loaded succeed
		bool isLoaded() const { return m_isLoaded; }

	protected:
		int								m_refCount;
		ResourcePath					m_path;
		bool							m_isLoaded;
	};

	template<typename T>
	class ResRef
	{
	public:
		ResRef()
			: m_ptr(nullptr)
		{}

		ResRef(T* ptr)
		{
			if (ptr)
			{
				ptr->addRefCount();
				m_ptr = ptr;
			}
			else
			{
				m_ptr = nullptr;
			}
		}

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

		void operator = (ResRef<T>& orig)
		{
			if (m_ptr)
			{
		 	m_ptr->subRefCount();
				m_ptr = nullptr;
			}

			if (orig->m_ptr)
			{
				orig->addRefCount();
				m_ptr = orig->m_ptr;
			}
		}

		void operator = (T* orig)
		{
			if (m_ptr)
			{
				m_ptr->subRefCount();
				m_ptr = nullptr;
			}

			if (orig)
			{
				orig->addRefCount();
				m_ptr = orig;
			}
		}

		T* operator -> ()
		{
			return m_ptr;
		}

		T* ptr()
		{
			return m_ptr;
		}

		operator T*() const 
		{ 
			return m_ptr; 
		}

	private:
		T*		m_ptr;
	};
}