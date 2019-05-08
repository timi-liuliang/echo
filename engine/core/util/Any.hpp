#pragma once

#include <algorithm>
#include <typeinfo>
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	/**s
	 * Any蔙eference boost, http://www.boost.org
	 */
    class any
    {
		// 友元函数
		template<typename ValueType> friend ValueType * any_cast(any*);

    public:
		any() : content(0) {}    
        template<typename ValueType> any(const ValueType & value) : content(new holder<ValueType>(value)){}
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

		// 运算符重载 "="
        template<typename ValueType> any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

		// 运算符重载"="
        any & operator=(any rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    public:
		// 是否为空
        bool empty() const
        {
            return !content;
        }

		// 获取类型信息
        const std::type_info & type() const
        {
            return content ? content->type() : typeid(void);
        }

    private: 
        class placeholder
        {
        public:
            virtual ~placeholder(){}

			// 返回类型
            virtual const std::type_info & type() const = 0;

			// 复制函数
            virtual placeholder * clone() const = 0;
        };

        template<typename ValueType>
		class holder : public placeholder
        {
        public:
            holder(const ValueType & value) : held(value){}

			// 返回值类型
            virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }

			// 复制
            virtual placeholder * clone() const
            {
                return new holder(held);
            }

		private:
			// 禁用运算符 "="
			holder& operator=(const holder &);

        public:
            ValueType held;
        };

	private:
        placeholder * content;			// 存储内容
    };

	// 转换失败
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
