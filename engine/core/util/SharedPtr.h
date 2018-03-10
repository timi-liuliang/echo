#pragma once

#include "UniquePtr.h"

namespace Echo
{
	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template<typename T>
	struct SharedPtrDelete
	{
		void operator() (T* t) const
		{
			EchoSafeDelete(t, T);
		}
	};

}