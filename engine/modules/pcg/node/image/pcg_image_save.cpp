#include "pcg_image_save.h"
#include "engine/core/io/io.h"
#include "engine/core/util/PathUtil.h"
#include <thirdparty/libpng/png.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

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
			if (!m_pathName.isEmpty())
			{
				String fullPath = IO::instance()->convertResPathToFullPath(m_pathName.getPath());
				String pathDir = PathUtil::GetFileDirPath(fullPath);
				if (!PathUtil::IsDirExist(pathDir))
					PathUtil::CreateDir(pathDir);

				if (m_format == PixelFormat::PF_R8_UINT)
				{
					vector<ui8>::type pixels(image->getWidth() * image->getHeight());

					i32 idx = 0;
					for (const Color& color : image->getColors())
					{
						pixels[idx++] = color.r * 255.99f;
					}

					i32 pixelBytes = PixelUtil::GetPixelSize(m_format);
					stbi_write_png(fullPath.c_str(), image->getWidth(), image->getHeight(), 1, pixels.data(), image->getWidth() * pixelBytes);
				}
				else if (m_format == PixelFormat::PF_R16_UINT)
				{
					// stbi don't support write 16 bit depth image, so we use libpng directly
					// https://github.com/nothings/stb/issues/605

					png_image png;
					memset(&png, 0, sizeof(png));
					png.version = PNG_IMAGE_VERSION;


					//vector<ui16>::type pixels(image->getWidth() * image->getHeight());

					//i32 idx = 0;
					//for (const Color& color : image->getColors())
					//{
					//	pixels[idx++] = ui16(color.r * 65535.99f);
					//}

					//i32 pixelBytes = PixelUtil::GetPixelSize(m_format);
					//stbi_write_png(fullPath.c_str(), image->getWidth(), image->getHeight(), 1, pixels.data(), image->getWidth() * pixelBytes);
				}
				else if (m_format == PixelFormat::PF_RGB8_UINT)
				{

				}
				else if (m_format == PixelFormat::PF_RGBA8_UINT)
				{

				}
			}
		}
	}
}

