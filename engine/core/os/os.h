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
		// Create process detatched
		static bool createProcessDetached(const String& command);

		// Get process name by id
		static String getProcessName(i64 processID);

		// Get process id by name
		static i64 getProcessID(const String& name);

		// Is process exist
		static bool isProcessExist(const String& name);

		// Enumerate all process infos
		static void enumAllProcesses(ProcessInfoArray& infos);

	public:
		// Get Ip
		static String getLocalIPV4();

	public:
		OS();
	};
}