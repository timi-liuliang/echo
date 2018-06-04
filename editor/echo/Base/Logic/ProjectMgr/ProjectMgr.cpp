#include "ProjectMgr.h"
#include "LogPanel.h"
#include "Studio.h"
#include <engine/core/main/Engine.h>
#include <engine/core/util/PathUtil.h>
#include "FreeImageHelper.h"

namespace Studio
{

	ProjectMgr::ProjectMgr()
		:m_pTextureCompreses(NULL)
	{

	}

	ProjectMgr::~ProjectMgr()
	{

	}

	// 新建项目文件
	void ProjectMgr::NewProject(const char* projectName)
	{
		Echo::ProjectSettings projectFile;
		m_pTextureCompreses = NULL;
		projectFile.save(projectName);
	}

	// 打开项目文件
	void ProjectMgr::OpenProject(const char* fullPathName)
	{
		m_projectFile.load(fullPathName);
		m_pTextureCompreses = m_projectFile.getTextureCompreses();
		//LoadImgCompressInfo(fileName);
		// 检测项目
		//CheckProject();

		AStudio::instance()->getConfigMgr()->addRecentProject(fullPathName);
	}

	// 资源检测
	void ProjectMgr::CheckProject()
	{
		// 清空
		m_files.clear();

		// 检测同名文件
		Echo::vector<Echo::ProjectSettings::ArchiveItem>::type& archives = m_projectFile.getArchives();
		for ( size_t archiveIdx = 0; archiveIdx < archives.size(); archiveIdx++ )
		{
			Echo::StringArray strs;
			Echo::String fullPath = m_projectFile.getPath() + archives[archiveIdx].m_archiveValue;
			Echo::PathUtil::EnumFilesInDir(strs, fullPath, false, true, true);

			// 记录名称与路径映射
			for ( size_t j = 0; j < strs.size(); j++ )
			{
				Echo::String fileName = Echo::PathUtil::GetPureFilename(strs[j]);
				Echo::StringUtil::LowerCase(fileName);
				if ( m_files.find(fileName) == m_files.end() )
					m_files[fileName] = strs[j];
			}
		}

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

	// 加载项目文件
	bool ProjectMgr::LoadProject(const char* fileName)
	{
		m_projectFile.load(fileName);
		m_pTextureCompreses = m_projectFile.getTextureCompreses();
		return true;
	}

	// 保存项目文件
	void ProjectMgr::SaveProject()
	{
		m_projectFile.save(m_projectFile.getPathName().c_str());
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

	// 添加图片压缩格式信息
	void ProjectMgr::addCompressChangeTextrue(PLATFORM platform, std::string name, Echo::ProjectSettings::TextureCompressType ctype)
	{
		if (m_pTextureCompreses)
		{
			Echo::ProjectSettings::TextureCompressItem* pItem = NULL;
			Echo::ProjectSettings::TextureCPIVec::iterator it = m_pTextureCompreses->begin();
			for (; it != m_pTextureCompreses->end(); ++it)
			{
				if (it->m_name == name)
				{
					pItem = &(*it);
					break;
				}
			}

			if (!pItem)
			{
				Echo::ProjectSettings::TextureCompressItem item;
				item.m_name = name;
				m_pTextureCompreses->push_back(item);
				pItem = &item;
			}

			switch (platform)
			{
			case IOS:
				pItem->m_iosCType = ctype; break;
			case ANDROID:
				pItem->m_androidCType = ctype; break;
			case WINDOWS:
				break;
			default:
				break;
			}
		}
	}
	

	Echo::ProjectSettings::TextureCompressItem* ProjectMgr::GetTextureCompressItem(const Echo::String& name)
	{
		if (m_pTextureCompreses)
		{
			Echo::ProjectSettings::TextureCompressItem* pItem;
			Echo::ProjectSettings::TextureCPIVec::iterator it = m_pTextureCompreses->begin();
			for (; it != m_pTextureCompreses->end(); ++it)
			{
				pItem = &(*it);
				if (pItem->m_name == name)
					return pItem;
			}
		}

		return NULL;
	}
}