#pragma once

#include "engine/core/editor/importer.h"

#ifdef ECHO_EDITOR_MODE
#include "thirdparty/openfbx/ofbx.h"

namespace Echo
{
	class FileImporter : public Importer
	{
		ECHO_CLASS(FileImporter, Importer)

	public:
		FileImporter();
		virtual ~FileImporter();

		// name
		virtual const char* getName() override { return "File(*.*)"; }

		// import
		virtual void run(const char* targetFolder) override;

	private:
		String		m_targetFoler;
	};
}
#endif
