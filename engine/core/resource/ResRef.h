#pragma once

namespace Echo
{
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

		ResRef(const ResRef<T>& orig)
		{
			if (orig.m_ptr)
			{
				m_ptr = orig.m_ptr;
				m_ptr->addRefCount();
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

			if (orig.m_ptr)
			{
				m_ptr = orig.m_ptr;
                m_ptr->addRefCount();
			}
		}
        
        void operator = (const ResRef<T>& orig)
        {
            if (m_ptr)
            {
                m_ptr->subRefCount();
                m_ptr = nullptr;
            }
            
            if (orig.m_ptr)
            {
                m_ptr = orig.m_ptr;
                m_ptr->addRefCount();
            }
        }

		void operator = (T* orig)
		{
			if (m_ptr != orig)
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
