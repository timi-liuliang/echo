#pragma once

#include <map>
#include <Engine/Core.h>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace Studio
{
	/**
	 * 编辑器配置
	 */
	class ConfigMgr
	{
	public:
		ConfigMgr();
		~ConfigMgr();

	public:
		// 读取配置文件
		bool loadCfgFile( );

		// 添加一个最近打开的项目
		bool addRecentProject( const char* fileName);

		// 使某项目切换到最前
		void switchProjectToTop(const char* fileName);

		// 保存配置文件
		bool saveCfgFile( );

		// 文件是否存在（如果不存在则创建）
		bool isFileExit( );

		// 当前工程是否已经存在
		bool isPathExist( Echo::String path );

		// 设置输出路径
		void setOutPutDir( const char* path ) { m_outPutDir = path; }

		// 获取输出路径
		const char* getOutPutDir( ) { return m_outPutDir.c_str(); }

		// 获取最近打开的项目文件
		Echo::String getLastOpenProjectFile();

		// 根据名称获取属性值
		Echo::String GetValue( const char* property);

		// 设置属性
		void SetValue( const char* property, const char* value);

		// 获取所有最近打开的工程
		void getAllRecentProject(Echo::list<Echo::String>::type& projects);

	private:
		// 保存到文件
		void saveData( rapidxml::xml_document<>& doc, rapidxml::xml_node<>* projectNode );

		// 读取最近打开的工程
		void loadRecentProject( rapidxml::xml_node<>* node);

		// 读取资源输出路径
		void loadOutPutDir( rapidxml::xml_node<>* node);

		// 加载属性值
		void loadPropertys( rapidxml::xml_node<>* node);

		// 添加到菜单中
		void addToMenu( );

	private:
		Echo::String						m_cfgFile;			// 配置文件
		Echo::list<Echo::String>::type		m_fileName;			// 最近打开的工程
		Echo::String						m_outPutDir;		// 资源转换输出路径
		size_t								m_maxRecentProjects;// 记录最近打
		std::map<Echo::String, Echo::String>m_propertys;		// 配置
	};
}