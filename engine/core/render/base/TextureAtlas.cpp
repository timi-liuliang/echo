#include "TextureAtlas.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
    TextureAtlas::TextureAtlas()
    {
    }

    TextureAtlas::TextureAtlas(const ResourcePath& path)
        : Res( path)
    {

    }
    
    TextureAtlas::~TextureAtlas()
    {

    }

    void TextureAtlas::bindMethods()
    {

    }

	void TextureAtlas::addAtla(const String& name, const Vector4& viewPort)
	{
		Atla atla;
		atla.m_name = name;
		atla.m_viewPort = viewPort;

		m_atlas.push_back(atla);
	}

	void TextureAtlas::clear()
	{
		m_atlas.clear();
	}

    Res* TextureAtlas::load(const ResourcePath& path)
    {
        MemoryReader reader(path.getPath());
        if (reader.getSize())
        {
            TextureAtlas* res = EchoNew(TextureAtlas(path));

			pugi::xml_document doc;
			if (doc.load_buffer(reader.getData<char*>(), reader.getSize()))
			{
				pugi::xml_node root = doc.child("atlas");
				for (pugi::xml_node child = root.child("atla"); child; child = child.next_sibling("atla"))
				{
					String name = child.attribute("name").as_string();
					Vector4 viewPort = StringUtil::ParseVec4(child.attribute("viewport").as_string());
					res->addAtla(name, viewPort);
				}
			}

            return res;
        }

        return nullptr;
    }

    void TextureAtlas::save()
    {
		String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());

		pugi::xml_document doc;

		// declaration
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		// root node
		pugi::xml_node root = doc.append_child("atlas");
		
		// all atlas
		for (const Atla& atla : m_atlas)
		{
			pugi::xml_node atlaNode = root.append_child("atla");
			atlaNode.append_attribute("name").set_value(atla.m_name.c_str());
			atlaNode.append_attribute("viewport").set_value(StringUtil::ToString(atla.m_viewPort).c_str());
		}

		doc.save_file(fullPath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }
}
