#include <engine/core/Math/EchoMath.h>
#include <Render/Renderer.h>
#include "DynamicSubMesh.h"
#include <engine/core/Util/LogManager.h>
#include "DynamicMesh.h"

namespace Echo
{
	// 构造函数
	DynamicSubMesh::DynamicSubMesh(DynamicMesh* mesh)
		: m_isNeedUpdateDataToGPU(false)
		, m_dynamicMesh(mesh)
	{

	}

	// 设置定点格式
	void DynamicSubMesh::setVertDefine(const VertexDefine& format)
	{
		m_vertInfo.m_isUseNormal = format.m_isUseNormal;
		m_vertInfo.m_isUseVertexColor = format.m_isUseVertexColor;
		m_vertInfo.m_isUseDiffuseUV = format.m_isUseDiffuseUV;
		m_vertInfo.build();
	}

	// 设置值
	void DynamicSubMesh::set(ui32 vertCount, const Byte* vertices, int vertexStride,ui32 indicesCount, const ui16* indices, const Box& box)
	{
		// 重置
		EchoSafeFree(m_vertInfo.m_vertices);
		EchoSafeFree(m_indices);

		m_lightmapResolution = 128;		
		m_vertInfo.m_count = vertCount;

		// 分配顶点数据存储空间
		ui32 vertBuffSize = m_vertInfo.m_count * m_vertInfo.m_stride;

		// 检测数据合法性
		if (m_vertInfo.m_stride==vertexStride)
		{
			m_vertInfo.m_vertices = EchoAlloc(Byte, vertBuffSize);

			memcpy(m_vertInfo.m_vertices, vertices, vertCount * m_vertInfo.m_stride);

			// load indices
			m_idxCount = indicesCount;
			m_idxStride = sizeof(Word);

			ui32 idxBuffSize = m_idxCount * m_idxStride;
			m_indices = EchoAlloc(Byte, idxBuffSize);
			memcpy(m_indices, indices, idxBuffSize);
			m_box = box;

			m_dynamicMesh->recalcBox();

			m_isNeedUpdateDataToGPU = true;
		}
		else
		{
			EchoLogError("DynamicSubMesh::set failed, vertexFormat isn't match to the vertexStride");
		}
	}

	// 加载函数
	bool DynamicSubMesh::load()
	{
		// 顶点缓冲	
		if (!m_vertexBuffer)
		{
			Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
			m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_CPU_WRITE, vertBuff);
		}

		// 索引缓冲		
		if (!m_indexBuffer)
		{
			Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
			m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_CPU_WRITE, indexBuff);
		}


		// 更新数据
		if (m_isNeedUpdateDataToGPU)
		{
			// 顶点缓冲
			Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
			m_vertexBuffer->updateData(vertBuff);

			// 索引缓冲
			Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
			m_indexBuffer->updateData(indexBuff);

			m_isNeedUpdateDataToGPU = true;
		}

		return true;
	}

	// 更新
	void DynamicSubMesh::update(ui32 delta)
	{
		load();
	}

	// 构造函数
	DynamicSubMeshLineStrip::DynamicSubMeshLineStrip(DynamicMesh* mesh)
		: DynamicSubMesh(mesh)
	{
		m_name = "DynamicSubMeshLineStrip";

		DynamicSubMesh::VertexDefine vertDefine;
		vertDefine.m_isUseNormal = true;
		vertDefine.m_isUseDiffuseUV = true;
		setVertDefine(vertDefine);
	}

	// 根据参数
	void DynamicSubMeshLineStrip::set(const String& params)
	{
		// 参数若为空,给予默认值
		if (params.empty())
		{
			vector<Vector3>::type positions;
			vector<Vector2>::type uvs;

			for (int i = -2; i <= 2; i++)
			{
				positions.push_back(Vector3(i, -0.5, 0.f));
				positions.push_back(Vector3(i,  0.5, 0.f));
				uvs.push_back(Vector2(i, 0));
				uvs.push_back(Vector2(i, 1));
			}

			set(positions, &uvs, nullptr);
		}
	}

	// 设置参数
	void DynamicSubMeshLineStrip::set(const vector<Vector3>::type& positions, const vector<Vector2>::type* uvs, const vector<Vector2>::type* colors)
	{
		EchoAssert(positions.size() == uvs->size());

		// 索引数据
		vector<Word>::type indices;
		const int column = positions.size() / 2 - 1;
		for (ui32 i = 0; i < column; i++)
		{
			indices.push_back(i * 2);
			indices.push_back(i * 2 + 1);
			indices.push_back(i * 2 + 3);

			indices.push_back(i * 2);
			indices.push_back(i * 2 + 3);
			indices.push_back(i * 2 + 2);
		}

		// 计算法线
		vector<Vector3>::type normals; normals.resize(positions.size());
		Math::TBNComputeNormal(normals.data(), positions.data(), positions.size(), indices.data(), indices.size()/3);

		// 顶点数据
		Box box;
		vector<float>::type	vertices;
		for (size_t i = 0; i < positions.size(); i++)
		{
			box.addPoint(positions[i]);

			// 位置
			vertices.push_back(positions[i].x);
			vertices.push_back(positions[i].y);
			vertices.push_back(positions[i].z);

			// 法线
			vertices.push_back(normals[0].x);
			vertices.push_back(normals[0].y);
			vertices.push_back(normals[0].z);

			// 纹理坐标
			if (uvs)
			{
				vertices.push_back((*uvs)[i].x);
				vertices.push_back((*uvs)[i].y);
			}
			else
			{
				vertices.push_back(0.f);
				vertices.push_back(0.f);
			}
		}

		DynamicSubMesh::set( positions.size(), (Byte*)vertices.data(), sizeof(float)*8, indices.size(), indices.data(), box);
	}

	// 构造函数
	DynamicSubMeshSphere::DynamicSubMeshSphere(DynamicMesh* mesh)
		: DynamicSubMesh(mesh)
	{
		m_name = "Sphere";

		VertexDefine define;
		define.m_isUseNormal = true;
		define.m_isUseDiffuseUV = true;
		setVertDefine(define);
	}

	// 析构函数
	DynamicSubMeshSphere::~DynamicSubMeshSphere()
	{

	}

	// 根据参数
	void DynamicSubMeshSphere::set(const String& params)
	{

	}

	void DynamicSubMeshSphere::set(const Vector3& pt, float radius, int latitude, int longitude)
	{
		vector<Vertex>::type vertices;		// 顶点数据
		vector<Word>::type	 indices;		// 索引数据
		Box box;

		box.reset();
		box.vMin = pt - Vector3(radius, radius, radius);
		box.vMax = pt + Vector3(radius, radius, radius);
		int i, j;

		/* Generate vertices and normals */
		fghGenerateSphere(radius, latitude, longitude, vertices);
		/* First, generate vertex index arrays for drawing with glDrawElements
		* All stacks, including top and bottom are covered with a triangle
		* strip.
		*/
		/* Create index vector */
		Word offset;
		indices.clear();
		/* top stack */
		for (j = 0; j < latitude; j++)
		{
			indices.push_back(j + 1);              /* 0 is top vertex, 1 is first for first stack */
			indices.push_back(0);
		}
		indices.push_back(1);                    /* repeat first slice's idx for closing off shape */
		indices.push_back(0);

		/* middle stacks: */
		/* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
		for (i = 0; i < longitude - 2; i++)
		{
			offset = 1 + i*latitude;                    /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
			for (j = 0; j < latitude; j++)
			{
				indices.push_back(offset + j + latitude);
				indices.push_back(offset + j);
			}
			indices.push_back(offset + latitude);        /* repeat first slice's idx for closing off shape */
			indices.push_back(offset);
		}

		/* bottom stack */
		offset = 1 + (longitude - 2)*latitude;               /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
		for (j = 0; j < latitude; j++)
		{
			indices.push_back(vertices.size() - 1);              /* zero based index, last element in array (bottom vertex)... */
			indices.push_back(offset + j);
		}
		indices.push_back(vertices.size() - 1);                  /* repeat first slice's idx for closing off shape */
		indices.push_back(offset);

		// 组织数据
		DynamicSubMesh::set( vertices.size(), (Byte*)vertices.data(), sizeof(float)*8,indices.size(), indices.data(), box);
	}

	// 辅助函数
	void DynamicSubMeshSphere::fghCircleTable(Real **sint, Real **cost, const int n, const bool halfCircle)
	{
		int i;

		/* Table size, the sign of n flips the circle direction */
		const int size = abs(n);

		/* Determine the angle between samples */
		const Real angle = (halfCircle ? 1 : 2)*(Real)Math::PI / (Real)((n == 0) ? 1 : n);

		/* Allocate memory for n samples, plus duplicate of first entry at the end */
		*sint = (Real*)malloc(sizeof(Real) * (size + 1));
		*cost = (Real*)malloc(sizeof(Real) * (size + 1));

		/* Bail out if memory allocation fails, fgError never returns */
		if (!(*sint) || !(*cost))
		{
			free(*sint);
			free(*cost);
			EchoLogError("Failed to allocate memory in fghCircleTable");
		}

		/* Compute cos and sin around the circle */
		(*sint)[0] = 0.0;
		(*cost)[0] = 1.0;

		for (i = 1; i < size; i++)
		{
			(*sint)[i] = (Real)sin(angle*i);
			(*cost)[i] = (Real)cos(angle*i);
		}


		if (halfCircle)
		{
			(*sint)[size] = 0.0f;  /* sin PI */
			(*cost)[size] = -1.0f;  /* cos PI */
		}
		else
		{
			/* Last sample is duplicate of the first (sin or cos of 2 PI) */
			(*sint)[size] = (*sint)[0];
			(*cost)[size] = (*cost)[0];
		}
	}

	void DynamicSubMeshSphere::fghGenerateSphere(Real radius, int slices, int stacks, vector<Vertex>::type& buffer)
	{
		int i, j;
		int idx = 0;    /* idx into vertex/normal buffer */
		Real x, y, z;

		/* Pre-computed circle */
		Real *sint1, *cost1;
		Real *sint2, *cost2;

		/* number of unique vertices */
		if (slices == 0 || stacks < 2)
		{
			/* nothing to generate */

			return;
		}
		buffer.clear();
		if (slices*(stacks - 1) + 2 > 65535)
		{
			/*
			* limit of glushort, thats 256*256 subdivisions, should be enough in practice. See note above
			*/
			EchoLogWarning("too many slices or stacks requested, indices will wrap");
		}

		/* precompute values on unit circle */
		fghCircleTable(&sint1, &cost1, -slices, false);
		fghCircleTable(&sint2, &cost2, stacks, true);

		/* top */
		buffer.push_back(Vertex(Vector3(0., 0., radius), Vector3(0., 0., 1.), Vector2::ZERO));

		idx = 3;

		/* each stack */
		for (i = 1; i < stacks; i++)
		{
			for (j = 0; j < slices; j++)
			{
				x = cost1[j] * sint2[i];
				y = sint1[j] * sint2[i];
				z = cost2[i];

				buffer.push_back(Vertex(Vector3(x*radius, y*radius, z*radius), Vector3(x, y, z), Vector2::ZERO));
			}
		}

		/* bottom */
		buffer.push_back(Vertex(Vector3(0., 0., -radius), Vector3(0, 0, -1), Vector2::ZERO));

		/* Done creating vertices, release sin and cos tables */
		free(sint1);
		free(cost1);
		free(sint2);
		free(cost2);
	}
}
