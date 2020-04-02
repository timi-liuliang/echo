#pragma once

#include <engine/core/main/GameSettings.h>

namespace Studio
{
	/**
	 * ProjectManager 2014-7-7
	 */
	class ProjectMgr
	{
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

		// new
		void newProject( const char* projectName);

		// 打开项目文件(全路径)
		void OpenProject(const char* fullPathName);

		// 获取所有文件
		Echo::map<Echo::String, Echo::String>::type& GetAllFiles() { return m_files; }

		// 获取全路径
		Echo::String GetFullPath( const Echo::String& name);

		// 判断文件是否存在
		bool IsFileExist( const char* fileName);

		// 添加文件
		void AddFileToProject( const char* fullPath);

		void RemoveFileToProject(const char* fileName);

		// 资源检测
		void CheckProject();

	private:
		// 检测纹理资源
		void checkTextures();

	private:
		Echo::map<Echo::String, Echo::String>::type		m_files;			 // 所有文件
	};
}
