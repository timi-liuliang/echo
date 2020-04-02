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

	void ProjectMgr::newProject(const char* projectName)
	{
	}

	void ProjectMgr::OpenProject(const char* fullPathName)
	{
		AStudio::instance()->getConfigMgr()->addRecentProject(fullPathName);
	}

	void ProjectMgr::CheckProject()
	{
		// clear
		m_files.clear();

		// check textures
		checkTextures();
	}

	void ProjectMgr::checkTextures()
	{
		Echo::StringArray exts = Echo::StringUtil::Split(".tga|.bmp|.jpg|.png", "|");
		for ( Echo::map<Echo::String, Echo::String>::type::iterator it = m_files.begin(); it != m_files.end(); it++ )
		{
			for ( size_t k = 0; k < exts.size(); k++ )
			{
				Echo::String fileExt = Echo::PathUtil::GetFileExt(it->first, true);
				if ( fileExt == exts[k] )
				{
					Echo::String fileLocation = it->second;
					Echo::FreeImageHelper::ImageInfo info;
					Echo::FreeImageHelper::instance()->getImageInfo(fileLocation.c_str(), info);
					if ( info.m_width != info.m_height )
					{
						Echo::String msg = Echo::StringUtil::Format("Texture [%s] width [%d] is not equal to height [%d]!", it->first.c_str(), info.m_width, info.m_height);
						//AStudio::Instance()->getLogPanel()->Error(msg.c_str());
					}
				}
			}
		}

	}

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

	Echo::String ProjectMgr::GetFullPath(const Echo::String& name)
	{
		Echo::map<Echo::String, Echo::String>::type::iterator it = m_files.find(name);
		if ( it != m_files.end() )
			return it->second;

		return "";
	}

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
