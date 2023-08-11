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

    bool OS::createProcessDetached(const String& command)
    {
    #ifdef ECHO_PLATFORM_WINDOWS
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(si);

        if (!CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
            return false;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    #endif

        return true;
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

    vector<i64>::type OS::getProcessID(const String& name)
    {
        ProcessInfoArray infos;
        enumAllProcesses(infos);

        vector<i64>::type result;
        for (ProcessInfo info : infos)
        {
            if(info.m_name == name)
                result.emplace_back(info.m_id);
        }

        return result;
    }

    bool OS::isProcessExist(const String& name)
    {
        return getProcessID(name).size() >= 0;
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

    void OS::closeProcesses(vector<i64>::type processIds)
    {
#ifdef ECHO_PLATFORM_WINDOWS
        for (i64 processId : processIds)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
            if (hProcess)
            {
                bool success = TerminateProcess(hProcess, 0);
                if (success)
                {

                }
            }
        }
#endif
    }

    String OS::getLocalIPV4()
    {
        return false;

#ifdef ECHO_PLATFORM_WINDOWS
        char szBuffer[1024];

        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 0);
        if (::WSAStartup(wVersionRequested, &wsaData) != 0)
            return "";


        if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
        {
            WSACleanup();
            return "";
        }

        struct hostent* host = gethostbyname(szBuffer);
        if (host == NULL)
        {
            WSACleanup();
            return "";
        }

        //Obtain the computer's IP
        unsigned char b1 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b1;
        unsigned char b2 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b2;
        unsigned char b3 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b3;
        unsigned char b4 = ((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b4;

        WSACleanup();
        
        return StringUtil::Format("%d.%d.%d.%d", int(b1), int(b2), int(b3), int(b4));
#endif

        return "";
    }
}