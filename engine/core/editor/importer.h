#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class Importer : public Object
	{
		ECHO_CLASS(Importer, Object);

	public:
		Importer();
		virtual ~Importer();

		// name
		virtual const char* getName() { return ""; }

		// import
		virtual void run(const char* targetFolder) {}
	};
}
#endif