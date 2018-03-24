#include "live2d_cubism.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	static void csmLogFunc(const char* message)
	{
		EchoLogError( message);
	}


	Live2dCubism::Live2dCubism()
		: m_moc(nullptr)
		, m_model(nullptr)
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
	{
		// set log fun
		csmSetLogFunction(csmLogFunc);

		setMoc("Res://girl/girl.moc3");
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
		MemoryReaderAlign memReader( res, csmAlignofMoc);
		if (memReader.getSize())
		{
			m_moc = csmReviveMocInPlace(memReader.getData<void*>(), memReader.getSize());
			if ( m_moc)
			{
				ui32 modelSize = csmGetSizeofModel(m_moc);
				int a = 10;
			}
		}
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