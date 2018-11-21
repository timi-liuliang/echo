#pragma once

#include <engine/core/math/Math.h>
#include "engine/core/geom/AABB.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class Node : public Object
	{
		ECHO_CLASS(Node, Object)

		friend class NodeTree;		

	public:
		typedef vector<Node*>::type NodeArray;

		// transform
		struct Transform
		{
			Vector3		m_pos;
			Quaternion	m_quat;
			Vector3		m_scale;

			Transform() { reset(); }

			// build matrix
			void buildMatrix(Matrix4& mat) const;

			// build inv matrix
			void buildInvMatrix(Matrix4& invMat) const;

			// operate "*"
			Transform operator* (const Transform& b) const;

			// reset
			void reset();
		};

		// lua script
		struct LuaScript
		{
			bool			m_isStart;
			bool			m_isRegistered;
			bool			m_isHaveScript;
			ResourcePath	m_file;					// file name
			String			m_globalTableName;		// global table name

			LuaScript() : m_isStart(false), m_isRegistered(false), m_isHaveScript(false), m_file("", ".lua"){}
			void start(Node* obj);
			void update(Node* obj);
		};

	public:
		Node();
		virtual ~Node();

		// parent(can only have one parent)
		void setParent(Node* pParent);
		Node* getParent() const;

		ui32 getChildNum() const { return static_cast<ui32>(m_children.size()); }
		Node* getChild(ui32 idx);
		Node* getChild(const char* name);
		i32   getChildIdx(Node* node);
		const NodeArray& getChildren() { return m_children; }

		// remove from tree
		void remove();

		// remove child
		bool isChildExist(const String& name);
		void insertChild(ui32 idx, Node* node);
		void addChild(Node* node);
		bool removeChild(Node* node);

		void setEnable(bool isEnable) { m_isEnable = isEnable; }
		bool isEnable() const { return m_isEnable; }

		// is branch node
		bool isLink() const { return m_isLink; }
		void setLink(bool isLink) { m_isLink = isLink; }
		
		// rotate base on current rotate state
		void rotate(const Quaternion& rot);

		void setLocalScaling(const Vector3& scl);
		void setLocalScalingXYZ(Real x, Real y, Real z);
		void setLocalOrientation(const Quaternion& ort);
		void setLocalPosition(const Vector3& pos);
		void setLocalYawPitchRoll(const Vector3& yawPitchRoll);
		void setWorldOrientation(const Quaternion& ort);

		// update recursive
		virtual void update(float delta, bool bUpdateChildren = false);
		
		const Transform& getWorldTransform() const { return m_worldTransform; }
		const Vector3& getLocalScaling() const;
		const Quaternion& getLocalOrientation() const;
		const Vector3 getLocalYawPitchRoll();
		const Vector3& getLocalPosition() const;
		const Vector3& getWorldScaling() const;
		const Quaternion& getWorldOrientation() const;

		// set world position
		void setWorldPosition(const Vector3& pos);
		void setWorldPositionX(float x);
		void setWorldPositionY(float y);
		void setWorldPositionZ(float z);

		// get world position
		const Vector3& getWorldPosition() const;
		float getWorldPositionX() const { return getWorldPosition().x; }
		float getWorldPositionY() const { return getWorldPosition().y; }
		float getWorldPositionZ() const { return getWorldPosition().z; }

		const Matrix4& getWorldMatrix();
		Matrix4	getInverseWorldMatrix() const;

		void convertWorldToLocalPosition(Vector3& posLocal, const Vector3& posWorld);
		void convertWorldToLocalOrientation(Quaternion& ortLocal, const Quaternion& ortWorld);

		// script
		const ResourcePath& getScript() { return m_script.m_file; }
		void setScript(const ResourcePath& path);

		// aabb
		void buildWorldAABB(AABB& aabb);
		const AABB& getLocalAABB() const { return m_localAABB; }
		
	public:
		// get node by path
		Node* getNode(const char* path);

		// queue free
		virtual void queueFree();

		// duplicate
		Node* duplicate(bool recursive);

		// save
		void save(const String& path);

		// instance
		static Node* load(const char* path);

		// load link
		static Node* loadLink(const String& path, bool isLink);

	private:
		// save xml recursive
		void saveXml(void* pugiNode, Node* node, bool recursive);

		// instance node
		static Node* instanceNodeTree(void* pugiNode, Node* parent);

		// register to script
		void registerToScript();

	protected:
		void needUpdate();

		virtual void update_self() {}

	protected:
		bool			m_isEnable;
		bool			m_isLink;			// belong to branch scene
		Node*			m_parent;
		NodeArray		m_children;
		bool			m_isTransformDirty;	// for rendering.
		Transform		m_localTransform;
		Transform		m_worldTransform;
		Matrix4			m_matWorld;			// cached derived transform as a 4x4 matrix
		AABB			m_localAABB;		// local aabb
		LuaScript		m_script;			// bind script
	};
}
