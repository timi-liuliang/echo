#include "live2d_cubism.h"

extern "C"
{
	#include "thirdparty\live2d\Cubism31SdkNative-EAP5\Core\include\Live2DCubismCore.h"
}

namespace Echo
{
	Live2dCubism::Live2dCubism()
		: m_model(nullptr)
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
	{

	}

	Live2dCubism::~Live2dCubism()
	{

	}

	// update per frame
	void Live2dCubism::update()
	{
		if (m_model)
		{
			csmUpdateModel((csmModel*)m_model);
		}
	}
}