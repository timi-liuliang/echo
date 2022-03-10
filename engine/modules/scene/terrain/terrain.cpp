#include "terrain.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/main/Engine.h"
#include "engine/modules/ui/font/font_library.h"

namespace Echo
{
    Terrain::Terrain()
    {
        setRenderType("3d");
    }
    
    Terrain::~Terrain()
    {
        clear();
    }
    
    void Terrain::bindMethods()
    {
        CLASS_BIND_METHOD(Terrain, getDataPath);
        CLASS_BIND_METHOD(Terrain, setDataPath);
        CLASS_BIND_METHOD(Terrain, getColumns);
        CLASS_BIND_METHOD(Terrain, getRows);
		CLASS_BIND_METHOD(Terrain, getHeightRange);
		CLASS_BIND_METHOD(Terrain, setHeightRange);
		CLASS_BIND_METHOD(Terrain, getGridSpacing);
		CLASS_BIND_METHOD(Terrain, setGridSpacing);
        CLASS_BIND_METHOD(Terrain, getMaterial);
        CLASS_BIND_METHOD(Terrain, setMaterial);
        
        CLASS_REGISTER_PROPERTY(Terrain, "Data", Variant::Type::ResourcePath, getDataPath, setDataPath);
		CLASS_REGISTER_PROPERTY(Terrain, "HeightRange", Variant::Type::Real, getHeightRange, setHeightRange);
		CLASS_REGISTER_PROPERTY(Terrain, "GridSpacing", Variant::Type::Int, getGridSpacing, setGridSpacing);
        CLASS_REGISTER_PROPERTY(Terrain, "Material", Variant::Type::Object, getMaterial, setMaterial);
        CLASS_REGISTER_PROPERTY_HINT(Terrain, "Material", PropertyHintType::ObjectType, "Material");
    }
    
    void Terrain::setDataPath(const ResourcePath& path)
    {
        EchoSafeDelete(m_heightmapImage, Image);
        EchoSafeDeleteContainer(m_layerImages, Image);

        if (m_dataPath.setPath(path.getPath()))
        {
            String heightmapPath = m_dataPath.getPath() + "heightmap.png";
            if (IO::instance()->isExist(heightmapPath))
            {
				m_heightmapImage = Image::loadFromFile(heightmapPath);
				if (m_heightmapImage)
				{
					m_columns = m_heightmapImage->getWidth();
					m_rows = m_heightmapImage->getHeight();

                    m_heightData.clear();
                    for (i32 column = 0; column < m_columns; column++)
                    {
                        for (i32 row = 0; row < m_rows; row++)
                        {
							Color color = m_heightmapImage->getColor(column, row, 0);
                            m_heightData.push_back(color.r);
                        }
                    }

					m_isRenderableDirty = true;
				}
            }

            for (i32 i = 0; i < 4; i++)
            {
                String layerImagePath = m_dataPath.getPath() + StringUtil::Format("layer_%i.png", i);
                if (IO::instance()->isExist(layerImagePath))
                {
					Image* layerImage = Image::loadFromFile(layerImagePath);
					m_layerImages.emplace_back(layerImage);
                }
                else
                {
                    m_layerImages.emplace_back(nullptr);
                }
            }
        }
    }

    void Terrain::setHeight(i32 minX, i32 minY, i32 width, i32 height, vector<float>::type& heightData)
    {
        m_rows = width;
        m_columns = height;
        m_heightData = heightData;

        m_isRenderableDirty = true;
    }

	void Terrain::setHeightRange(float range)
	{ 
		m_heightRange = range;

		m_isRenderableDirty = true;
	}

	void Terrain::setGridSpacing(i32 gridSpacing)
	{ 
		m_gridSpacing = Math::Clamp(gridSpacing, 1, 512);

        m_isRenderableDirty = true;
	}

    void Terrain::setMaterial( Object* material)
    {
        m_material = (Material*)material;
        
        m_isRenderableDirty = true;
    }
    
    void Terrain::buildRenderable()
    {
        if (m_isRenderableDirty && m_columns > 0 && m_rows > 0)
        {
            clearRenderable();
            
            // make sure one material is valid
            if(!m_material)
            {
                ShaderProgramPtr shader = ShaderProgram::getDefault3D({ "HAS_NORMALS" });
                
                // material
                m_material = ECHO_CREATE_RES(Material);
                m_material->setShaderPath(shader->getPath());
            }
            
            // mesh
			updateMeshBuffer();
            
			// create renderable
            m_renderable = RenderProxy::create(m_mesh, m_material, this, true);
            
            m_isRenderableDirty = false;
        }
    }
    
    void Terrain::updateInternal(float elapsedTime)
    {
        if (isNeedRender())
            buildRenderable();

        if (m_renderable)
            m_renderable->setSubmitToRenderQueue(isNeedRender());
    }
    
    void Terrain::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
    {
        if(m_columns>0 && m_rows>0)
        {
            // vertex buffer
			for (i32 row = 0; row < m_rows; row++)
            {
				for (i32 column = 0; column < m_columns; column++)
                {
                    VertexFormat vert;
                    vert.m_position = Vector3(row * m_gridSpacing, getHeight(row, column), column * m_gridSpacing);
                    vert.m_color = 0xFFFFFFFF;
                    vert.m_normal = getNormal(row, column);
                    vert.m_uv = Vector2(row, column);
                    vert.m_layerIndices = Color(0, 1, 2, 3).getABGR();

                    float weight = Math::IntervalRandom(0.f, 1.f);
                    vert.m_layerWeights = Vector4(getWeight(row, column, 0), getWeight(row, column, 1), getWeight(row, column, 2), getWeight(row, column, 3));
                    oVertices.emplace_back(vert);
                }
            }
            
            // index buffer
			i32 rowLength = m_rows - 1;
			i32 columnLength = m_columns - 1;
			for (i32 row = 0; row < rowLength; row++)
			{
				for (i32 column = 0; column < columnLength; column++)
                {
                    i32 indexLeftTop = row * m_columns + column;
                    i32 indexRightTop = indexLeftTop + 1;
                    i32 indexLeftBottom = indexLeftTop + m_columns;
                    i32 indexRightBottom = indexRightTop + m_columns;
                    
                    oIndices.emplace_back(indexLeftTop);
                    oIndices.emplace_back(indexRightBottom);
                    oIndices.emplace_back(indexRightTop);
                    oIndices.emplace_back(indexLeftTop);
                    oIndices.emplace_back(indexLeftBottom);
                    oIndices.emplace_back(indexRightBottom);
                }
            }
        }
    }
    
    void Terrain::updateMeshBuffer()
    {
		// create mesh
		if (!m_mesh) m_mesh = Mesh::create(true, true);

		// update data
        VertexArray    vertices;
        IndiceArray    indices;
        buildMeshData(vertices, indices);
        
        MeshVertexFormat define;
        define.m_isUseNormal = true;
        define.m_isUseVertexColor = true;
        define.m_isUseUV = true;
        define.m_isUseBlendingData = true;
        
        m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
        m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

        m_localAABB = m_mesh->getLocalBox();
    }
    
    void Terrain::clear()
    {
        clearRenderable();
    }
    
    void Terrain::clearRenderable()
    {
        m_renderable.reset();
        m_mesh.reset();
    }
    
    float Terrain::getHeight(i32 x, i32 z)
    {
        i32 offset = z * m_rows + x;
        return offset < m_heightData.size() ? (m_heightData[offset] * 2.f - 1.f) * m_heightRange : 0.f;
    }
    
    Vector3 Terrain::getNormal( i32 x, i32 z)
    {
        if(!m_heightData.empty())
        {
            float h0 = getHeight(   x,   z);
            float h1 = getHeight( x+1,   z) - h0;
            float h2 = getHeight(   x, z-1) - h0;
            float h3 = getHeight( x-1,   z) - h0;
            float h4 = getHeight(   x, z+1) - h0;
            
            Vector3 normal( h3 - h1, 2, h2 - h4);
            normal.normalize();
            
            return normal;
        }

        return Vector3::UNIT_Y;
    }

    float Terrain::getWeight(i32 x, i32 z, i32 index)
    {
        if (index<m_layerImages.size() && m_layerImages[index])
        {
			i32 column = Math::Clamp(x, 0, m_columns - 1);
			i32 row = Math::Clamp(z, 0, m_rows - 1);
			Color color = m_layerImages[index]->getColor(column, row, 0);
			float height = color.r;

			return height;
        }

        return 0.f;
    }
}

