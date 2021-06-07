#pragma once

#include "engine/core/terminal/command.h"

namespace Echo
{
	class PvdCommand : public Command
	{
		ECHO_CLASS(PvdCommand, Command)

	public:
		PvdCommand();
		virtual ~PvdCommand();

		// exec
		virtual bool exec(const StringArray& args);
	};
}