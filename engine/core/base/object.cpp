#include "object.h"

namespace Echo
{
	Object::Object()
	{
	}

	// get class name
	const String& Object::getClassName() const
	{
		static String className = "Object";
		return className;
	}
}