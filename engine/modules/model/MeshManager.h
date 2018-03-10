#pragma once

#include "engine/core/Resource/ResourceManager.h"
#include <functional>
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	/**
	 * 模型管理器
	 */
	class DynamicSubMesh;
	class Mesh;
	class MeshManager : public ResourceManager
	{
		__DeclareSingleton(MeshManager);

	public:
		typedef std::function<Mesh*(const String& name)> Creater;

		MeshManager();
		virtual ~MeshManager();

		// 创建模型
		Mesh* createMesh(const String& name, bool needBackUp = false);

		static void registerCustomMeshCreater(const String& name, Creater creater);
		
	protected:
		/// @copydoc ResourceManager::createImpl
		Resource* createImpl(const String& name, bool isManual);	
	};
}
