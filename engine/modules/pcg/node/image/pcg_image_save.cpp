#include "pcg_image_save.h"
#include "engine/core/io/io.h"
#include "engine/core/util/PathUtil.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace Echo
{
	PCGImageSave::PCGImageSave()
	{
		m_inputs.push_back( EchoNew(PCGConnectPoint(this, "Image")));

		m_outputs.push_back(EchoNew(PCGConnectPoint(this, "Image", PCGConnectPoint::Output)));
	}

	PCGImageSave::~PCGImageSave()
	{

	}

	void PCGImageSave::bindMethods()
	{
		CLASS_BIND_METHOD(PCGImageSave, getPathName, DEF_METHOD("getPathName"));
		CLASS_BIND_METHOD(PCGImageSave, setPathName, DEF_METHOD("setPathName"));

		CLASS_REGISTER_PROPERTY(PCGImageSave, "PathName", Variant::Type::ResourcePath, "getPathName", "setPathName");
		CLASS_REGISTER_PROPERTY_HINT(PCGImageSave, "PathName", PropertyHintType::ResourceBehavior, "save");
	}

	void PCGImageSave::setOutputFormat()
	{

	}

	void PCGImageSave::setPathName(const ResourcePath& pathName)
	{
		if(m_pathName.setPath(pathName.getPath()))
		{
			m_dirtyFlag = true;
		}
	}

	void PCGImageSave::run()
	{
		PCGDataPtr inputData = m_inputs[0]->getData();
		if (inputData)
		{
			PCGImagePtr imageData = dynamic_cast<PCGImage*>(inputData.ptr());
			save(imageData);
		}
	}

	// http://chanhaeng.blogspot.com/2018/12/how-to-use-stbimagewrite.html
	void PCGImageSave::save(PCGImagePtr image)
	{
		if (image)
		{
			i32 idx = 0;
			vector<ui8>::type pixels(image->getWidth()*image->getHeight());

			for (const Color& color : image->getColors())
			{
				pixels[idx++] = color.r * 255.99f;
			}

			if (!m_pathName.isEmpty())
			{
				String fullPath = IO::instance()->convertResPathToFullPath(m_pathName.getPath());
				String pathDir = PathUtil::GetFileDirPath(fullPath);
				if (!PathUtil::IsDirExist(pathDir))
					PathUtil::CreateDir(pathDir);

				stbi_write_png(fullPath.c_str(), image->getWidth(), image->getHeight(), 1, pixels.data(), image->getWidth() * 1);
			}
		}
	}
}

