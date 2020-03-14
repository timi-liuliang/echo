#include "TextureAtlas.h"
#include "engine/core/io/IO.h"

namespace Echo
{
    TextureAtlas::TextureAtlas()
    {
        //EchoAssert(false);
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

    Res* TextureAtlas::load(const ResourcePath& path)
    {
        MemoryReader reader(path.getPath());
        if (reader.getSize())
        {
            TextureAtlas* res = EchoNew(TextureAtlas(path));
            return res;
        }

        return nullptr;
    }

    void TextureAtlas::save()
    {
        const char* content = "Empty ImageSet";
        if (content)
        {
            String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());
            std::ofstream f(fullPath.c_str());

            f << content;

            f.flush();
            f.close();
        }
    }
}
