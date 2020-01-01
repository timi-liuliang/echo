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
			bool			m_isHaveScript;
			ResourcePath	m_file;					// file name
			String			m_globalTableName;		// global table name

			LuaScript() : m_isStart(false), m_isHaveScript(false), m_file("", ".lua"){}
            void bind(Node* obj);
			void start(Node* obj);
			void update(Node* obj);
			void release(Node* obj);
		};

	public:
		Node();
		virtual ~Node();

		// parent(can only have one parent)
		void setParent(Node* pParent);
		Node* getParent() const;

		ui32 getChildNum() const { return static_cast<ui32>(m_children.size()); }
		Node* getChildByIndex(ui32 idx);
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
        
        // call lua function of this node
        virtual void callLuaFunction(const String& funName, const Variant** args, int argCount) override;

	public:
		// get property value
		Variant getPropertyValueR(const String& propertyName);

		// ch
		Variant ch(const String& path, const String& propertyName);
		
	public:
		// get node by path
		Node* getNode(const char* path);
        
        // get node by path
        template<typename T> T getNodeT(const char* path);

		// build node path
		String getNodePath() const;
		String getNodePathRelativeTo(const Node* baseNode) const;

		// queue free
		virtual void queueFree() override;

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
		virtual void registerToScript() override;

	protected:
        // dirty update flag
		void needUpdate();
        
        // start (the first time update the node)
        virtual void start() {}

        // update self
		virtual void update_self() {}

	protected:
		bool			m_isEnable = true;
		bool			m_isLink = false;	        // belong to branch scene
		Node*			m_parent = nullptr;
		NodeArray		m_children;
		bool			m_isTransformDirty = false;	// for rendering.
		Transform		m_localTransform;
		Transform		m_worldTransform;
		Matrix4			m_matWorld;			        // cached derived transform as a 4x4 matrix
		AABB			m_localAABB;		        // local aabb
		LuaScript		m_script;			        // bind script
	};
    
    // get node by path
    template<typename T>
    T Node::getNodeT(const char* path)
    {
        Node* node = getNode(path);
        return ECHO_DOWN_CAST<T>(node);
    }
}
