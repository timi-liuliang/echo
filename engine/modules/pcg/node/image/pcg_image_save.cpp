#include "pcg_image_save.h"

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

	}

	void PCGImageSave::setOutputFormat()
	{

	}

	void PCGImageSave::setPathName(const String& pathName)
	{
		m_pathName = pathName;
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

			stbi_write_png("D:/test.png", image->getWidth(), image->getHeight(), 1, pixels.data(), image->getWidth() * 1);
		}
	}
}

