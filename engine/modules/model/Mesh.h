#ifndef __ECHO_MESH_H__
#define __ECHO_MESH_H__

#include "engine/core/render/render/RenderInput.h"
#include "Engine/core/Resource/Resource.h"
#include "Engine/core/Geom/Box.h"
#include "engine/core/Util/Array.hpp"
//#include "scl/bitset.h"

namespace Echo
{
	/**
	 * 模型
	 */
	class SubMesh;
	class DataStream;
	class Mesh: public Resource
	{
		friend class MeshManager;
		friend class Model;
	public:
		Mesh(const String& name);
		virtual ~Mesh();

		// 获取包围盒
		const Box& getBox() const;

		// 重计算包围盒
		void recalcBox();

		void reBuildVertexBuffer();

		// 是否是动画模型
		bool isSkinned() const { return m_isSkinned; }

		// some function to re build
		virtual void rebuildVertexFomat_Insert(const RenderInput::VertexElement& element, void* templateData);
		virtual void rebuildVertexFomat_Remove(RenderInput::VertexSemantic semantic);

		// 获取版本号
		bool getUseSubMeshBoneMatrices() const { return m_useSubMeshBoneMatrices; }

		// 获取顶点数量
		int getVertexCount();

		// 获取子模型数量
		int getSubMeshNum() const { return static_cast<int>( m_subMeshs.size()); }

		// 获取子模型
		SubMesh* getSubMesh( int idx);

		// 优化子模型
		void redundancySubMeshs();

		// 设置是否保留漫反射贴图CPU像素数据
		void setRetainPreparedData(bool setting){ m_isRetainPreparedData = setting;}

		// 保存
		virtual bool save( const char* filePath);
		virtual bool saveToFile( const char* filePath );

		void generateTangentData( bool useNormalMap );

		bool isValid();

		// 更新
		virtual void update( ui32 delta, const Echo::Vector3& vpos, const Echo::Quaternion& qrotate){};

		// 是否需要更新
		virtual bool isNeedUpdate(){ return false; }

		virtual bool			needCloneNew(){ return false; }
		virtual Mesh*			cloneNew(){ return NULL; }

		virtual const String&	getCloneName(){ return m_clone_name; }

		const array<bool, 256>&	getEffectiveBones();

	protected:
		virtual size_t	calculateSize() const;
		virtual bool prepareImpl( DataStream* stream);
		virtual bool prepareImplInteral(DataStream* stream, bool isSkinned);
		virtual void unprepareImpl();
		virtual bool loadImpl();
		virtual void unloadImpl();

		// 文件解析
		virtual bool parseFromFileVersion_old(DataStream* stream, bool useHalfFloat);
		void disableDefaultTexLoad();

	protected:
		bool					m_useSubMeshBoneMatrices;
		bool					m_bNeedBackUp;
		Box						m_box;
		vector<SubMesh*>::type	m_subMeshs;				// 子模型列表
		bool					m_isSkinned;			// 是否为带动画模型
		bool					m_isRetainPreparedData; // 是否保留漫反射贴图CPU像素数据
		bool					m_isValid;
		Echo::String			m_clone_name;			//当前被clone的名字
		bool					m_isDisableDefaultTex;
		array<bool, 256>		m_effectiveBones;
	};

	/**
	 * 实例化网格。 
	 * 杨印宇
	 */
	class InstanceMesh : public Mesh
	{
	public:
		InstanceMesh(const String& name) :Mesh(name){}
		~InstanceMesh() {}

		// 获取模板模型名称
		Echo::String GetTemplateMeshName() const { return m_templateName; }

		// 获取实例数量
		int	GetInstanceCount() const { return m_instanceCount; }

		// 根据索引获取实例位置
		Echo::Vector3 GetInstancePos( size_t id) const { EchoAssert(id>=0&&id<m_instancePos.size()); return m_instancePos[id]; }

		// 根据索引获取实例旋转
		Echo::Quaternion GetInstanceRot( size_t id) const { EchoAssert(id>=0&&id<m_instanceRot.size()); return m_instanceRot[id]; }

		// 根据索引获取实例缩放
		Echo::Vector3 GetInstanceScale( size_t id) const { EchoAssert(id>=0&&id<m_instanceScale.size()); return m_instanceScale[id]; }

	protected:
		// 内部加载实现
		bool prepareImplInteral(DataStream* stream, bool isSkinned);
		virtual void unloadImpl();

		// 文件解析
		bool parseFromFileOneSumesh(DataStream* stream, bool useHalfFloat);
	private:
		// 实例化顶点数据
		void InstanceVertices(SubMesh* subMesh, const vector<Echo::Vector3>::type& instancePos, const vector<Echo::Quaternion>::type& instanceRot, const vector<Echo::Vector3>::type& instanceScale);

		// 实例化索引数据
		void InstanceIndices(SubMesh* subMesh,size_t instanceVertexCount,size_t instanceCount);

		// 实例化不同submesh的顶点数据
		void InstanceSubMeshVertices(const vector<Echo::Vector3>::type& instancePos, const vector<Echo::Quaternion>::type& instanceRot, const vector<Echo::Vector3>::type& instanceScale);

		// 实例化不同submesh的索引数据
		void InstanceSubMeshIndices(size_t firstSubMeshVertexCount, size_t instanceCount);

	protected:
		Echo::String					m_templateName;			// 模板网格的名字。
		int								m_instanceCount;		// 实例总数。
		int								m_subMeshCount;
		bool							m_isSameModel;
		vector<Echo::Vector3>::type		m_instancePos;
		vector<Echo::Quaternion>::type	m_instanceRot;
		vector<Echo::Vector3>::type		m_instanceScale;
	};
}

#endif