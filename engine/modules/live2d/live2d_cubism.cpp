#include "live2d_cubism.h"

namespace Echo
{
	Live2dCubism::Live2dCubism()
		: m_moc(nullptr)
		, m_model(nullptr)
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
	{
		setMoc("Res://girl.moc");
	}

	Live2dCubism::~Live2dCubism()
	{

	}

	void Live2dCubism::bindMethods()
	{
		int a = 10;
	}

	// set moc
	void Live2dCubism::setMoc(const String& res)
	{
		MemoryReader memReader( res);

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