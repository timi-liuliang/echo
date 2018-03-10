#pragma once

#include "Foundation/Memory/MemManager.h"

namespace Echo
{
	template<class _Ty>
	struct default_delete
	{	// default deleter for unique_ptr
		typedef default_delete<_Ty> _Myt;

		default_delete()
		{	// default construct
		}

		template<class _Ty2,
		class = typename std::enable_if<std::is_convertible<_Ty2 *, _Ty *>::value,
			void>::type>
			default_delete(const default_delete<_Ty2>&)
		{	// construct from another default_delete
		}

		void operator()(_Ty *_Ptr) const
		{	// delete a pointer
			static_assert(0 < sizeof(_Ty),
			"can't delete an incomplete type");
			EchoSafeDelete(_Ptr, _Ty);
		}
	};

	template<typename T>
	using UniquePtr = std::unique_ptr<T, default_delete<T>>;
}