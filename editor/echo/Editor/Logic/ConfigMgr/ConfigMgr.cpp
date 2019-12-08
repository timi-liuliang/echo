#include "ConfigMgr.h"
#include <engine/core/util/PathUtil.h>
#include <ostream>

namespace Studio
{
	static ConfigMgr* g_instance = nullptr;

	ConfigMgr::ConfigMgr()
		: m_outPutDir( "" )
		, m_maxRecentProjects( 10)
	{
		m_cfgFile = Echo::PathUtil::GetCurrentDir() + "/cache/echo.cache";
		g_instance = this;
	}

	ConfigMgr::~ConfigMgr()
	{
		saveCfgFile();
	}

	// instance
	ConfigMgr* ConfigMgr::instance()
	{
		return g_instance;
	}

	bool ConfigMgr::isFileExit( )
	{
		FILE* pFile = fopen( m_cfgFile.c_str(), "rb" );
		if ( NULL == pFile )
		{
			// create dir
			Echo::String dir = Echo::PathUtil::GetFileDirPath( m_cfgFile);
			if( !Echo::PathUtil::IsDirExist( dir))
				Echo::PathUtil::CreateDir( dir);

			// open file
			pFile = fopen( m_cfgFile.c_str(), "wb" );
		}

		fclose( pFile );

		return true;
	}

	bool ConfigMgr::loadCfgFile( )
	{
		m_recentProjects.clear();
		  
		isFileExit();
		try
		{	
			pugi::xml_document doc;
			doc.load_file(m_cfgFile.c_str());

			// root节点
			pugi::xml_node projectNode = doc.child( "project" );
			if( projectNode)
			{
				loadRecentProject( &projectNode );
				loadOutPutDir( &projectNode );
				loadPropertys( &projectNode);
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

	bool ConfigMgr::saveCfgFile( )
	{
		pugi::xml_document doc;
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		pugi::xml_node rootnode= doc.append_child("project" );

		saveData( doc, &rootnode);

		doc.save_file(m_cfgFile.c_str(), "\t", 1U, pugi::encoding_utf8);

		return true;
	}

	void ConfigMgr::switchProjectToTop(const char* fileName)
	{		
		for (Echo::list<Echo::String>::iterator iter = m_recentProjects.begin(); iter != m_recentProjects.end(); ++iter)
		{
			if ((*iter) == fileName)
			{
				m_recentProjects.erase(iter);
				m_recentProjects.push_front(fileName);

				return;
			}
		}
	}

	bool ConfigMgr::addRecentProject( const char* fileName )
	{
		if ( isPathExist(fileName) )
		{
			switchProjectToTop(fileName);
			saveCfgFile();

			return true;
		}

		size_t size = m_recentProjects.size();
		if ( size >= m_maxRecentProjects )
		{
			m_recentProjects.pop_back();
		}

		m_recentProjects.push_front(fileName);
		saveCfgFile();

		return true;
	}

    void ConfigMgr::removeRencentProject(const Echo::String& fileName)
    {
        m_recentProjects.remove(fileName);
        
        saveCfgFile();
    }

	Echo::String ConfigMgr::getLastOpenProjectFile()
	{
		if( m_recentProjects.size())
		{
			Echo::list<Echo::String>::type::reverse_iterator it = m_recentProjects.rbegin();

			return *it;
		}

		return Echo::String();
	}

	bool ConfigMgr::isPathExist( Echo::String path )
	{
		Echo::list<Echo::String>::iterator iter = m_recentProjects.begin();
		for ( ; iter != m_recentProjects.end(); ++iter )
		{
			if ( (*iter) == path )
			{
				return true;
			}
		}

		return false;
	}

	void ConfigMgr::saveData( pugi::xml_document& doc, pugi::xml_node* projectNode)
	{
		if( projectNode)
		{
			pugi::xml_node recentNodes = projectNode->append_child("recentProjects" );
			pugi::xml_node outputDir = projectNode->append_child("outputDir" );
			pugi::xml_node propertys = projectNode->append_child("propertys");

			Echo::list<Echo::String>::iterator Iter = m_recentProjects.begin();
			for( ; Iter != m_recentProjects.end(); ++Iter )
			{
				pugi::xml_node recentNode = recentNodes.append_child( "recentProject" );
				recentNode.append_attribute("project_value") = (*Iter).c_str();
			}

			outputDir.append_attribute("dir_value") = m_outPutDir.c_str();

			// 保存所有属性
			for( std::map<Echo::String, Echo::String>::iterator it=m_propertys.begin(); it!=m_propertys.end(); it++)
			{
				pugi::xml_node propertyNode = propertys.append_child("property");
				propertyNode.append_attribute("property") = it->first.c_str();
				propertyNode.append_attribute("value") = it->second.c_str();
			}
		}
	}

	void ConfigMgr::loadRecentProject( pugi::xml_node* node)
	{
		if ( node )
		{
			pugi::xml_node recentNodes = node->child( "recentProjects" );
			if ( !recentNodes )
				return ;

			for ( pugi::xml_node recentNode = recentNodes.child("recentProject"); recentNode; recentNode=recentNode.next_sibling("recentProject") )
			{
				Echo::String recentValue= recentNode.attribute( "project_value" ).as_string();
				if ( !isPathExist( recentValue ) )
				{
					m_recentProjects.push_back(recentValue);
				}
			}
		}
	}

	void ConfigMgr::loadOutPutDir( pugi::xml_node* node )
	{
		if ( node )
		{
			pugi::xml_node outputDirNode = node->child( "outputDir" );
			if ( outputDirNode )
				m_outPutDir = outputDirNode.attribute( "dir_value").as_string("");
		}
	}

	void ConfigMgr::loadPropertys( pugi::xml_node* node)
	{
		if ( node )
		{
			pugi::xml_node propertysNode = node->child( "propertys" );
			if( propertysNode)
			{
				for ( pugi::xml_node propertyNode = propertysNode.child("property");  propertyNode; propertyNode=propertyNode.next_sibling("property") )
				{
					Echo::String property = propertyNode.attribute("property").as_string();
					Echo::String value    = propertyNode.attribute( "value").as_string();
					
					m_propertys[property] = value;
				}
			}
		}
	}

	Echo::String ConfigMgr::getValue( const char* property)
	{
		if (!m_recentProjects.empty())
		{
			Echo::String propertyName = m_recentProjects.front() + Echo::String(":") + property;
			auto it = m_propertys.find(propertyName);
			if (it != m_propertys.end())
			{
				return it->second;
			}
		}

		return "";
	}

	void ConfigMgr::setValue( const char* property, const char* value)
	{
		if (!m_recentProjects.empty())
		{
			Echo::String propertyName = m_recentProjects.front() + Echo::String(":") + property;
			m_propertys[propertyName] = value;

			saveCfgFile();
		}
	}

	void ConfigMgr::getAllRecentProject(Echo::list<Echo::String>::type& projects)
	{
		projects.insert(projects.begin(), m_recentProjects.begin(), m_recentProjects.end());
	}
}
