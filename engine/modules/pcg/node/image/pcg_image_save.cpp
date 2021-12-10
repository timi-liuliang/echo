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
		CLASS_BIND_METHOD(PCGImageSave, getPathName);
		CLASS_BIND_METHOD(PCGImageSave, setPathName);
		CLASS_BIND_METHOD(PCGImageSave, getFormat);
		CLASS_BIND_METHOD(PCGImageSave, setFormat);

		CLASS_REGISTER_PROPERTY(PCGImageSave, "PathName", Variant::Type::ResourcePath, getPathName, setPathName);
		CLASS_REGISTER_PROPERTY_HINT(PCGImageSave, "PathName", PropertyHintType::ResourceBehavior, "save");
		CLASS_REGISTER_PROPERTY(PCGImageSave, "Format", Variant::Type::StringOption, getFormat, setFormat);
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

	void PCGImageSave::setFormat(const StringOption& format)
	{
		m_format.setValue(format);
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

				if (m_format.getValue() == "PF_R8_UINT")
				{
					vector<ui8>::type pixels(image->getWidth() * image->getHeight());

					i32 idx = 0;
					for (const Color& color : image->getColors())
					{
						pixels[idx++] = color.r * 255.99f;
					}

					i32 pixelBytes = PixelUtil::GetPixelSize(PixelFormat::PF_R8_UINT);
					stbi_write_png(fullPath.c_str(), image->getWidth(), image->getHeight(), 1, pixels.data(), image->getWidth() * pixelBytes);
				}
				else if (m_format.getValue() == "PF_R16_UINT")
				{
					// stbi don't support write 16 bit depth image, so we use libpng directly
					// https://github.com/nothings/stb/issues/605

					// libpng examples 
					// https://gist.github.com/jeroen/10eb17a9fb0e5799b772

					i32 bit_depth = sizeof(png_uint_16) * 8;
					i32 pixelSize = 1;

					png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
					if (png)
					{
						png_infop info = png_create_info_struct(png);
						setjmp(png_jmpbuf(png));

						FILE* fp = fopen(fullPath.c_str(), "wb");
						png_init_io(png, fp);

						png_set_IHDR(
							png, 
							info, 
							image->getWidth(), 
							image->getHeight(), 
							bit_depth,
							PNG_COLOR_TYPE_GRAY,
							PNG_INTERLACE_NONE,
							PNG_COMPRESSION_TYPE_DEFAULT,
							PNG_FILTER_TYPE_DEFAULT);

						png_write_info(png, info);
						png_set_swap(png);

						png_uint_16** row_pointers = (png_uint_16**)png_malloc(png, image->getHeight() * sizeof(png_uint_16*));
						for (i32 y = 0; y < image->getHeight(); y++)
						{
							png_uint_16* row = (png_uint_16*)png_malloc(png, sizeof(png_uint_16) * image->getWidth() * pixelSize);					
							row_pointers[y] = row;

							for (i32 x = 0; x < image->getWidth(); x++)
							{
								const Color& color = image->getValue(x, y);
								ui16 finalValue = ui16(Math::Clamp(color.r * 65535.f, 0.f, 65535.f));
								row[x] = finalValue;
							}
						}

						png_write_image(png, (png_bytepp)row_pointers);
						png_write_end(png, nullptr);

						for (i32 y = 0; y < image->getHeight(); y++) 
						{
							png_free(png, row_pointers[y]);
						}

						png_free(png, row_pointers);
						png_destroy_write_struct(&png, &info);

						fclose(fp);
					}
				}
				else if (m_format.getValue() == "PF_RGB8_UINT")
				{

				}
				else if (m_format.getValue() == "PF_RGBA8_UINT")
				{

				}
			}
		}
	}
}

