#pragma once

#include "engine/modules/pcg/node/pcg_node.h"
#include "engine/modules/pcg/data/image/pcg_image.h"

namespace Echo
{
	class PCGImageSave : public PCGNode
	{
		ECHO_CLASS(PCGImageSave, PCGNode)

	public:
		PCGImageSave();
		virtual ~PCGImageSave();

		// Name
		virtual String getType() const { return "ImageSave"; }

		// Format
		void setOutputFormat();

		// Path
		void setPathName(const String& pathName);

		// Run
		virtual void run() override;

	public:
		// Save
		void save(PCGImagePtr image);

	protected:
		String			m_pathName;
	};
}
