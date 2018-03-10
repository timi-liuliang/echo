#pragma once

#include <Engine/core/main/ProjectFile.h>

namespace Studio
{
	/**
	 * 项目文件分析器 2014-7-7
	 */
	class ProjectMgr
	{
		typedef std::map<std::string, Echo::ProjectFile::TextureCompressType>  CompressMap;
	public:
		//压缩纹理输出平台
		enum PLATFORM
		{
			IOS		= 0, 
			ANDROID	= 1,
			WINDOWS = 2
		};

	public:
		ProjectMgr();
		~ProjectMgr();

		// 新建项目文件
		void NewProject( const char* projectName);

		// 打开项目文件(全路径)
		void OpenProject(const char* fullPathName);

		// 保存项目文件
		void SaveProject();

		// 加载项目文件
		bool LoadProject(const char* fileName);

		// 获取项目文件
		Echo::ProjectFile& GetProjectFile() { return m_projectFile; }

		// 获取所有文件
		Echo::map<Echo::String, Echo::String>::type& GetAllFiles() { return m_files; }

		// 获取主路径
		const Echo::String& GetRootPath() const { return m_projectFile.getPath(); }

		// 获取全路径
		Echo::String GetFullPath( const Echo::String& name);

		// 判断文件是否存在
		bool IsFileExist( const char* fileName);

		// 添加文件
		void AddFileToProject( const char* fullPath);

		void RemoveFileToProject(const char* fileName);

		// 获取项目文件路径名称
		const Echo::String& GetProjectFilePathName() { return m_projectFile.getPathName(); }

		//记录压缩格式变化的文件
		void addCompressChangeTextrue(PLATFORM platform, std::string name, Echo::ProjectFile::TextureCompressType ctype);

		// 根据图片名字获取压缩格式信息
		Echo::ProjectFile::TextureCompressItem* GetTextureCompressItem(const Echo::String& name);

		// 资源检测
		void CheckProject();

	private:
		// 检测纹理资源
		void checkTextures();

	private:
		Echo::ProjectFile							m_projectFile;		 // 项目剖析器
		Echo::map<Echo::String, Echo::String>::type	m_files;			 // 所有文件
		Echo::ProjectFile::TextureCPIVec*			m_pTextureCompreses; // 纹理压缩信息表
	};
}