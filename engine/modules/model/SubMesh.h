#pragma once

#include "Engine/Core.h"
#include "engine/core/render/render//RenderInput.h"

namespace Echo
{
	/** 
	 * SubMesh 2013-11-6
	 */
	class Mesh;
	class SubMesh
	{
		friend class Mesh;
		friend class DynamicMesh;
		friend class InstanceMesh;
		typedef RenderInput::VertexElementList RIVEL;

	public:
		// 顶点信息
		struct VertexInfo
		{
			ui32		m_count;				// 顶点数量
			bool		m_isUseNormal;			// 是否使用法线
			bool		m_isUseVertexColor;		// 是否使用顶点色
			bool		m_isUseDiffuseUV;		// 是否使用漫反射贴图UV 
			bool		m_isUseLightmapUV;		// 是否使用光照图UV
			bool		m_isUseBoneData;		// 是否使用骨骼数据(索引权重)
			bool		m_isUseTangentBinormal;	// 是否使用切线与副线
			ui32		m_stride;				// 顶点格式大小
			Byte		m_posOffset;			// 顶点位置数据偏移量
			Byte		m_normalOffset;			// 顶点法线数据偏移量
			Byte		m_colorOffset;			// 顶点颜色数据偏移量
			Byte		m_uv0Offset;			// 顶点纹理坐标0偏移量
			Byte		m_uv1Offset;			// 顶点纹理坐标1偏移量
			Byte		m_boneIndicesOffset;	// 顶点骨骼索引偏移量
			Byte		m_boneWeightsOffset;	// 顶点骨骼权重偏移量
			Byte		m_tangentOffset;		// 切线数据偏移量
			RIVEL		m_vertexElements;		// 顶点格式
			Byte*		m_vertices;				// 顶点数据
			vector<Vector3>::type	m_positions;// 位置数据(物理使用,不可删除,静态模型only)

			// 构造函数
			VertexInfo();

			// 计算偏移量,顶点格式
			void build();

			// 获取顶点格式大小
			ui32 getVertexStride() const;

			// 获取顶点数量
			ui32 getVertexCount() const;

			// 获取顶点数据
			Byte* getVertices() const;

			// 判断顶点格式中是否含有指定类型的数据
			bool isVertexUsage(RenderInput::VertexSemantic semantic) const;

			// 获取顶点位置数据
			Vector3& getPosition(Word index);

			// 获取顶点法线数据
			const Vector3& getNormal(Word index);

			// 获取顶点颜色数据
			Dword& getColor(Word index);

			// 获取顶点UV数据0
			const Vector2& getUV0(Word index);

			// 获取顶点UV数据1
			const Vector2& getUV1(Word index);

			// 获取切线
			Vector3& getTangent(Word index);

			// 记录位置数据
			void copyPositions();

			//状态置空
			void reset();
		};

		// 切空间数据
		struct Tangent
		{
			Vector3		m_tangent;		// 切线
			Vector3		m_binormal;		// 副线

			Tangent()
				: m_tangent( Vector3::ZERO)
				, m_binormal( Vector3::ZERO)
			{}
		};

		struct SinglelVertexBuff
		{
			GPUBuffer* m_gpubuff;
			RenderInput::VertexElementList m_vertex_element_list;
			SinglelVertexBuff()
				:m_gpubuff(NULL)
			{

			}
		};

		typedef Echo::vector<SinglelVertexBuff>::type MutiVertexBuff;
		typedef Echo::vector<SinglelVertexBuff>::const_iterator itrMutiVertexBuff;

	public:
		// 获取名称
		const String& getName() const { return m_name; }

		// 获取漫反射纹理
		TextureRes* getDiffuseTexture() const;

		// get mask texture
		TextureRes* getMasktexture() const;

		// 获取顶点信息
		VertexInfo& getVertexInfo() { return m_vertInfo; }

		// 获取顶点格式大小
		ui32 getVertexStride() const { return m_vertInfo.m_stride; }

		// 获取顶点数量
		ui32 getVertexCount() const { return m_vertInfo.m_count; }

		// 获取顶点数据
		Byte* getVertices() const { return m_vertInfo.m_vertices; }

		// 获取顶点缓冲
		GPUBuffer* getVertexBuffer() const;

		// 获取索引缓冲
		GPUBuffer* getIndexBuffer() const;

		// 获取面数量
		ui32 getFaceCount() const;

		// 获取索引数量
		ui32 getIndexCount() const;

		// 获取索引格式大小
		ui32 getIndexStride() const;

		// 获取索引数据
		Word* getIndices() const;

		// 判断顶点格式中是否含有指定类型的数据
		bool isVertexUsage(RenderInput::VertexSemantic semantic) const { return m_vertInfo.isVertexUsage(semantic);}

		// 获取顶点格式
		const RenderInput::VertexElementList& getVertexElements() const;

		// 获取本地包围盒
		const Box& getLocalBox() const;

		// 获取受影响的骨骼数量
		ui32 getBoneNum() const { return static_cast<ui32>(m_boneIdxs.size()); }

		// 获取受影响的骨骼索引
		ui32 getBoneIdx( int idx) { return m_boneIdxs[idx]; }

		// 获取光照图大小
		int getLightmapResolution() const { return m_lightmapResolution; }

		// 设置光照图大小
		void setLightmapResolution( int resolution) { m_lightmapResolution = resolution; }

		// 是否使用半精度浮点数
		bool getUseHalfFloat(){ return m_bHalfFloat; }

		void generateTangentData( bool useNormalMap );

		void setVisible(bool state) { m_visible = state; }
		bool* isVisible() { return &m_visible; }

		bool isMultiVertexBuff(){ return m_is_muti_vertex_buff; }

		const MutiVertexBuff& getMultiVertexBuff(){ return m_muti_vertex_buff; }

		// 更新
		virtual void update(ui32 delta){};

		void removeRedundancyVertex();

	protected:
		SubMesh();
		virtual ~SubMesh();

		// 加载
		virtual bool load();

		// 卸载
		virtual void unloadImpl();

		// ...
		virtual void unprepareImpl();

		// 附加数据
		virtual void dataInsert(const RenderInput::VertexElement& element, void* templateData);

		// 移除数据
		virtual void dataRemove(RenderInput::VertexSemantic semantic);

		// 计算占用内存大小
		virtual size_t calculateSize() const;

		// 设置顶点信息 floatPos: 大于0，连续float个数，小于0，float段间隔字节数
		virtual void setVertexStructInfo(bool isSkinned, bool useHalfFloat, std::vector<int>& floatPos);

		// 计算切线数据
		virtual void buildTangentData();

		virtual void reBuildVertexBuffer();

	protected:
		String					m_name;					// 名称
		Box						m_box;					// 包围盒
		int						m_lightmapResolution;	// 光照图分辨率
		VertexInfo				m_vertInfo;				// 顶点信息
		ui32					m_idxCount;				// 索引数量
		ui32					m_idxStride;			// 索引格式大小
		Byte*					m_indices;				// 索引数据
		GPUBuffer*				m_vertexBuffer;			// 顶点缓冲
		GPUBuffer*				m_indexBuffer;			// 索引缓冲

		TextureRes*				m_pDiffTex;				// 漫反射纹理
		TextureRes*				m_pMaskTex;				// mask
		bool					m_bHalfFloat;
		vector<ui32>::type		m_boneIdxs;				// 骨骼索引

		bool					m_visible;				// 是否显示
		bool					m_is_muti_vertex_buff;		// 是否是多流
		MutiVertexBuff			m_muti_vertex_buff;		// 多流顶点buff
	};
}

