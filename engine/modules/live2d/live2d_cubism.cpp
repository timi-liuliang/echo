#include "live2d_cubism.h"

extern "C"
{
	#include "thirdparty\live2d\Cubism31SdkNative-EAP5\Core\include\Live2DCubismCore.h"
}

namespace Echo
{
	Live2dCubism::Live2dCubism()
		: m_moc(nullptr)
		, m_model(nullptr)
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
	{

	}

	Live2dCubism::~Live2dCubism()
	{

	}

	void Live2dCubism::bindMethods()
	{
		int a = 10;
	}

	// set moc
	void setMoc(const String& res)
	{
		//m_moc = csmReviveMocInPlace(void* address, const unsigned int size);
	}

	// set model
	void setModel(const String& model)
	{
		//m_model = csmInitializeModelInPlace(m_moc, );
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