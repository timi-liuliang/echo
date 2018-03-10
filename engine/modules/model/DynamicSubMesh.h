#pragma once

#include <Rapidxml/rapidxml.hpp>
#include <engine/core/Geom/Box.h>
#include <engine/core/Math/Vector3.h>
#include "SubMesh.h"
#include <engine/core/Memory/MemAllocDef.h>

namespace Echo
{
	/**
	* 动态模型子模型
	*/
	class DynamicMesh;
	class DynamicSubMesh : public SubMesh
	{
	public:
		struct VertexDefine
		{
			bool	m_isUseNormal;
			bool	m_isUseVertexColor;
			bool	m_isUseDiffuseUV;

			VertexDefine()
				: m_isUseNormal(false), m_isUseVertexColor(false), m_isUseDiffuseUV(false)
			{}
		};

	public:
		DynamicSubMesh(DynamicMesh* mesh);
		virtual ~DynamicSubMesh() {}

		// 设置定点格式
		void setVertDefine(const VertexDefine& format);

		// 设置参数
		virtual void set(const String& params){}

		// 设置值
		void set(ui32 vertCount, const Byte* vertices, int vertexStride, ui32 indicesCount, const ui16* indices, const Box& box);

	protected:
		// 更新
		virtual void update(ui32 delta);

		// 加载
		virtual bool load();

	protected:
		bool			m_isNeedUpdateDataToGPU;		// 数据是否需要上传到GPU
		DynamicMesh*	m_dynamicMesh;					// 所属模型
	};

	/**
	* 线性条带子模型
	*/
	class DynamicSubMeshLineStrip : public DynamicSubMesh
	{
	public:
		DynamicSubMeshLineStrip(DynamicMesh* mesh);
		virtual ~DynamicSubMeshLineStrip() {}

		// 根据参数
		virtual void set(const String& params);

		// 设置参数
		void set(const vector<Vector3>::type& positions, const vector<Vector2>::type* uvs, const vector<Vector2>::type* colors);
	};

	/**
	 * 球形动态子模型
	 */
	class DynamicSubMeshSphere : public DynamicSubMesh
	{
	public:
		// 顶点格式
		struct Vertex
		{
			Vector3 position;		// 位置
			Vector3 normal;			// 法线
			Vector2 uv;				// UV

			// 构造函数
			Vertex(const Vector3& p, const Vector3& n, const Vector2& t)
				: position(p), normal(n), uv(t)
			{}
		};

	public:
		DynamicSubMeshSphere(DynamicMesh* mesh);
		virtual ~DynamicSubMeshSphere();

		// 根据参数
		virtual void set(const String& params);

		// 根据参数
		void set(const Vector3& center, float radius, int latitude, int longitude);

	private:
		// 辅助函数
		void fghCircleTable(Real **sint, Real **cost, const int n, const bool halfCircle);
		void fghGenerateSphere(Real radius, int slices, int stacks, vector<Vertex>::type& buffer);
	};
}
