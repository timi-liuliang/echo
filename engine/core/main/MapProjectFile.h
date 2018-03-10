#ifndef _ECHO_MAPMapProjectFile_H_
#define _ECHO_MAPMapProjectFile_H_

#include "Engine/Core.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace rapidxml;

namespace Echo
{
	/**
	 * 项目文件
	 */
	class MapProjectFile
	{
		typedef std::map<std::string, std::string>  BlocksProps;
		typedef std::map<std::string, BlocksProps>  BlocksPropMap;
	public:
		// 存档
		struct BlockItem
		{
			String		id;
			String		blocktype;		// 
			String		blockname;		// 
			String		lineto;		//

			BlockItem()
			{}
		};

	public:
		MapProjectFile();
		~MapProjectFile();

		// 加载
		void load( const char* fileName);

		// 保存
		void save( const char* fileName);


	public:
		// blocks信息
		vector<BlockItem>::type& getBlocks(){return m_blocks;}

		// blocks信息
		 void setBlocks(vector<BlockItem>::type& info){m_blocks = info;}

		 // blocks属性信息
		 BlocksPropMap& getBlockProps(){return m_props;}

		 // blocks属性信息
		 void setBlockProps(BlocksPropMap& info){m_props = info;}
	private:
		// 从xml中读取blocks信息
		void loadBlocks(xml_node<>* projectNode);

		// 将场景中的blocks保存进xml
		void saveBlocks(xml_document<>& doc, xml_node<>* projectNode);

		// 从xml中读取blocks属性信息
		void loadBlockProps(xml_node<>* projectNode);

		// 将场景中的blocks属性保存进xml
		void saveBlockProps(xml_document<>& doc, xml_node<>* projectNode);

	private:
		vector<BlockItem>::type		m_blocks;		// 存储各个窗口类型以及关联关系
		BlocksPropMap				m_props;		// 存储各个窗口的独立属性
	};
}

#endif