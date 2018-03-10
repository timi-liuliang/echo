 #include "Engine/modules/Model/SkinnedMesh.h"
 #include "engine/core/Util/LogManager.h"
 #include "engine/core/Util/PathUtil.h"
 #include "Engine/core/Render/TextureResManager.h"
 #include "Render/Renderer.h"
 #include "engine/core/resource/DataStream.h"
 #include "Render/PixelFormat.h"
 #include "Render/GPUBuffer.h"
 
 namespace Echo
 {
 	// 构造函数
 	SkinnedMesh::SkinnedMesh(const String& name)
 		: Mesh(name)
 	{
 	}
 
	// 析构函数
 	SkinnedMesh::~SkinnedMesh()
 	{
 	}
 
	// 加载
	bool SkinnedMesh::prepareImpl(DataStream* stream)
 	{
		return prepareImplInteral(stream, true);
 	}
 }