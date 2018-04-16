#pragma once

#include "engine/core/base/class.h"
#include <engine/core/Math/EchoMath.h>
#include "engine/core/Util/PtrMonitor.h"
#include "engine/core/geom/Box.h"

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
			String		m_file;					// file name
			String		m_globalTableName;		// global table name

			void start(Object* obj);
			void update();
		};

	public:
		Node();
		virtual ~Node();

		// bind methods to script
		static void bindMethods();

		ui32 getIdentifier() const { return m_identifier; }
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

		void setVisible(bool bVisible);
		bool isVisible() const;
		
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

		// remember property recursive
		void savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

		// instance node
		static Node* instanceNode(void* pugiNode);
		static void instanceChildNode(void* pugiNode, Node* parent);

		// remember property recursive
		static void loadPropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

	protected:
		inline void		needUpdate();

		virtual void update() {}

	protected:
		ui32			m_identifier;		// 唯一标识符
		Node*			m_parent;
		bool			m_isVisible;

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

		Box				m_localAABB;	// local aabb

		LuaScript		m_script;		// bind script
	};
}
