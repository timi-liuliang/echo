#include "TextureAtlas.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include "engine/core/render/base/Texture.h"
#include "engine/core/util/PathUtil.h"

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
		CLASS_BIND_METHOD(TextureAtlas, getTextureRes, DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(TextureAtlas, setTextureRes, DEF_METHOD("setTextureRes"));

		CLASS_REGISTER_PROPERTY(TextureAtlas, "Texture", Variant::Type::ResourcePath, "getTextureRes", "setTextureRes");
    }

	void TextureAtlas::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			m_texture = ECHO_DOWN_CAST<Texture*>(Res::get(path.getPath()));
		}
	}

	void TextureAtlas::addAtla(const String& name, const Vector4& viewPort)
	{
		Atla atla;
		atla.m_name = name;
		atla.m_viewPort = viewPort;

		m_atlas.push_back(atla);
	}

	void TextureAtlas::removeAtla(const String& name)
	{
		for (vector<Atla>::type::iterator it = m_atlas.begin(); it != m_atlas.end(); it++)
		{
			if (it->m_name == name)
			{
				m_atlas.erase(it);
				break;
			}
		}
	}

	bool TextureAtlas::getViewport(const String& name, Vector4& viewPort)
	{
		for (Atla& atla : m_atlas)
		{
			if (atla.m_name == name)
			{
				viewPort = atla.m_viewPort;
				return true;
			}
		}

		return false;
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
				res->setTextureRes(String(root.attribute("texture").as_string()));

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
		root.append_attribute("texture").set_value(m_textureRes.getPath().c_str());
		
		// all atlas
		for (const Atla& atla : m_atlas)
		{
			pugi::xml_node atlaNode = root.append_child("atla");
			atlaNode.append_attribute("name").set_value(atla.m_name.c_str());
			atlaNode.append_attribute("viewport").set_value(StringUtil::ToString(atla.m_viewPort).c_str());
		}

		doc.save_file(fullPath.c_str(), "\t", 1U, pugi::encoding_utf8);
    }

	void TextureAtlas::enumFilesInDir(StringArray& ret, const String& rootPath, bool bIncDir, bool bIncSubDirs, bool isAbsPath)
	{
		ret.clear();

		for (Atla& atla : m_atlas)
		{
			String path = (isAbsPath ? PathUtil::GetRenameExtFile(getPath() + "/", "") : "") + atla.m_name + ".atla";
			ret.push_back(path);
		}
	}
}
