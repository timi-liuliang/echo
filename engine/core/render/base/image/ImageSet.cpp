#include "ImageSet.h"
#include "engine/core/io/IO.h"

namespace Echo
{
    ImageSet::ImageSet()
    {
        //EchoAssert(false);
    }

    ImageSet::ImageSet(const ResourcePath& path)
        : Res( path)
    {

    }
    
    ImageSet::~ImageSet()
    {

    }

    void ImageSet::bindMethods()
    {

    }

    Res* ImageSet::load(const ResourcePath& path)
    {
        MemoryReader reader(path.getPath());
        if (reader.getSize())
        {
            ImageSet* res = EchoNew(ImageSet(path));
            return res;
        }

        return nullptr;
    }

    void ImageSet::save()
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
