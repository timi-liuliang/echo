#pragma once

#include "command.h"

namespace Echo
{
	/**
	 * Terminal - some times we don't need make a ui for all functional.
	 * We use terminal singleton to run command
	 */
	class Terminal : public Object
	{
		ECHO_SINGLETON_CLASS(Terminal, Object)

	public:
		Terminal();
		virtual ~Terminal();

		// instance
		static Terminal* instance();

		// execute command
		bool execCmd();
	};
}