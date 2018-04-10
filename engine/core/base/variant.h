#pragma once

#include "TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class Variant
	{
	public:
		enum Type
		{
			Bool,
			Real,
			Vector3,
			String,
		};

		struct CallError
		{
			enum Error
			{
				CE_Ok,
				CE_InvalidMethod,
				CE_InvalidArgument,
				CE_TooManyArguments,
				CE_ToolFewArguments,
				CE_InstanceIsNull,
			};
		};

	public:
		Variant(){}
		Variant(const Echo::String& str){}

		// to string
		Echo::String toString() { return ""; }

	private:
		Type			m_type;

		union 
		{
			bool		m_bool;
			i64			m_int;
			float		m_real;
		};
	};
}