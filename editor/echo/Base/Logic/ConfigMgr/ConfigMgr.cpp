#include "ConfigMgr.h"
#include <engine/core/util/PathUtil.h>
#include <ostream>

namespace Studio
{
	using namespace rapidxml;
	
	// 构造函数
	ConfigMgr::ConfigMgr()
		: m_outPutDir( "" )
		, m_maxRecentProjects( 10)
	{
		m_cfgFile = Echo::PathUtil::GetCurrentDir() + "/cache/echo.cache";
	}

	// 析构函数
	ConfigMgr::~ConfigMgr()
	{
		saveCfgFile();
	}

	// 若配置文件不存在，创建之
	bool ConfigMgr::isFileExit( )
	{
		FILE* pFile = fopen( m_cfgFile.c_str(), "rb" );
		if ( NULL == pFile )
		{
			// 路径不存在先创建路径
			Echo::String dir = Echo::PathUtil::GetFileDirPath( m_cfgFile);
			if( !Echo::PathUtil::IsDirExist( dir))
				Echo::PathUtil::CreateDir( dir);

			// 不存在则直接创建
			pFile = fopen( m_cfgFile.c_str(), "wb" );
		}

		fclose( pFile );

		return true;
	}

	// 加载配置
	bool ConfigMgr::loadCfgFile( )
	{
		m_fileName.clear();
		  
		isFileExit();
		try
		{	
			file<> fdoc( m_cfgFile.c_str());
			xml_document<> doc;
			doc.parse<0>( fdoc.data());

			// root节点
			xml_node<>* projectNode = doc.first_node( "project" );
			if( projectNode)
			{
				loadRecentProject( projectNode );
				loadOutPutDir( projectNode );
				loadPropertys( projectNode);
			}
			else
			{
				
			}
		}
		catch ( ... )
		{
			return false;
		}

		return true;
	}

	// 写到本地文件中
	bool ConfigMgr::saveCfgFile( )
	{
		xml_document<> doc;
		xml_node<>* xmlnode = doc.allocate_node( node_pi, doc.allocate_string( "xml version='1.0' encoding='utf-8'" ) );
		xml_node<>* rootnode= doc.allocate_node( node_element, "project" );

		doc.append_node( xmlnode);
		doc.append_node( rootnode);

		saveData( doc, rootnode);

		std::ofstream out( m_cfgFile.c_str());
		out << doc;

		return true;
	}

	// 使某项目切换到最前
	void ConfigMgr::switchProjectToTop(const char* fileName)
	{		
		for (Echo::list<Echo::String>::iterator iter = m_fileName.begin(); iter != m_fileName.end(); ++iter)
		{
			if ((*iter) == fileName)
			{
				m_fileName.erase(iter);
				m_fileName.push_front(fileName);

				return;
			}
		}
	}

	//　添加一个最近打开的工程
	bool ConfigMgr::addRecentProject( const char* fileName )
	{
		if ( isPathExist(fileName) )
		{
			switchProjectToTop(fileName);
			saveCfgFile();

			return true;
		}

		// 最多保留十个，超过则去掉最早的
		size_t size = m_fileName.size();
		if ( size >= m_maxRecentProjects )
		{
			m_fileName.pop_back();
		}

		m_fileName.push_front(fileName);
		addToMenu();
		saveCfgFile();

		return true;
	}

	// 获取最近打开的项目文件
	Echo::String ConfigMgr::getLastOpenProjectFile()
	{
		if( m_fileName.size())
		{
			Echo::list<Echo::String>::type::reverse_iterator it = m_fileName.rbegin();

			return *it;
		}

		return Echo::String();
	}

	// 判断路径是否存在
	bool ConfigMgr::isPathExist( Echo::String path )
	{
		Echo::list<Echo::String>::iterator iter = m_fileName.begin();
		for ( ; iter != m_fileName.end(); ++iter )
		{
			if ( (*iter) == path )
			{
				return true;
			}
		}

		return false;
	}

	// 保存数据
	void ConfigMgr::saveData( rapidxml::xml_document<>& doc, rapidxml::xml_node<>* projectNode )
	{
		if( projectNode)
		{
			xml_node<>* recentNodes = doc.allocate_node( node_element, "recentProjects" );
			xml_node<>* outputDir = doc.allocate_node( node_element, "outputDir" );
			xml_node<>* propertys = doc.allocate_node( node_element, "propertys");
			projectNode->append_node( recentNodes );
			projectNode->append_node( outputDir );
			projectNode->append_node( propertys);

			Echo::list<Echo::String>::iterator Iter = m_fileName.begin();
			for( ; Iter != m_fileName.end(); ++Iter )
			{
				xml_node<>* recentNode      = doc.allocate_node( node_element, "recentProject" );
				xml_attribute<>* recentValue= doc.allocate_attribute( "project_value", (*Iter).c_str() );

				recentNode->append_attribute( recentValue );
				recentNodes->append_node( recentNode );
			}

			xml_attribute<>* dirValue = doc.allocate_attribute( "dir_value", m_outPutDir.c_str() );
			outputDir->append_attribute( dirValue );

			// 保存所有属性
			for( std::map<Echo::String, Echo::String>::iterator it=m_propertys.begin(); it!=m_propertys.end(); it++)
			{
				xml_node<>* propertyNode = doc.allocate_node( node_element, "property" );
				xml_attribute<>* property= doc.allocate_attribute( "property", it->first.c_str());
				xml_attribute<>* value   = doc.allocate_attribute( "value", it->second.c_str());

				propertyNode->append_attribute( property );
				propertyNode->append_attribute( value);
				propertys->append_node( propertyNode );
			}
		}
	}

	// 从文件读取最近的工程 
	void ConfigMgr::loadRecentProject( rapidxml::xml_node<>* node )
	{
		if ( node )
		{
			xml_node<>* recentNodes = node->first_node( "recentProjects" );
			if ( !recentNodes )
			{
				return ;
			}
			for ( xml_node<>* recentNode = recentNodes->first_node("recentProject"); recentNode; recentNode=recentNode->next_sibling("recentProject") )
			{
				Echo::String recentValue= recentNode->first_attribute( "project_value" )->value();
				if ( !isPathExist( recentValue ) )
				{
					m_fileName.push_back(recentValue);
				}
			}
		}
		
		addToMenu();
	}

	// 读取输出路径
	void ConfigMgr::loadOutPutDir( rapidxml::xml_node<>* node )
	{
		if ( node )
		{
			xml_node<>* outputDirNode = node->first_node( "outputDir" );
			if ( outputDirNode )
			{
				xml_attribute<>* node = outputDirNode->first_attribute( "dir_value" );
				if ( node )
				{
					Echo::String dir = node->value();
					m_outPutDir = dir;
				}
			}
		}
	}

	// 加载属性值
	void ConfigMgr::loadPropertys( rapidxml::xml_node<>* node)
	{
		if ( node )
		{
			xml_node<>* propertysNode = node->first_node( "propertys" );
			if( propertysNode)
			{
				for ( xml_node<>* propertyNode = propertysNode->first_node("property");  propertyNode; propertyNode=propertyNode->next_sibling("property") )
				{
					Echo::String property = propertyNode->first_attribute( "property" )->value();
					Echo::String value    = propertyNode->first_attribute( "value")->value();
					
					m_propertys[property] = value;
				}
			}
		}
	}

	// 添加到菜单中
	void ConfigMgr::addToMenu()
	{
		//QMenu* pMenu = NULL;//UI_MainWindow->GetRecentMenu();
		//if ( !pMenu )
		//{
		//	return ;
		//}

		//pMenu->clear();
		//Echo::list<Echo::String>::iterator Iter = m_fileName.begin();
		//for ( ; Iter != m_fileName.end(); ++Iter )
		//{
		//	// 将目录缓存在action中，方便MainWindow获取
		//	QAction* pAction = new QAction( pMenu );
		//	pAction->setText( (*Iter).c_str() );
		//	pMenu->addAction( pAction );
		//}
	}

	// 根据名称获取属性值
	Echo::String ConfigMgr::GetValue( const char* property)
	{
		std::map<Echo::String, Echo::String>::iterator it = m_propertys.find( property);
		if( it!=m_propertys.end())
		{
			return it->second;
		}

		return "";
	}

	// 设置属性
	void ConfigMgr::SetValue( const char* property, const char* value)
	{
		m_propertys[property] = value;

		saveCfgFile();
	}

	void ConfigMgr::getAllRecentProject(Echo::list<Echo::String>::type& projects)
	{
		projects.insert(projects.begin(), m_fileName.begin(), m_fileName.end());
	}
}