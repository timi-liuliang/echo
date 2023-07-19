#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
	class OS
	{
	public:
		struct ProcessInfo
		{
			i64		m_id;
			String	m_name;
		};
		typedef vector<ProcessInfo>::type ProcessInfoArray;

	public:
		~OS();

	public:
		// Get process name by id
		String getProcessName(i64 processID);

		// Get process id by name
		i64 getProcessID(const String& name);

		// Is process exist
		bool isProcessExist(const String& name);

		// Enumerate all process infos
		void enumAllProcesses(ProcessInfoArray& infos);

	public:
		OS();
	};
}