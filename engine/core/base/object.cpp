#include "object.h"

namespace Echo
{
	// get class name
	const String& Object::getClassName() const
	{
		static String className = "Object";
		return className;
	}
}