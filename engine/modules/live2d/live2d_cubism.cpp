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
		, m_modelSize(0)
		, m_modelMemory(nullptr)
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
	{
		// set log fun
		csmSetLogFunction(csmLogFunc);

		setMoc("Res://girl/girl.moc3");
	}

	Live2dCubism::~Live2dCubism()
	{
		EchoSafeFreeAlign(m_modelMemory, csmAlignofModel);
	}

	void Live2dCubism::bindMethods()
	{
	}

	// parse paramters
	void Live2dCubism::parseParams()
	{
		int paramerCount = csmGetParameterCount(m_model);
		if (paramerCount > 0)
		{
			m_params.resize(paramerCount);
			const char** ids = csmGetParameterIds(m_model);
			const float* curValues = csmGetParameterValues(m_model);
			const float* defaultValues = csmGetParameterDefaultValues(m_model);
			const float* minValues = csmGetParameterMinimumValues(m_model);
			const float* maxValues = csmGetParameterMaximumValues(m_model);
			for (int i = 0; i < paramerCount; i++)
			{
				Paramter& param = m_params[i];
				param.m_name = ids[i];
				param.m_value = curValues[i];
				param.m_defaultValue = defaultValues[i];
				param.m_minValue = minValues[i];
				param.m_maxValue = maxValues[i];
			}
		}
	}

	// parse parts
	void Live2dCubism::parseParts()
	{
		int partCount = csmGetPartCount(m_model);
		if (partCount > 0)
		{
			m_parts.resize(partCount);
			const char** ids = csmGetPartIds(m_model);
			const float* opacities = csmGetPartOpacities(m_model);
			for (int i = 0; i < partCount; i++)
			{
				Part& part = m_parts[i];
				part.m_name = ids[i];
				part.m_opacities = opacities[i];
			}
		}
	}

	// parse drawables
	void Live2dCubism::parseDrawables()
	{
		int drawableCount = csmGetDrawableCount(m_model);
		if (drawableCount > 0)
		{
			m_drawables.resize(drawableCount);
			const char** ids = csmGetDrawableIds(m_model);
			const csmFlags* constantFlags = csmGetDrawableConstantFlags(m_model);
			const csmFlags* dynamicFlags = csmGetDrawableDynamicFlags(m_model);
			const int* textureIndices = csmGetDrawableTextureIndices(m_model);
			const int* drawOrders = csmGetDrawableDrawOrders(m_model);
			const int* renderOrders = csmGetDrawableRenderOrders(m_model);
			const float* opacities = csmGetDrawableOpacities(m_model);
			const int*  maskCounts = csmGetDrawableMaskCounts(m_model);
			const int** masks = csmGetDrawableMasks(m_model);
			const int* vertexCounts = csmGetDrawableVertexCounts( m_model);
			const csmVector2** positions = csmGetDrawableVertexPositions( m_model);
			const csmVector2** uvs = csmGetDrawableVertexUvs( m_model);
			const int* indexCounts = csmGetDrawableIndexCounts(m_model);
			const unsigned short** indices = csmGetDrawableIndices( m_model);
			for (int i = 0; i < drawableCount; i++)
			{
				// reference
				Drawable& drawable = m_drawables[i];

				drawable.m_name = ids[i];
				drawable.m_constantFlag = constantFlags[i];
				drawable.m_dynamicFlag = dynamicFlags[i];
				drawable.m_textureIndex = textureIndices[i];
				drawable.m_drawOrder = drawOrders[i];
				drawable.m_renderOrder = renderOrders[i];
				drawable.m_opacitie = opacities[i];
				ui32 maskCount = maskCounts[i];
				for (int j = 0; j < maskCount; j++)
				{
					drawable.m_masks.push_back(masks[i][j]);
				}
				
				// vertexs
				ui32 vertexCount = vertexCounts[i];
				for (int j = 0; j < vertexCount; j++)
				{
					csmVector2 pos = positions[i][j];
					csmVector2 uv = uvs[i][j];

					VertexFormat vert;
					vert.m_position = Vector3(pos.X, pos.Y, 0.f);
					vert.m_uv = Vector2(uv.X, uv.Y);

					drawable.m_vertices.push_back( vert);
				}

				// indices
				ui32 indeceCount = indexCounts[i];
				for (int j = 0; j < indeceCount; j++)
				{
					drawable.m_indices.push_back( indices[i][j]);
				}
			}
		}
	}

	// parse canvas info
	void Live2dCubism::parseCanvasInfo()
	{
		csmVector2 sizeInBytes;
		csmVector2 originInPixels;
		float	   pixelsPerUnit;
		csmReadCanvasInfo(m_model, &sizeInBytes, &originInPixels, &pixelsPerUnit);
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
				m_modelSize = csmGetSizeofModel(m_moc);
				m_modelMemory = EchoMallocAlign(m_modelSize, csmAlignofModel);
				m_model = csmInitializeModelInPlace(m_moc, m_modelMemory, m_modelSize);

				parseCanvasInfo();
				parseParams();
				parseParts();
				parseDrawables();
			}
		}
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