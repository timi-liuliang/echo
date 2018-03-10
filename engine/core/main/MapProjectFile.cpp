#include "MapProjectFile.h"
#include "Root.h"
#include <ostream>

namespace Echo
{
	// 构造函数
	MapProjectFile::MapProjectFile()
	{

	}

	// 析构函数
	MapProjectFile::~MapProjectFile()
	{

	}

	// 加载
	void MapProjectFile::load( const char* fileName)
	{
		try
		{
			file<> fdoc( fileName);
			xml_document<> doc;
			doc.parse<0>( fdoc.data());

			// 项目结点
			xml_node<>* projectNode = doc.first_node( "copy");
			if( projectNode)
			{
				loadBlocks(projectNode);
				loadBlockProps(projectNode);
			}
			else
			{
				EchoLogError( "MapProjectFile parse failed");
			}
		}
		catch ( ...)
		{
			EchoLogError( "MapProjectFile load failed");
		}
	}

	// 保存
	void MapProjectFile::save( const char* fileName)
	{
		//list<String>	saveHelper;

		xml_document<> doc;
		xml_node<>* xmlnode = doc.allocate_node( node_pi, doc.allocate_string( "xml version='1.0' encoding='utf-8'"));
		xml_node<>* rootnode= doc.allocate_node( node_element, "copy");

		doc.append_node( xmlnode);
		doc.append_node( rootnode);

		// 保存窗口关联属性
		saveBlocks(doc, rootnode);

		// 保存窗口具体属性
		saveBlockProps(doc, rootnode);

		// 写文件
		std::ofstream out( fileName);
		out << doc;
	}


	// 读取信息
	void MapProjectFile::loadBlocks( xml_node<>* projectNode )
	{
		m_blocks.clear();
		xml_node<>* blockNode = projectNode->first_node( "block");
		while(blockNode)
		{
			xml_node<>* subNode = blockNode->first_node("id");
			String id = subNode->value();
			subNode = blockNode->first_node("blocktype");
			String blocktype = subNode->value();
			subNode = blockNode->first_node("blockname");
			String blockname = subNode->value();
			subNode = blockNode->first_node("lineto");
			String lineto = subNode->value();

			BlockItem item;
			//item.id = atoi(id);
			item.id = id;
			item.blockname = blockname;
			item.blocktype = blocktype;
			item.lineto = lineto;
			m_blocks.push_back(item);

			blockNode = blockNode->next_sibling("block");
		}
	}

	// 保存信息
	void MapProjectFile::saveBlocks( xml_document<>& doc, xml_node<>* projectNode )
	{
		if( projectNode)
		{
			//xml_node<>* textureCompresesNode = doc.allocate_node( node_element, "block");
			//projectNode->append_node( textureCompresesNode);

			for( size_t i=0; i<m_blocks.size(); i++)
			{
				xml_node<>* blockNode = doc.allocate_node( node_element, "block");

				xml_node<>* subNode = doc.allocate_node( node_element, "id",m_blocks[i].id.c_str());
				blockNode->append_node( subNode);

				subNode = doc.allocate_node( node_element, "blocktype",m_blocks[i].blocktype.c_str());
				blockNode->append_node( subNode);

				subNode = doc.allocate_node( node_element, "blockname",m_blocks[i].blockname.c_str());
				blockNode->append_node( subNode);

				subNode = doc.allocate_node( node_element, "lineto",m_blocks[i].lineto.c_str());
				blockNode->append_node( subNode);

				projectNode->append_node( blockNode);
			}
		}
	}

	void MapProjectFile::loadBlockProps( xml_node<>* projectNode )
	{
		m_props.clear();
		xml_node<>* blockNode = projectNode->first_node( "props");
		while(blockNode)
		{
			BlocksProps blockProps;
			xml_node<>* subNode = blockNode->first_node();
			while(subNode)
			{
				String sname = subNode->name();
				String svalue = subNode->value();
				blockProps.insert(std::make_pair(sname,svalue));
				subNode = subNode->next_sibling();
			}
			BlocksProps::iterator it = blockProps.find("id");
			if (it != blockProps.end())
			{
				m_props.insert(std::make_pair(it->second,blockProps));
			}

			blockNode = blockNode->next_sibling("props");
		}
	}

	void MapProjectFile::saveBlockProps( xml_document<>& doc, xml_node<>* projectNode )
	{
		if( projectNode)
		{
			//xml_node<>* textureCompresesNode = doc.allocate_node( node_element, "block");
			//projectNode->append_node( textureCompresesNode);
			BlocksPropMap::iterator it = m_props.begin();
			for (;it!= m_props.end();++it)
			{
				xml_node<>* blockNode = doc.allocate_node( node_element, "props");

				BlocksProps::iterator propIter = it->second.begin();
				for (;propIter!= it->second.end();++propIter)
				{
					xml_node<>* subNode = doc.allocate_node( node_element, propIter->first.c_str(),propIter->second.c_str());
					blockNode->append_node( subNode);
				}

				projectNode->append_node( blockNode);
			}
		}
	}

}