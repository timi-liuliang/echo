#pragma once

#include "engine/core/Util/StringUtil.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	/**
	 * 项目文件
	 */
	class ProjectFile
	{
	public:
		// 存档
		struct ArchiveItem
		{
			String		m_archiveType;		// 存档类目
			String		m_archiveValue;		// 存档值

			ArchiveItem( const String& type, const String& value)
				: m_archiveType( type), m_archiveValue( value)
			{
				StringUtil::LowerCase(m_archiveType);
				StringUtil::LowerCase(m_archiveValue);
			}
		};

		enum TextureCompressType
		{
			DoNotCompress	= 0,
			DEFAULT			= 1,
			PVRTC1_2		= 2,
			PVRTC1_4		= 3,
			PVRTC1_2_RGB	= 4,
			PVRTC1_4_RGB	= 5,
			ETC1			= 6,  
			r4g4b4a4		= 7,
			r5g6b5			= 8,
		};

		// 纹理压缩控制
		struct TextureCompressItem
		{
			String				   m_name;			//纹理名字
			TextureCompressType    m_iosCType;		//是否压缩成PVR（暂时用bool，以后有多种压缩格式的话改成int或枚举）
			TextureCompressType    m_androidCType;  //是否压缩成ETC
			Real				   m_scale;			//图片缩放设置

			TextureCompressItem(const String& name,TextureCompressType iosc = DEFAULT ,TextureCompressType androidc = DEFAULT,const Real& scale = -1.0f )
				: m_name(name)
				, m_iosCType(iosc)
				, m_androidCType(androidc)
				, m_scale(scale)
			{

			}

			TextureCompressItem()
				: m_name("")
				, m_iosCType(DEFAULT)
				, m_androidCType(DEFAULT)
				, m_scale(-1.0f)
			{

			}
		};
		typedef vector<TextureCompressItem>::type TextureCPIVec;

	public:
		ProjectFile();
		~ProjectFile();

		// 获取项目路径
		const String& getPath() const { return m_path; }

		// 获取项目名称
		const String& getName() const { return m_name; }

		// 获取全路径
		const String& getPathName() const { return m_pathName;  }

		// 加载
		void load( const char* fileName);

		// 保存
		void save( const char* fileName);

		// 配置引擎
		void setupResource();

	public:
		// 获取存档信息
		vector<ArchiveItem>::type& getArchives() { return m_archives; }

		// 获取纹理压缩信息
		TextureCPIVec* getTextureCompreses(){ return &m_TextureCompreses; }

		//返回压缩格式对应字符串名称
		static char* getCompressTypeName(TextureCompressType ctype);

		//根据字符串名称返回压缩格式对应
		static TextureCompressType getCompressTypeFormName(std::string cname);

	private:
		// 加载存档配置
		void loadArchives( pugi::xml_node* projectNode);

		// 保存存档
		void saveArchives( pugi::xml_document& doc, pugi::xml_node* projectNode);

		// 是否已存在
		bool isArchiveExist( const String& archiveType, const String& archiveValue);

	private:
		String								m_path;				// 项目路径
		String								m_name;				// 项目文件名
		String								m_pathName;			// 项目全路径(包含名称)
		vector<ArchiveItem>::type			m_archives;			// 存档配置
		TextureCPIVec						m_TextureCompreses; // 纹理压缩信息表
	};
}
