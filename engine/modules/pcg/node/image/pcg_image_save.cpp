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

					i32 depth = 16;
					i32 pixelSize = 1;

					png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
					if (png_ptr)
					{
						png_infop info_ptr = png_create_info_struct(png_ptr);
						setjmp(png_jmpbuf(png_ptr));

						png_set_IHDR(
							png_ptr, 
							info_ptr, 
							image->getWidth(), 
							image->getHeight(), 
							depth,
							PNG_COLOR_TYPE_GRAY,
							PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_DEFAULT,
							PNG_FILTER_TYPE_DEFAULT);

						png_uint_16** row_pointers = (png_uint_16**)png_malloc(png_ptr, image->getHeight() * sizeof(png_uint_16*));
						for (i32 y = 0; y < image->getHeight(); y++)
						{
							png_uint_16* row = (png_uint_16*)png_malloc(png_ptr, sizeof(png_uint_16) * image->getWidth() * pixelSize);					
							row_pointers[y] = row;

							for (i32 x = 0; x < image->getWidth(); x++)
							{
								const Color& color = image->getValue(x, y);
								*row++ = ui16(color.r * 65535.99f);
							}
						}

						FILE* fp = fopen(fullPath.c_str(), "wb");
						png_init_io(png_ptr, fp);
						png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
						png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

						for (i32 y = 0; y < image->getHeight(); y++) 
						{
							png_free(png_ptr, row_pointers[y]);
						}

						png_free(png_ptr, row_pointers);
						png_destroy_write_struct(&png_ptr, &info_ptr);

						fclose(fp);
					}
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

