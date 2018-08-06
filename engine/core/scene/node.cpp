#include "Node.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/script/lua/LuaBinder.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	void Node::LuaScript::start(Node* obj)
	{
		m_isValid = Engine::instance()->getConfig().m_isGame && IO::instance()->isResourceExists(m_file.getPath());
		if (m_isValid)
		{
			m_globalTableName = StringUtil::Format("_Nodes._%d", obj->getId());;
			luaex::LuaEx::instance()->register_object("Node", m_globalTableName.c_str(), obj);

			String fileName = PathUtil::GetPureFilename(m_file.getPath(), false);
			String moduleName = StringUtil::Replace(m_file.getPath(), "Res://", "");
			moduleName = StringUtil::Replace(moduleName, "/", ".");
			moduleName = StringUtil::Replace(moduleName, "\\", ".");
			moduleName = StringUtil::Replace(moduleName, ".lua", "");
			
			String luaStr = StringUtil::Format(
				"local script_table = require \"%s\"\n"\
				"utils.append_table(%s, script_table)", moduleName.c_str(),m_globalTableName.c_str());

			LuaBinder::instance()->execString(luaStr);
		}
	}

	void Node::LuaScript::update(Node* obj)
	{
		if (!m_isStart)
		{
			start(obj);
			m_isStart = true;
		}
	}

	Node::Node()
		: m_parent(NULL)
		, m_isEnable(true)
		, m_isLink(false)
		, m_posLocal(Vector3::ZERO)
		, m_ortLocal(Quaternion::IDENTITY)
		, m_sclLocal(Vector3::ONE)
		, m_bModify(false)
		, m_bMatrixDirty(false)
	{
		m_sclWorld = m_sclLocal;
		m_ortWorld = m_ortLocal;
		m_posWorld = m_posLocal;

		m_matWorld = Matrix4::IDENTITY;
		
		needUpdate();
        
        m_children.clear();
	}

	// 析构函数
	Node::~Node()
	{
	}

	// 设置父节点
	void Node::setParent(Node* parent)
	{
		if(m_parent)
			m_parent->removeChild(this);

		m_parent = parent;

		// make sure the name is unique in current layer
		int    id = 1;
		String name = getName();
		while (parent->isChildExist(getName()))
		{
			id++;
			setName(StringUtil::Format( "%s%d", name.c_str(), id));
		}

		if(m_parent)
			m_parent->m_children.push_back(this);

		needUpdate();
	}

	void Node::scale(const Vector3& scl)
	{
		m_sclLocal = m_sclLocal * scl;
		
		needUpdate();
	}

	void Node::roll(const Real randian)
	{
		rotate(Vector3::UNIT_Z, randian);
	}

	void Node::pitch(const Real randian)
	{
		rotate(Vector3::UNIT_X, randian);
	}

	void Node::yaw(const Real randian)
	{
		rotate(Vector3::UNIT_Y, randian);
	}

	void Node::rotate(const Vector3& vAxis, const Real randian)
	{
		Quaternion rot(vAxis, randian);
		rotate(rot);
	}

	void Node::rotate(const Quaternion& rot)
	{
		// Normalise quaternion to avoid drift
		Quaternion qnorm = rot;
		qnorm.normalize();

		// Note the order of the mult, i.e. q comes after
		m_ortLocal = m_ortLocal * qnorm;

		needUpdate();
	}

	void Node::translate(const Vector3& d)
	{
		m_posLocal += m_ortLocal * d;

		needUpdate();
	}

	void Node::setLocalScaling(const Vector3& scl)
	{
		m_sclLocal = scl;
		needUpdate();
	}

	void Node::setLocalScalingXYZ(Real x, Real y, Real z)
	{
		Vector3 scl(x, y, z);
		setLocalScaling(scl);
	}

	void Node::setLocalOrientation(const Quaternion& ort)
	{
		m_ortLocal = ort;
		m_ortLocal.normalize();

		needUpdate();
	}

	void Node::setLocalPosition(const Vector3& pos)
	{
		m_posLocal = pos;
		needUpdate();
	}

	void Node::setLocalPositionX(float pos)
	{
		m_posLocal.x = pos;
		needUpdate();
	}

	void Node::setLocalPositionXYZ(Real posX, Real posY, Real posZ)
	{
		Vector3 pos(posX, posY, posZ);
		setLocalPosition(pos);
	}

	void Node::setWorldOrientation(const Quaternion& ort)
	{
		if(m_parent)
		{
			Quaternion ortLocal;
			m_parent->convertWorldToLocalOrientation(ortLocal, ort);
			setLocalOrientation(ortLocal);
		}
		else
		{
			setLocalOrientation(ort);
		}
	}

	void Node::setWorldPosition(const Vector3& pos)
	{
		if(m_parent)
		{
			Vector3 posLocal;
			m_parent->convertWorldToLocalPosition(posLocal, pos);
			setLocalPosition(posLocal);
		}
		else
		{
			setLocalPosition(pos);
		}
	}

	Node* Node::getParent() const
	{
		return m_parent;
	}

	Node* Node::getChild(ui32 idx)
	{
		return *std::next(m_children.begin(), idx);
	}

	// remove from tree
	void Node::remove()
	{
		Node* parent = getParent();
		if (parent)
			parent->removeChild(this);
	}

	// remove child
	bool Node::isChildExist(const String& name)
	{
		for (Node* child : m_children)
		{
			if (child->getName() == name)
				return true;
		}

		return false;
	}

	// remove child
	void Node::addChild(Node* node)
	{
		node->setParent(this);
	}

	// remove child
	bool Node::removeChild(Node* node)
	{
		for( NodeArray::iterator it = m_children.begin(); it!=m_children.end(); it++)
		{
			if (*it == node)
			{
				m_children.erase(it);
				return true;
			}
		}

		return false;
	}

	const Vector3& Node::getLocalScaling() const
	{
		return m_sclLocal;
	}

	const Quaternion& Node::getLocalOrientation() const
	{
		return m_ortLocal;
	}

	const Vector3 Node::getLocalYawPitchRoll()
	{
		Vector3 yawpitchroll;
		m_ortLocal.toEulerAngle(yawpitchroll.x, yawpitchroll.y, yawpitchroll.z);

		return yawpitchroll;
	}

	void Node::setLocalYawPitchRoll(const Vector3& yawPitchRoll)
	{
		Quaternion quat;
		quat.fromEulerAngle(yawPitchRoll.x, yawPitchRoll.y, yawPitchRoll.z);

		setLocalOrientation(quat);
	}

	const Vector3& Node::getLocalPosition() const
	{
		return m_posLocal;
	}

	const Vector3& Node::getWorldScaling() const
	{
		return m_sclWorld;
	}

	const Quaternion& Node::getWorldOrientation() const
	{
		return m_ortWorld;
	}

	void Node::getWorldOrientationWXYZ(Real* w /* = 0 */, Real* x /* = 0 */, Real* y /* = 0 */, Real* z /* = 0 */)
	{
		*w = m_ortWorld.w;
		*x = m_ortWorld.x;
		*y = m_ortWorld.y;
		*z = m_ortWorld.z;
	}

	const Vector3& Node::getWorldPosition() const
	{
		return m_posWorld;
	}
	
	void Node::getWorldPositionXYZ(Real* x /* = 0 */, Real* y /* = 0 */, Real* z /* = 0 */)
	{
		*x = m_posWorld.x;
		*y = m_posWorld.y;
		*z = m_posWorld.z;
	}
	
	const Matrix4& Node::getWorldMatrix()
	{
		if (m_bMatrixDirty)
		{
			if(m_bModify)
				update_self();

			m_matWorld.makeScaling(getWorldScaling());

			Matrix4 matRot;
			matRot.fromQuan(getWorldOrientation());
			m_matWorld = m_matWorld * matRot;

			m_matWorld.translate(getWorldPosition());
			m_bMatrixDirty = false;
		}

		return m_matWorld;
	}

	Matrix4 Node::getInverseWorldMatrix() const
	{
		Matrix4 inverseMat;
		inverseMat.makeTranslation(-m_posWorld);
		Matrix4 matRot;
		Quaternion rot = m_ortWorld;
		rot.conjugate();
		matRot.fromQuan(rot);
		inverseMat = inverseMat * matRot;
		matRot.makeScaling(1.0f/m_sclWorld.x, 1.0f/m_sclWorld.y, 1.0f/m_sclLocal.z);
		inverseMat = inverseMat * matRot;
		return inverseMat;
	}

	void Node::convertWorldToLocalPosition(Vector3& posLocal, const Vector3& posWorld)
	{
		Quaternion ortWorldInv = m_ortWorld;
		ortWorldInv.inverse();
		posLocal = ortWorldInv * (posWorld - m_posWorld) / m_sclWorld;
	}

	void Node::convertWorldToLocalOrientation(Quaternion& ortLocal, const Quaternion& ortWorld)
	{
		Quaternion ortWorldInv = m_ortWorld;
		ortWorldInv.inverse();

		ortLocal = ortWorldInv * ortWorld;
	}

	void Node::setScript(const ResourcePath& path)
	{
		if(!path.getPath().empty())
			m_script.m_file.setPath(path.getPath());
	}

	void Node::needUpdate()
	{
		if (m_bModify)
			return;

		m_bModify = true;
		m_bMatrixDirty = true;

		for (Node* node : m_children)
		{
			node->needUpdate();
		}
	}

	void Node::update(float delta, bool bUpdateChildren)
	{
		if (!m_isEnable)
			return;

		if (m_bModify)
		{
			if (m_parent)
			{
				m_ortWorld = m_parent->m_ortWorld * m_ortLocal;

				// Update scale
				m_sclWorld = m_parent->m_sclWorld * m_sclLocal;

				// Change position vector based on parent's orientation & scale
				m_posWorld = m_parent->m_ortWorld * (m_parent->m_sclWorld * m_posLocal);
				m_posWorld += m_parent->m_posWorld;
			}
			else
			{
				m_ortWorld = m_ortLocal;
				m_posWorld = m_posLocal;
				m_sclWorld = m_sclLocal;
			}

			m_bModify = false;
		}

		// update world matrix
		getWorldMatrix();

		// script update
		m_script.update(this);

		update_self();

		if (bUpdateChildren)
		{
			for (Node* node : m_children)
			{
				node->update(delta, bUpdateChildren);
			}
		}
	}

	// bind methods
	void Node::bindMethods()
	{
		luaex::LuaEx* luaEx = luaex::LuaEx::instance();
		if (!luaEx)
			return;

		luaEx->register_class("Node");
		luaEx->register_function<Node, float>("Node", "setPosX", &Node::setLocalPositionX);

		CLASS_BIND_METHOD(Node, getLocalPosition,	  DEF_METHOD("getPos"));
		CLASS_BIND_METHOD(Node, getWorldPosition,	  DEF_METHOD("getWorldPos"));
		CLASS_BIND_METHOD(Node, getLocalYawPitchRoll, DEF_METHOD("getYawPitchRoll"));
		CLASS_BIND_METHOD(Node, getLocalScaling,	  DEF_METHOD("getScale"));
		CLASS_BIND_METHOD(Node, setLocalPosition,	  DEF_METHOD("setPos"));
		CLASS_BIND_METHOD(Node, setLocalScaling,	  DEF_METHOD("setScale"));
		CLASS_BIND_METHOD(Node, setLocalYawPitchRoll, DEF_METHOD("setYawPitchRoll"));
		CLASS_BIND_METHOD(Node, setEnable,			  DEF_METHOD("setEnable"));
		CLASS_BIND_METHOD(Node, isEnable,			  DEF_METHOD("isEnable"));
		CLASS_BIND_METHOD(Node, setScript,			  DEF_METHOD("setScript"));
		CLASS_BIND_METHOD(Node, getScript,			  DEF_METHOD("getScript"));

		CLASS_REGISTER_PROPERTY(Node, "Enable", Variant::Type::Bool, "isEnable", "setEnable");
		CLASS_REGISTER_PROPERTY(Node, "Position", Variant::Type::Vector3, "getPos", "setPos");
		CLASS_REGISTER_PROPERTY(Node, "Rotation", Variant::Type::Vector3, "getYawPitchRoll", "setYawPitchRoll");
		CLASS_REGISTER_PROPERTY(Node, "Scale",  Variant::Type::Vector3, "getScale", "setScale");
		CLASS_REGISTER_PROPERTY(Node, "Script", Variant::Type::ResourcePath, "getScript", "setScript");
	}

	// queue free
	void Node::queueFree()
	{
		if (m_parent)
		{
			m_parent->removeChild(this);
			m_parent = nullptr;
		}		

		NodeArray children = m_children;
		for (Node* n : children)
		{
			n->queueFree();
		}

		ECHO_DELETE_T(this, Node);
	}

	// duplicate
	Node* Node::duplicate(bool recursive)
	{
		// save
		pugi::xml_document doc;
		pugi::xml_node root = doc.append_child("node");
		saveXml(&root, this, recursive);

		// load
		Node* duplicateNode = instanceNodeTree(&root, nullptr);

		return duplicateNode;
	}

	// save
	void Node::save(const String& path)
	{
		String fullPath = IO::instance()->getFullPath(path);

		pugi::xml_document doc;

		// declaration
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		// root node
		pugi::xml_node root = doc.append_child("node");

		saveXml(&root, this, true);

		doc.save_file(fullPath.c_str(), "\t", 1U, pugi::encoding_utf8);
	}

	// save xml recursive
	void Node::saveXml(void* pugiNode, Node* node, bool recursive)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		xmlNode->append_attribute("name").set_value(node->getName().c_str());
		xmlNode->append_attribute("class").set_value(node->getClassName().c_str());
		xmlNode->append_attribute("path").set_value(node->getPath().c_str());

		savePropertyRecursive(pugiNode, node, node->getClassName());

		if (recursive)
		{
			for (ui32 idx = 0; idx < node->getChildNum(); idx++)
			{
				Node* child = node->getChild(idx);
				if (child && !child->isLink())
				{
					pugi::xml_node newNode = xmlNode->append_child("node");
					saveXml(&newNode, child, true);
				}
			}
		}
	}

	// load
	Node* Node::load(const String& path, bool isLink)
	{
		MemoryReader reader(path);
		if (reader.getSize())
		{
			pugi::xml_document doc;
			if (doc.load_buffer(reader.getData<char*>(), reader.getSize()))
			{
				pugi::xml_node root = doc.child("node");
				Node* rootNode = instanceNodeTree(&root, nullptr);
				if (isLink)
				{
					rootNode->setPath(path);
					for (Echo::ui32 idx = 0; idx < rootNode->getChildNum(); idx++)
					{
						rootNode->getChild(idx)->setLink(true);
					}
				}

				return rootNode;
			}
		}

		EchoLogError("Node::load failed. path [%s] not exist", path.c_str());

		return nullptr;
	}

	Node* Node::instanceNodeTree(void* pugiNode, Node* parent)
	{
		if (pugiNode)
		{
			pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;
			Echo::String path = xmlNode->attribute("path").value();
			Node* node = path.empty() ? ECHO_DOWN_CAST<Node*>(instanceObject(pugiNode)) : load(path, true);
			if (node)
			{
				if (!path.empty())
				{
					// overwrite property
					loadPropertyRecursive(xmlNode, node, node->getClassName());
				}

				if (parent)
					parent->addChild(node);

				for (pugi::xml_node child = xmlNode->child("node"); child; child = child.next_sibling("node"))
				{
					instanceNodeTree(&child, node);
				}

				return node;
			}
		}

		return nullptr;
	}
}