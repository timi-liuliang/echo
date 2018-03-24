#include "FIleSystemWatcher.h"
#include <engine/core/Util/PathUtil.h>
#include <Engine/core/Render/TextureResManager.h>
#include <Engine/core/main/Root.h>
#include "Studio.h"
#include <iterator>
#include "EchoEngine.h"

namespace Studio
{
	// 构造函数
	FileSystemWatcher::FileSystemWatcher()
	{
		m_fileSystemWatcher = new QFileSystemWatcher;

		QObject::connect(m_fileSystemWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onFileChanged(const QString&)));
		QObject::connect(m_fileSystemWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirChanged(const QString&)));
	}

	FileSystemWatcher::~FileSystemWatcher()
	{
	}

	// 添加监视路径(递归)
	void FileSystemWatcher::addPath(const char* _path)
	{
		Echo::String path = _path;
		if (!Echo::PathUtil::IsAbsolutePath(path))
			path = EchoRoot->getResPath() + path;

		// 所有子路径
		Echo::StringArray files;
		Echo::PathUtil::EnumFilesInDir(files, path, true, true, true);
		files.push_back(path);
		for (const Echo::String& file : files)
		{
			m_fileSystemWatcher->addPath(file.c_str());
		}

		// 记录所有目录文件信息
		for (Echo::String& file : files)
		{
			if (Echo::PathUtil::IsDir(file))
			{
				Echo::StringArray curDirfiles;
				Echo::PathUtil::EnumFilesInDir(curDirfiles, file, true, false, true);
				m_filesInfo[file] = curDirfiles;
			}
		}	
	}

	void FileSystemWatcher::addIgnoreFileName(const char* ignoreName)
	{
		m_ignoreFileNames.push_back(ignoreName);
	}

	void FileSystemWatcher::updateThumbnail(Echo::String fileExt)
	{
		// 更新缩略图
		Echo::StringArray exts = Echo::StringUtil::Split(".effect|.model|.mesh|.skin|.material|.scene|.png|.tga|.bmp|.jpg", "|");
		for (Echo::String& ext : exts)
		{
			if (ext == fileExt)
				ThumbnailMgr::update();
		}
	}

	// 文件修改
	void FileSystemWatcher::onFileChanged(const QString& file)
	{
		Echo::String filePathName = file.toStdString().c_str();
		Echo::String fileExt = Echo::PathUtil::GetFileExt(filePathName, true);
		Echo::String fileName = Echo::PathUtil::GetPureFilename(filePathName);
		Echo::StringUtil::LowerCase(fileName);
		Echo::StringUtil::LowerCase(fileExt);

		for (size_t i = 0; i < m_ignoreFileNames.size(); ++i)
		{
			if (fileName.find(m_ignoreFileNames[i]) != Echo::String::npos)
			{
				return;
			}
		}

		// 如果是删除文件信号
		std::ifstream origStream;
		origStream.open(filePathName.c_str(), std::ios::in | std::ios::binary);
		if (origStream.fail())
		{
			return;
		}

		if (fileExt == ".tga" || fileExt == ".bmp" || fileExt == ".png" || fileExt == ".jpg")
		{
			Echo::Resource* resource = EchoTextureResManager->getByName(fileName);
			if (resource)
				resource->reload();
		}

		// c++热加载
		emit Signal_fileChanged(file);

		updateThumbnail(fileExt);
	}

	// 文件目录修改
	void FileSystemWatcher::onDirChanged(const QString& dir)
	{
		Echo::String fileDir = dir.toStdString().c_str();
		if (!fileDir.empty())
		{
			Echo::StringArray newFiles;
			Echo::PathUtil::EnumFilesInDir(newFiles, fileDir, true, false, true);

			Echo::StringArray oldFiles = m_filesInfo[dir.toStdString().c_str()];

			// 求差集
			Echo::StringArray addedFiles; /*addedFiles.resize(newFiles.size());*/
			Echo::StringArray deletedFiles; /*deletedFiles.resize(oldFiles.size());*/

			sort(newFiles.begin(), newFiles.end());
			sort(oldFiles.begin(), oldFiles.end());

			set_difference(newFiles.begin(), newFiles.end(), oldFiles.begin(), oldFiles.end(), std::inserter(addedFiles, addedFiles.end()));
			set_difference(oldFiles.begin(), oldFiles.end(), newFiles.begin(), newFiles.end(), std::inserter(deletedFiles, deletedFiles.end()));

			for (const Echo::String& file : newFiles)
			{
				Echo::String fileExt = Echo::PathUtil::GetFileExt(file.c_str(), true);
				if (fileExt == ".tga" || fileExt == ".bmp" || fileExt == ".png" || fileExt == ".jpg")
				{
					m_fileSystemWatcher->addPath(file.c_str());
				}
			}
			// 处理文件添加
			for (const Echo::String& file : addedFiles)
			{
				onFileAdd(file);
			}

			// 处理文件删除
			for (const Echo::String& file : deletedFiles)
			{
				onFileDelete(file);
			}

			m_filesInfo[dir.toStdString().c_str()] = newFiles;
		}
	}

	// 文件添加
	void FileSystemWatcher::onFileAdd(const Echo::String& file)
	{
		if (!Echo::PathUtil::IsDir(file))
		{
			//EchoResourceManager->addFile("filesystem", file);

			// 添加到项目管理
			AStudio::Instance()->getProjectMgr()->AddFileToProject(file.c_str());
		}

		m_fileSystemWatcher->addPath(file.c_str());

		onFileChanged(file.c_str());
	}

	// 文件删除
	void FileSystemWatcher::onFileDelete(const Echo::String& file)
	{
		if (!Echo::PathUtil::IsDir(file))
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(file);
			//EchoResourceManager->removeFile(fileName);
		}

		m_fileSystemWatcher->removePath(file.c_str());
		Echo::String fileExt = Echo::PathUtil::GetFileExt(file.c_str(), true);
		updateThumbnail(fileExt);
	}
}