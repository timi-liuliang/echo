#include "ProjectSettings.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"
#include <ostream>

namespace Echo
{
	// 构造函数
	ProjectSettings::ProjectSettings()
	{
		addSetting("Application/Launch/LaunchScene", Variant::Type::String);
	}

	// 析构函数
	ProjectSettings::~ProjectSettings()
	{

	}

	// add setting
	void ProjectSettings::addSetting(const String& name, Variant::Type type)
	{
		auto it = m_settings.find(name);
		if (it == m_settings.end())
		{
			Setting setting;
			setting.m_name = name;
			setting.m_type = type;

			m_settings[name] = setting;
		}
		else
		{
			EchoLogError("Category [%s] has exist.", name.c_str());
		}
	}

	// 设置配置
	void ProjectSettings::setSetting(const String& name, const String& value)
	{

	}

	// 获取设置
	const ProjectSettings::Setting* ProjectSettings::getSetting(const String& name)
	{
		auto it = m_settings.find(name);
		if (it != m_settings.end())
		{
			return &(it->second);
		}

		return nullptr;
	}

	// 加载
	void ProjectSettings::load(const char* pathName)
	{
		try
		{
			// 记录路径，名称，全路径
			m_path = Echo::PathUtil::GetFileDirPath(pathName);
			m_name = Echo::PathUtil::GetLastPathName(pathName);
			m_pathName = pathName;

			// 解析项目文件
			pugi::xml_document doc;
			doc.load_file(m_pathName.c_str());

			// 项目结点
			pugi::xml_node projectNode = doc.child("project");
			if ( projectNode )
			{
				loadArchives(&projectNode);
			}
			else
			{
				EchoLogError("ProjectFile parse failed");
			}
		}
		catch ( ... )
		{
			EchoLogError("ProjectFile load failed");
		}
	}

	// 保存
	void ProjectSettings::save(const char* fileName)
	{
		if ( !fileName )
			fileName = m_pathName.c_str();

		pugi::xml_document doc;
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		pugi::xml_node rootnode = doc.append_child("project");

		// 保存Archive属性
		saveArchives(doc, &rootnode);

		// write xml
		doc.save_file(fileName, "\t", 1U, pugi::encoding_utf8);
	}

	// 加载存档配置
	void ProjectSettings::loadArchives(pugi::xml_node* projectNode)
	{
		m_archives.clear();

		pugi::xml_node archivesNode = projectNode->child("archives");
		for ( pugi::xml_node archiveNode = archivesNode.child("archive"); archiveNode; archiveNode = archiveNode.next_sibling("archive") )
		{
			String archiveType = archiveNode.attribute("archive_type").as_string();
			String archiveValue = archiveNode.attribute("archive_value").as_string();
			if ( archiveType == "filesystem" )
				PathUtil::FormatPath(archiveValue, true);

			if ( !isArchiveExist(archiveType, archiveValue) )
				m_archives.push_back(ArchiveItem(archiveType, archiveValue));
			else
				EchoLogError("Archive %s has existed", archiveValue.c_str());
		}
	}

	// 保存存档
	void ProjectSettings::saveArchives(pugi::xml_document& doc, pugi::xml_node* projectNode)
	{
		if ( projectNode )
		{
			pugi::xml_node archivesNode = projectNode->append_child("archives");

			for ( size_t i = 0; i < m_archives.size(); i++ )
			{
				pugi::xml_node archiveNode =  archivesNode.append_child("archive");
				archiveNode.append_attribute("archive_type") = m_archives[i].m_archiveType.c_str();
				archiveNode.append_attribute("archive_value") = m_archives[i].m_archiveValue.c_str();
			}
		}
	}

	// 是否已存在
	bool ProjectSettings::isArchiveExist(const String& archiveType, const String& archiveValue)
	{
		for ( size_t i = 0; i < m_archives.size(); i++ )
		{
			if ( archiveType == m_archives[i].m_archiveType && archiveValue == m_archives[i].m_archiveValue )
				return true;
		}

		return false;
	}

	// 配置引擎
	void ProjectSettings::setupResource()
	{
		//在读取Project配置文件设置的目录
		for ( size_t i = 0; i < m_archives.size(); i++ )
		{
			IO::instance()->addArchive(m_archives[i].m_archiveValue, m_archives[i].m_archiveType);
		}
	}

	//返回压缩格式对应字符串名称
	char* ProjectSettings::getCompressTypeName(TextureCompressType ctype)
	{
		char* name;
		switch ( ctype )
		{
			case DoNotCompress:		name = "DoNotCompress";	break;
			case DEFAULT:			name = "DEFAULT";		break;
			case PVRTC1_2:			name = "PVRTC1_2";		break;
			case PVRTC1_4:			name = "PVRTC1_4";		break;
			case PVRTC1_2_RGB:		name = "PVRTC1_2_RGB";	break;
			case PVRTC1_4_RGB:		name = "PVRTC1_4_RGB";	break;
			case ETC1:				name = "ETC1";			break;
			case r4g4b4a4:			name = "r4g4b4a4";		break;
			case r5g6b5:			name = "r5g6b5";		break;
			default:				name = "DEFALUT";		break;
		}
		return name;
	}

	//根据字符串名称返回压缩格式对应
	ProjectSettings::TextureCompressType ProjectSettings::getCompressTypeFormName(std::string cname)
	{
		ProjectSettings::TextureCompressType ctype;
		if ( cname == "DoNotCompress" )
		{
			ctype = DoNotCompress;
		}
		else if ( cname == "DEFAULT" )
		{
			ctype = DEFAULT;
		}
		else if ( cname == "PVRTC1_2" )
		{
			ctype = PVRTC1_2;
		}
		else if ( cname == "PVRTC1_4" )
		{
			ctype = PVRTC1_4;
		}
		else if ( cname == "PVRTC1_2_RGB" )
		{
			ctype = PVRTC1_2_RGB;
		}
		else if ( cname == "PVRTC1_4_RGB" )
		{
			ctype = PVRTC1_4_RGB;
		}
		else if ( cname == "ETC1" )
		{
			ctype = ETC1;
		}
		else if ( cname == "r4g4b4a4" )
		{
			ctype = r4g4b4a4;
		}
		else if ( cname == "r5g6b5" )
		{
			ctype = r5g6b5;
		}
		else
		{
			ctype = DEFAULT;
		}
		return ctype;

	}

}