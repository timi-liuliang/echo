#pragma once

#include <engine/core/Math/Math.h>
#include "engine/core/Util/PtrMonitor.h"
#include "engine/core/geom/AABB.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class Node : public Object
	{
		ECHO_CLASS(Node, Object)

		friend class NodeTree;
		typedef vector<Node*>::type NodeArray;

	public:
		// lua script
		struct LuaScript
		{
			bool			m_isStart;
			bool			m_isValid;
			ResourcePath	m_file;					// file name
			String			m_globalTableName;		// global table name

			LuaScript() : m_isStart(false), m_isValid(false), m_file("", ".lua"){}
			void start(Node* obj);
			void update(Node* obj);
			bool isValid()const { return m_isValid; }
		};

	public:
		Node();
		virtual ~Node();

		// parent(can only have one parent)
		void setParent(Node* pParent);
		Node* getParent() const;

		ui32 getChildNum() const { return m_children.size(); }
		Node* getChild(ui32 idx);

		// remove from tree
		void remove();

		// remove child
		bool isChildExist(const String& name);
		void addChild(Node* node);
		bool removeChild(Node* node);

		void setActive(bool isActive) { m_isActive = isActive; }
		bool isActive() const { return m_isActive; }
		
		void scale(const Vector3& scl);
		void roll(const Real randian);
		void pitch(const Real randian);
		void yaw(const Real randian);
		void rotate(const Vector3& vAxis, const Real randian);
		void rotate(const Quaternion& rot);
		void translate(const Vector3& d);

		void setLocalScaling(const Vector3& scl);
		void setLocalScalingXYZ(Real x, Real y, Real z);
		void setLocalOrientation(const Quaternion& ort);
		void setLocalPosition(const Vector3& pos);
		void setLocalPositionX(float pos);
		void setLocalPositionXYZ(Real posX, Real posY, Real posZ);
		void setLocalYawPitchRoll(const Vector3& yawPitchRoll);

		void setWorldOrientation(const Quaternion& ort);
		void setWorldPosition(const Vector3& pos);

		void update(float delta, bool bUpdateChildren = false);

		const Vector3& getLocalScaling() const;
		const Quaternion& getLocalOrientation() const;
		const Vector3 getLocalYawPitchRoll();
		const Vector3& getLocalPosition() const;
		const Vector3& getWorldScaling() const;
		const Quaternion& getWorldOrientation() const;
		void getWorldOrientationWXYZ(Real* w = 0, Real* x = 0, Real* y = 0, Real* z = 0);
		const Vector3& getWorldPosition() const;
		void getWorldPositionXYZ(Real* x = 0, Real* y = 0, Real* z = 0);

		const Matrix4& getWorldMatrix();
		Matrix4	getInverseWorldMatrix() const;

		void convertWorldToLocalPosition(Vector3& posLocal, const Vector3& posWorld);
		void convertWorldToLocalOrientation(Quaternion& ortLocal, const Quaternion& ortWorld);

		// script
		const ResourcePath& getScript() { return m_script.m_file; }
		void setScript(const ResourcePath& path);

	public:
		Node* getNode(const String& path) { return nullptr; }

		// 获取全局变量值
		virtual void* getGlobalUniformValue(const String& name);

	public:
		// save
		void save(const String& path);

		// load
		static Node* load(const String& path);

	private:
		// save xml recursive
		void saveXml(void* pugiNode, Node* node);

		// instance node
		static Node* instanceNodeTree(void* pugiNode, Node* parent);

	protected:
		inline void		needUpdate();

		virtual void update() {}

	protected:
		bool			m_isActive;

		Node*			m_parent;

		// only to notify the parent is modifyed.
		NodeArray		m_children;

		Vector3			m_posLocal;
		Quaternion		m_ortLocal;
		Vector3			m_sclLocal;

		// Cached world space transform
		Vector3			m_posWorld;
		Quaternion		m_ortWorld;
		Vector3			m_sclWorld;

		/// Cached derived transform as a 4x4 matrix
		Matrix4			m_matWorld;

		bool			m_bModify;      //for caculate. ie: getWorldPostion
		bool			m_bMatrixDirty; //for rendering.

		AABB			m_localAABB;	// local aabb

		LuaScript		m_script;		// bind script
	};
}
