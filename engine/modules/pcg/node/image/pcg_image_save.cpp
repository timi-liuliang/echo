#include "pcg_image_save.h"

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

	}

	void PCGImageSave::save(PCGImagePtr Image)
	{

	}
}

