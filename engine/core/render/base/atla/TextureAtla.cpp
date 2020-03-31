#include "TextureAtla.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include "engine/core/render/base/Texture.h"

namespace Echo
{
    TextureAtla::TextureAtla()
    {
    }

	TextureAtla::TextureAtla(const ResourcePath& path)
        : Res( path)
    {

    }
    
	TextureAtla::~TextureAtla()
    {

    }

    void TextureAtla::bindMethods()
    {

    }

    Res* TextureAtla::load(const ResourcePath& path)
    {
   //     MemoryReader reader(path.getPath());
   //     if (reader.getSize())
   //     {
			//TextureAtla* res = EchoNew(TextureAtla(path));

			//pugi::xml_document doc;
			//if (doc.load_buffer(reader.getData<char*>(), reader.getSize()))
			//{
			//	pugi::xml_node root = doc.child("atlas");
			//	res->setTextureRes(String(root.attribute("texture").as_string()));

			//	for (pugi::xml_node child = root.child("atla"); child; child = child.next_sibling("atla"))
			//	{
			//		String name = child.attribute("name").as_string();
			//		Vector4 viewPort = StringUtil::ParseVec4(child.attribute("viewport").as_string());
			//		res->addAtla(name, viewPort);
			//	}
			//}

   //         return res;
   //     }

        return nullptr;
    }

    void TextureAtla::save()
    {
		//String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());

		//pugi::xml_document doc;

		//// declaration
		//pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		//dec.append_attribute("version") = "1.0";
		//dec.append_attribute("encoding") = "utf-8";

		//// root node
		//pugi::xml_node root = doc.append_child("atlas");
		//root.append_attribute("texture").set_value(m_textureRes.getPath().c_str());
		//
		//// all atlas
		//for (const Atla& atla : m_atlas)
		//{
		//	pugi::xml_node atlaNode = root.append_child("atla");
		//	atlaNode.append_attribute("name").set_value(atla.m_name.c_str());
		//	atlaNode.append_attribute("viewport").set_value(StringUtil::ToString(atla.m_viewPort).c_str());
		//}

		//doc.save_file(fullPath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }
}
