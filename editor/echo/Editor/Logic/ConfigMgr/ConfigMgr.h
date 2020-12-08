#pragma once

#include <map>
#include <thirdparty/pugixml/pugixml.hpp>
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class ConfigMgr
	{
	public:
		ConfigMgr();
		~ConfigMgr();

		// instance
		static ConfigMgr* instance();

		// load
		bool loadCfgFile( );

		// Recent Projects operate
		bool addRecentProject( const char* fileName);
        void removeRencentProject(const Echo::String& fileName);
		void switchProjectToTop(const char* fileName);

		// save
		bool saveCfgFile( );

		// is exist
		bool isPathExist( Echo::String path);

		// output dir
		void setOutPutDir( const char* path ) { m_outPutDir = path; }
		const char* getOutPutDir( ) { return m_outPutDir.c_str(); }

		// get last open project
		Echo::String getLastOpenProjectFile();

		// value
		Echo::String getValue( const char* property);
		void setValue( const char* property, const char* value);

		// all recent projects
		void getAllRecentProject(Echo::list<Echo::String>::type& projects);

	private:
		// save
		void saveData( pugi::xml_document& doc, pugi::xml_node* projectNode );

		// load recent
		void loadRecentProject( pugi::xml_node* node);
		void loadOutPutDir( pugi::xml_node* node);
		void loadPropertys( pugi::xml_node* node);

	private:
		Echo::String						m_cfgFile;
		Echo::list<Echo::String>::type		m_recentProjects;
		Echo::String						m_outPutDir;
		size_t								m_maxRecentProjects;
		std::map<Echo::String, Echo::String>m_propertys;
	};
}
