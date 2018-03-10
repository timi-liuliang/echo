#include "ProjectFile.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/resource/ResourceGroupManager.h"
#include <ostream>

namespace Echo
{
	// 构造函数
	ProjectFile::ProjectFile()
	{
		m_uiRootPath = "../media/ui";
		m_defaultMaterial = "general.mt";
	}

	// 析构函数
	ProjectFile::~ProjectFile()
	{

	}

	// 加载
	void ProjectFile::load(const char* pathName)
	{
		try
		{
			// 记录路径，名称，全路径
			m_path = Echo::PathUtil::GetFileDirPath(pathName);
			m_name = Echo::PathUtil::GetLastPathName(pathName);
			m_pathName = pathName;

			// 解析项目文件
			file<> fdoc(m_pathName.c_str());
			xml_document<> doc;
			doc.parse<0>(fdoc.data());

			// 项目结点
			xml_node<>* projectNode = doc.first_node("project");
			if ( projectNode )
			{
				loadArchives(projectNode);
				loadTextureCompreses(projectNode);
				loadUIPath(projectNode);

				// get default Material Name
				xml_node<>* materialNode = projectNode->first_node("defaultMaterial");
				if (materialNode)
				{
					xml_attribute<>* pName = materialNode->first_attribute();
					m_defaultMaterial = pName->value();
				}
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
	void ProjectFile::save(const char* fileName)
	{
		if ( !fileName )
			fileName = m_pathName.c_str();

		xml_document<> doc;
		xml_node<>* xmlnode = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0' encoding='utf-8'"));
		xml_node<>* rootnode = doc.allocate_node(node_element, "project");

		doc.append_node(xmlnode);
		doc.append_node(rootnode);

		// 保存Archive属性
		saveArchives(doc, rootnode);

		// 保存压缩信息
		saveTextureCompreses(doc, rootnode);

		// 写文件
		std::ofstream out(fileName);
		out << doc;
	}

	// 加载ui配置
	void ProjectFile::loadUIPath(xml_node<>* projectNode)
	{
		try
		{
			xml_node<>* uiNode = projectNode->first_node("ui");
			if ( uiNode )
			{
				xml_node<>* rootNode = uiNode->first_node("uiroot");
				if ( rootNode )
				{
					m_uiRootPath = rootNode->first_attribute("root_value")->value();
				}
			}
		}
		catch ( ... )
		{
			EchoLogError("Load ui root path value failed");
		}

	}

	// 加载存档配置
	void ProjectFile::loadArchives(xml_node<>* projectNode)
	{
		m_archives.clear();

		xml_node<>* archivesNode = projectNode->first_node("archives");
		for ( xml_node<>* archiveNode = archivesNode->first_node("archive"); archiveNode; archiveNode = archiveNode->next_sibling("archive") )
		{
			String archiveType = archiveNode->first_attribute("archive_type")->value();
			String archiveValue = archiveNode->first_attribute("archive_value")->value();
			if ( archiveType == "filesystem" )
				PathUtil::FormatPath(archiveValue, true);

			if ( !isArchiveExist(archiveType, archiveValue) )
				m_archives.push_back(ArchiveItem(archiveType, archiveValue));
			else
				EchoLogError("Archive %s has existed", archiveValue.c_str());
		}
	}

	// 保存存档
	void ProjectFile::saveArchives(xml_document<>& doc, xml_node<>* projectNode)
	{
		if ( projectNode )
		{
			xml_node<>* archivesNode = doc.allocate_node(node_element, "archives");
			projectNode->append_node(archivesNode);

			for ( size_t i = 0; i < m_archives.size(); i++ )
			{
				xml_node<>* archiveNode = doc.allocate_node(node_element, "archive");
				xml_attribute<>* archiveType = doc.allocate_attribute("archive_type", m_archives[i].m_archiveType.c_str());
				xml_attribute<>* archiveValue = doc.allocate_attribute("archive_value", m_archives[i].m_archiveValue.c_str());

				archiveNode->append_attribute(archiveType);
				archiveNode->append_attribute(archiveValue);
				archivesNode->append_node(archiveNode);
			}
		}
	}

	// 是否已存在
	bool ProjectFile::isArchiveExist(const String& archiveType, const String& archiveValue)
	{
		for ( size_t i = 0; i < m_archives.size(); i++ )
		{
			if ( archiveType == m_archives[i].m_archiveType && archiveValue == m_archives[i].m_archiveValue )
				return true;
		}

		return false;
	}

	// 配置引擎
	void ProjectFile::setupResource()
	{
		//在读取Project配置文件设置的目录
		for ( size_t i = 0; i < m_archives.size(); i++ )
		{
			ResourceGroupManager::instance()->addArchive(m_archives[i].m_archiveValue, m_archives[i].m_archiveType);
		}
	}

	// 读取纹理压缩信息
	void ProjectFile::loadTextureCompreses(xml_node<>* projectNode)
	{
		m_TextureCompreses.clear();
		xml_node<>* textureCompresesNode = projectNode->first_node("textureCompreses");
		if ( textureCompresesNode )
		{
			xml_node<>* textureNode = textureCompresesNode->first_node("texture");
			while ( textureNode )
			{
				String name = textureNode->first_attribute("name")->value();
				char* str = textureNode->first_attribute("cios")->value();
				char* str1 = textureNode->first_attribute("candroid")->value();
				Real _scale;
				if (textureNode->first_attribute("scale"))
				{
					_scale = StringUtil::ParseReal(textureNode->first_attribute("scale")->value());
				}
				
				TextureCompressType cios = getCompressTypeFormName(str);
				TextureCompressType candroid = getCompressTypeFormName(str1);
				m_TextureCompreses.push_back(TextureCompressItem(name, cios, candroid,_scale));

				textureNode = textureNode->next_sibling("texture");
			}
		}
	}

	// 保存纹理压缩信息
	void ProjectFile::saveTextureCompreses(xml_document<>& doc, xml_node<>* projectNode)
	{
		if ( projectNode )
		{
			xml_node<>* textureCompresesNode = doc.allocate_node(node_element, "textureCompreses");
			projectNode->append_node(textureCompresesNode);

			for ( size_t i = 0; i < m_TextureCompreses.size(); i++ )
			{
				xml_node<>* textureNode = doc.allocate_node(node_element, "texture");
				xml_attribute<>* name = doc.allocate_attribute("name", m_TextureCompreses[i].m_name.c_str());
				char* cTypeName = getCompressTypeName(m_TextureCompreses[i].m_iosCType);
				xml_attribute<>* cios = doc.allocate_attribute("cios", cTypeName);
				cTypeName = getCompressTypeName(m_TextureCompreses[i].m_androidCType);
				xml_attribute<>* candroid = doc.allocate_attribute("candroid", cTypeName);
				String _scaleStr = StringUtil::ToString(m_TextureCompreses[i].m_scale);
				xml_attribute<>* _scale = doc.allocate_attribute("scale", doc.allocate_string(_scaleStr.c_str()));

				textureNode->append_attribute(_scale);
				textureNode->append_attribute(cios);
				textureNode->append_attribute(candroid);
				textureNode->append_attribute(name);
				textureCompresesNode->append_node(textureNode);
			}
		}
	}

	//返回压缩格式对应字符串名称
	char* ProjectFile::getCompressTypeName(TextureCompressType ctype)
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
	ProjectFile::TextureCompressType ProjectFile::getCompressTypeFormName(std::string cname)
	{
		ProjectFile::TextureCompressType ctype;
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