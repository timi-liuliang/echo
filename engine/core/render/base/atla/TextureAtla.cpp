#include "TextureAtla.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include "engine/core/render/base/Texture.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
	TextureAtla::TextureAtla()
	{
	}

	TextureAtla::TextureAtla(const ResourcePath& path)
		: Res(path)
	{
		String atlasPath = PathUtil::GetFileDirPath(path.getPath(), false) + ".atlas";
		m_owner = ECHO_DOWN_CAST<TextureAtlas*>(Res::get(atlasPath));

		m_atlaName = PathUtil::GetPureFilename(path.getPath(), false);
	}

	TextureAtla::~TextureAtla()
	{

	}

	void TextureAtla::bindMethods()
	{

	}

	Res* TextureAtla::load(const ResourcePath& path)
	{
		if (!path.isEmpty())
		{
			TextureAtla* res = EchoNew(TextureAtla(path));
			return res;
		}

        return nullptr;
    }

    void TextureAtla::save()
    {

    }
}
