#include "os.h"
#include "engine/core/base/echo_def.h"

#ifdef ECHO_PLATFORM_WINDOWS
#include <windows.h>
#include <Psapi.h>
#endif

namespace Echo
{
	OS::OS()
	{

	}

	OS::~OS()
	{

	}

    String OS::getProcessName(i64 processID)
    {
        TCHAR szProcessName[MAX_PATH] = TEXT("");

    #ifdef ECHO_PLATFORM_WINDOWS
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (NULL != hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseName(hProcess, hMod, szProcessName,
                    sizeof(szProcessName) / sizeof(TCHAR));
            }
        }

        CloseHandle(hProcess);
    #endif

        return szProcessName;
    }

    i64 OS::getProcessID(const String& name)
    {
        ProcessInfoArray infos;
        enumAllProcesses(infos);

        for (ProcessInfo info : infos)
        {
            if(info.m_name == name)
                return info.m_id;
        }

        return -1;
    }

    bool OS::isProcessExist(const String& name)
    {
        return getProcessID(name) >=0;
    }

	void OS::enumAllProcesses(ProcessInfoArray& infos)
	{
	#ifdef ECHO_PLATFORM_WINDOWS
        DWORD aProcesses[1024], cbNeeded;
        unsigned int i;

        if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
        {
            DWORD cProcesses = cbNeeded / sizeof(DWORD);
            for (i = 0; i < cProcesses; i++)
            {
                i64 processID = aProcesses[i];
                if (processID != 0)
                {
                    String processName = getProcessName(processID);
                    if (!processName.empty())
                    {
                        ProcessInfo info;
                        info.m_id = processID;
                        info.m_name = processName;

                        infos.emplace_back(info);
                    }
                }
            }
        }

	#endif
	}
}