#pragma once

#include <engine/core/base/object.h>

namespace Echo
{
	class Light : public Object
	{
		ECHO_CLASS(Light, Object);

	public:
        virtual ~Light();
		Light();

	protected:

	};
}
