#include "ProjectMgr.h"
#include "LogPanel.h"
#include "Studio.h"
#include <engine/core/main/Engine.h>
#include <engine/core/util/PathUtil.h>
#include "FreeImageHelper.h"

namespace Studio
{

	ProjectMgr::ProjectMgr()
	{

	}

	ProjectMgr::~ProjectMgr()
	{

	}

	// 新建项目文件
	void ProjectMgr::NewProject(const char* projectName)
	{
	}

	// 打开项目文件
	void ProjectMgr::OpenProject(const char* fullPathName)
	{
		AStudio::instance()->getConfigMgr()->addRecentProject(fullPathName);
	}

	// 资源检测
	void ProjectMgr::CheckProject()
	{
		// 清空
		m_files.clear();

		// 检测纹理
		checkTextures();
	}

	// 检测纹理资源
	void ProjectMgr::checkTextures()
	{
		// 分解后缀
		Echo::StringArray exts = Echo::StringUtil::Split(".tga|.bmp|.jpg|.png", "|");

		// 检测纹理
		for ( Echo::map<Echo::String, Echo::String>::type::iterator it = m_files.begin(); it != m_files.end(); it++ )
		{
			for ( size_t k = 0; k < exts.size(); k++ )
			{
				Echo::String fileExt = Echo::PathUtil::GetFileExt(it->first, true);
				if ( fileExt == exts[k] )
				{
					Echo::String fileLocation = it->second;
					Echo::FreeImageHelper::ImageInfo info;
					Echo::FreeImageHelper::getImageInfo(fileLocation.c_str(), info);
					if ( info.m_width != info.m_height )
					{
						Echo::String msg = Echo::StringUtil::Format("Texture [%s] width [%d] is not equal to height [%d]!", it->first.c_str(), info.m_width, info.m_height);
						//AStudio::Instance()->getLogPanel()->Error(msg.c_str());
					}
				}
			}
		}

	}

	// 添加文件
	void ProjectMgr::AddFileToProject(const char* fullPath)
	{
		Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
		Echo::StringUtil::LowerCase(fileName);
		if ( m_files.find(fileName) == m_files.end() )
			m_files[fileName] = fullPath;
	}

	void ProjectMgr::RemoveFileToProject(const char* fileName)
	{
		Echo::String str(fileName);
		Echo::StringUtil::LowerCase(str);
		if ( m_files.find(str) == m_files.end() )
			m_files.erase(m_files.find(str));
	}

	// 获取全路径
	Echo::String ProjectMgr::GetFullPath(const Echo::String& name)
	{
		Echo::map<Echo::String, Echo::String>::type::iterator it = m_files.find(name);
		if ( it != m_files.end() )
			return it->second;

		return "";
	}

	// 判断文件是否存在
	bool ProjectMgr::IsFileExist(const char* fileName)
	{
		Echo::String lowerFileName = fileName;
		Echo::StringUtil::LowerCase(lowerFileName);
		Echo::map<Echo::String, Echo::String>::type::iterator it = m_files.find(lowerFileName);
		if ( it != m_files.end() )
			return true;

		return false;
	}
}