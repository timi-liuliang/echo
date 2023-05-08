#include "file_importer.h"
#include "engine/core/editor/editor.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/mesh/mesh.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	FileImporter::FileImporter()
	{
	}

	FileImporter::~FileImporter()
	{

	}

	void FileImporter::bindMethods()
	{

	}

	void FileImporter::run(const char* targetFolder)
	{
		if (IO::instance()->convertFullPathToResPath(targetFolder, m_targetFoler))
		{
			QStringList resFiles = QFileDialog::getOpenFileNames(nullptr, "Import Files", "", "*.*", nullptr, QFileDialog::DontUseNativeDialog);
			for (const QString& qFile : resFiles)
			{
				String srcFile = qFile.toStdString().c_str();
				if (!srcFile.empty())
				{
					String fileName = PathUtil::GetPureFilename(srcFile);
					String destFile = IO::instance()->convertResPathToFullPath(m_targetFoler + fileName);

					PathUtil::CopyFilePath(srcFile, destFile, true);
				}
			}
		}
	}
}
#endif
