#pragma once

#include <algorithm>
#include <typeinfo>
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	/**s
	 * Any reference boost, http://www.boost.org
	 */
    class any
    {
		template<typename ValueType> friend ValueType * any_cast(any*);

    public:
		any() : content(0) {}    
        template<typename ValueType> any(const ValueType & value) : content(EchoNew( holder<ValueType>(value))){}
        any(const any & other) : content(other.content ? other.content->clone() : 0){}
        ~any(){ EchoSafeDelete(content, placeholder);}

		// error
		static void error(const char* msg);

    public:
		// swap
        any & swap(any & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

		// operate "="
        template<typename ValueType> any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

		// override "="
        any & operator=(any rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    public:
		// is empty
        bool empty() const
        {
            return !content;
        }

        // clear
        void clear()
        {
            EchoSafeDelete(content, placeholder);
        }

		// get type
        const std::type_info & type() const
        {
            return content ? content->type() : typeid(void);
        }

    private: 
        class placeholder
        {
        public:
            virtual ~placeholder(){}

			// type
            virtual const std::type_info & type() const = 0;

			// clone
            virtual placeholder * clone() const = 0;
        };

        template<typename ValueType>
		class holder : public placeholder
        {
        public:
            holder(const ValueType & value) : held(value){}

			// type
            virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }

			// clone
            virtual placeholder * clone() const
            {
                return EchoNew( holder(held));
            }

		private:
			// override "="
			holder& operator=(const holder &);

        public:
            ValueType held;
        };

	private:
        placeholder* content = nullptr;
    };

	// bad cast
    class bad_any_cast : public std::bad_cast
    {
    public:
        virtual const char * what() const throw()
        {
            return "Echo::bad_any_cast: failed conversion using Echo::any_cast";
        }
    };

    template<typename ValueType>
    ValueType* any_cast(any * operand)
    {
        return operand && operand->type() == typeid(ValueType) ? &static_cast<any::holder<ValueType> *>(operand->content)->held : 0;
    }

    template<typename ValueType>
    inline const ValueType * any_cast(const any * operand)
    {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    ValueType& any_cast(any& operand)
    {
        ValueType* result = any_cast<ValueType>(&operand);
        if(!result)
			any::error( "any_cast failed,so terrible!");

        return *result;
    }

	template<typename ValueType>
	const ValueType& any_cast(const any& operand)
	{
		const ValueType* result = any_cast<ValueType>(&operand);
		if (!result)
			any::error("any_cast failed,so terrible!");

		return *result;
	}
}
