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

		// catergory
		virtual String getCategory() const override { return "Image"; }

		// Format
		void setOutputFormat();

		// Path
		const ResourcePath& getPathName() const { return m_pathName; }
		void setPathName(const ResourcePath& pathName);

		// polygon mode
		const StringOption& getFormat() const { return m_format; }
		void setFormat(const StringOption& format);

		// Run
		virtual void run() override;

	public:
		// Save
		void save(PCGImagePtr image);

	protected:
		StringOption	m_format = StringOption("PF_R8_UINT", { "PF_R8_UINT","PF_R16_UINT" });
		ResourcePath	m_pathName = ResourcePath("", ".png");
	};
}
