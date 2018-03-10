#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <engine/core/Memory/MemManager.h>

namespace Studio
{
	/**
	 * 文件系统监视器
	 */
	class EchoEngine;
	class FileSystemWatcher : public QObject
	{
		Q_OBJECT
		typedef Echo::map<Echo::String, Echo::StringArray>::type FileMapping;
	public:
		FileSystemWatcher();
		~FileSystemWatcher();

		// 添加监视路径(递归)
		void addPath( const char* path);
		void addIgnoreFileName(const char* ignoreName);
	
	private:
		void updateThumbnail(Echo::String fileExt);

	public: signals:
		// 数据变动
		void Signal_fileChanged(const QString& file);

	public slots:
		// 文件修改
		void onFileChanged(const QString& file);

		// 文件添加
		void onFileAdd(const Echo::String& file);

		// 文件删除
		void onFileDelete(const Echo::String& file);

		// 文件目录修改
		void onDirChanged(const QString& dir);

	public:
		QFileSystemWatcher*		m_fileSystemWatcher;
		FileMapping				m_filesInfo;
		Echo::vector<Echo::String>::type m_ignoreFileNames;
	};
}