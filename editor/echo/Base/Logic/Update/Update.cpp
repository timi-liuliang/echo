#include "Update.h"
#include <engine/core/Base/echo_def.h>
#include <engine/core/io/DataStream.h>
#include <engine/core/Util/PathUtil.h>
#include <QMessageBox>
#include <QApplication>

namespace Studio
{
	// 更新Astudio
	static void http_update_astudio_cb(void* arg, const char* url, void* data, size_t len, int responsecode)
	{
		// 写入当前目录Cache文件夹
		Echo::String savePath = Echo::PathUtil::GetCurrentDir() + "/update/astudio-setup.exe";
		Echo::PathUtil::WriteData(savePath.c_str(), data, len);

		// 启动
		Echo::String cmd = Echo::PathUtil::GetCurrentDir() + "/update/astudio-setup.exe";
		//WinExec(cmd.c_str(), SW_SHOW);
	}

	// 获取所有可更新版本
	static void http_get_all_versions_cb(void* arg, const char* url, void* data, size_t len, int responsecode)
	{
		Echo::StringArray* oResult = (Echo::StringArray*)arg;

		Echo::String httpText = (const char*)data;
		size_t beginPos = httpText.find(">astudio", 0);
		size_t endPos = httpText.find("setup.exe<", 0);
		while (beginPos != Echo::String::npos && endPos!=Echo::String::npos)
		{
			Echo::String astudio = httpText.substr(beginPos+1, endPos-beginPos+8);
			httpText = Echo::StringUtil::Replace(httpText, astudio, "");

			oResult->push_back(astudio);

			beginPos = httpText.find(">astudio", 0);
			endPos = httpText.find("setup.exe<", 0);
		}
	}

	// 获取当前可更新版本
	Echo::StringArray Update::getAllEnabledVersions()
	{
		Echo::StringArray result;
		//network::httpget("http://10.137.81.231:80/", http_get_all_versions_cb, &result);

		return result;
	}

	// 下载某版本并运行
	void Update::downloadVersion(const Echo::String& resName)
	{
		//network::httpget(("http://10.137.81.231:80/" + resName).c_str(), http_update_astudio_cb, nullptr);
	}
}