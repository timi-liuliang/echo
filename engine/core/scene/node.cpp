#include "node.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/script/lua/lua_binder.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	void Node::LuaScript::start(Node* obj)
	{
		m_isHaveScript = IO::instance()->isResourceExists(m_file.getPath());
		if ( Engine::instance()->getConfig().m_isGame )
		{
			if ( m_isHaveScript)
			{
				obj->registerToScript();

				String fileName = PathUtil::GetPureFilename(m_file.getPath(), false);
				String moduleName = StringUtil::Replace(m_file.getPath(), "Res://", "");
				moduleName = StringUtil::Replace(moduleName, "/", ".");
				moduleName = StringUtil::Replace(moduleName, "\\", ".");
				moduleName = StringUtil::Replace(moduleName, ".lua", "");

				String luaStr = StringUtil::Format(
					"nodes._%d = objs._%d\n"\
					"local script_table = require \"%s\"\n"\
					"utils.append_table(%s, script_table)\n" \
					"%s:start()\n", obj->getId(), obj->getId(), moduleName.c_str(), m_globalTableName.c_str(), m_globalTableName.c_str());

				LuaBinder::instance()->execString(luaStr);
			}
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

	// build matrix
	void Node::Transform::buildMatrix(Matrix4& mat) const
	{
		mat.makeScaling( m_scale);

		Matrix4 matRot;
		matRot.fromQuan(m_quat);
		mat = mat * matRot;

		mat.translate(m_pos);
	}

	void Node::Transform::buildInvMatrix(Matrix4& invMat) const
	{
		invMat.makeTranslation(-m_pos);
		Matrix4 matRot;
		Quaternion rot = m_quat;
		rot.conjugate();
		matRot.fromQuan(rot);
		invMat = invMat * matRot;
		matRot.makeScaling(1.0f / m_scale.x, 1.0f / m_scale.y, 1.0f / m_scale.z);
		invMat = invMat * matRot;
	}

	// operate "*"
	Node::Transform Node::Transform::operator* (const Transform& b) const
	{
		Transform result;

		result.m_quat = m_quat * b.m_quat;
		result.m_scale = m_scale * b.m_scale;
		result.m_pos = m_quat * (m_scale * b.m_pos);
		result.m_pos += m_pos;

		return result;
	}

	void Node::Transform::reset()
	{
		m_pos = Vector3::ZERO;
		m_quat = Quaternion::IDENTITY;
		m_scale = Vector3::ONE;
	}

	Node::Node()
	{
		m_matWorld = Matrix4::IDENTITY;
		needUpdate();   
        m_children.clear();
	}

	Node::~Node()
	{
	}

	void Node::rotate(const Quaternion& rot)
	{
		// Normalise quaternion to avoid drift
		Quaternion qnorm = rot;
		qnorm.normalize();

		// Note the order of the mult, i.e. q comes after
		m_localTransform.m_quat = m_localTransform.m_quat * qnorm;

		needUpdate();
	}

	void Node::setLocalScaling(const Vector3& scl)
	{
		m_localTransform.m_scale = scl;
		needUpdate();
	}

	void Node::setLocalScalingXYZ(Real x, Real y, Real z)
	{
		Vector3 scl(x, y, z);
		setLocalScaling(scl);
	}

	void Node::setLocalOrientation(const Quaternion& ort)
	{
		m_localTransform.m_quat = ort;
		m_localTransform.m_quat.normalize();

		needUpdate();
	}

	void Node::setLocalPosition(const Vector3& pos)
	{
		m_localTransform.m_pos = pos;
		needUpdate();
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
        
        m_worldTransform.m_pos = pos;
	}

	void Node::setWorldPositionX(float x)
	{
		Vector3 position = getWorldPosition();
		position.x = x;
		setWorldPosition(position);
	}

	void Node::setWorldPositionY(float y)
	{
		Vector3 position = getWorldPosition();
		position.y = y;
		setWorldPosition(position);
	}

	void Node::setWorldPositionZ(float z)
	{
		Vector3 position = getWorldPosition();
		position.z = z;
		setWorldPosition(position);
	}

	Node* Node::getParent() const
	{
		return m_parent;
	}

	Node* Node::getChildByIndex(ui32 idx)
	{
		return idx<m_children.size() ? *std::next(m_children.begin(), idx) : nullptr;
	}

	Node* Node::getChild(const char* name)
	{
		for (Node* child : m_children)
		{
			if (child->getName() == name)
				return child;
		}

		return nullptr;
	}

	i32 Node::getChildIdx(Node* node)
	{
		size_t pos = std::find(m_children.begin(), m_children.end(), node) - m_children.begin();
		return pos > m_children.size() ? -1 : static_cast<i32>(pos);
	}

	void Node::setParent(Node* parent)
	{
		if (parent)
		{
			parent->insertChild(parent->getChildNum(), this);
		}
		else
		{
			this->remove();
			m_parent = nullptr;
			needUpdate();
		}
	}

	void Node::insertChild(ui32 idx, Node* node)
	{
		node->remove();

		// make sure the name is unique in current layer
		int    id = 1;
		String name = node->getName();
		while (isChildExist(node->getName()))
		{
			id++;
			node->setName(StringUtil::Format("%s%d", name.c_str(), id));
		}

		node->m_parent = this;
		m_children.insert(m_children.begin() + idx, node);

		needUpdate();
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
		return m_localTransform.m_scale;
	}

	const Quaternion& Node::getLocalOrientation() const
	{
		return m_localTransform.m_quat;
	}

	const Vector3 Node::getLocalYawPitchRoll()
	{
		Vector3 yawpitchroll;
		m_localTransform.m_quat.toEulerAngle(yawpitchroll.x, yawpitchroll.y, yawpitchroll.z);

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
		return m_localTransform.m_pos;
	}

	const Vector3& Node::getWorldScaling() const
	{
		return m_worldTransform.m_scale;
	}

	const Quaternion& Node::getWorldOrientation() const
	{
		return m_worldTransform.m_quat;
	}

	const Vector3& Node::getWorldPosition() const
	{
		return m_worldTransform.m_pos;
	}
	
	const Matrix4& Node::getWorldMatrix()
	{
		if (m_isTransformDirty)
		{
			// build mat world matrix
			m_worldTransform = m_parent ? m_parent->getWorldTransform() * m_localTransform : m_localTransform;
			m_worldTransform.buildMatrix(m_matWorld);
			m_isTransformDirty = false;
		}

		return m_matWorld;
	}

	void Node::buildWorldAABB(AABB& aabb)
	{
		aabb = m_localAABB.isValid() ? m_localAABB : AABB( -0.3f, -0.3f, -0.3f, 0.3f, 0.3f, 0.3f);
		aabb = aabb.transform(getWorldMatrix());
	}

	Matrix4 Node::getInverseWorldMatrix() const
	{
		Matrix4 invMat;
		m_worldTransform.buildInvMatrix(invMat);

		return invMat;
	}

	void Node::convertWorldToLocalPosition(Vector3& posLocal, const Vector3& posWorld)
	{
		Quaternion ortWorldInv = m_worldTransform.m_quat;
		ortWorldInv.inverse();
		posLocal = ortWorldInv * (posWorld - m_worldTransform.m_pos) / m_worldTransform.m_scale;
	}

	void Node::convertWorldToLocalOrientation(Quaternion& ortLocal, const Quaternion& ortWorld)
	{
		Quaternion ortWorldInv = m_worldTransform.m_quat;
		ortWorldInv.inverse();

		ortLocal = ortWorldInv * ortWorld;
	}

	void Node::setScript(const ResourcePath& path)
	{
		m_script.m_file.setPath(path.getPath());
	}

	// register to script
	void Node::registerToScript()
	{
		if (!m_script.m_isRegistered)
		{
			m_script.m_globalTableName = StringUtil::Format("objs._%d", this->getId());
			LuaBinder::instance()->registerObject( getClassName(), m_script.m_globalTableName.c_str(), this);

			m_script.m_isRegistered = true;
		}
	}
    
    void Node::callLuaFunction(const String& funName, const Variant** args, int argCount)
    {
		if (Engine::instance()->getConfig().m_isGame)
		{
			registerToScript();

			String objectFun = StringUtil::Format("nodes._%d.%s", this->getId(), funName.c_str());
			LuaBinder::instance()->call<void>(objectFun.c_str());
		}
    }

	void Node::needUpdate()
	{
		if (m_isTransformDirty)
			return;

		m_isTransformDirty = true;

		for (Node* node : m_children)
		{
			node->needUpdate();
		}
	}

	void Node::update(float delta, bool bUpdateChildren)
	{
		if (!m_isEnable)
			return;

		// update world matrix
		getWorldMatrix();

		// script update
		m_script.update(this);

		update_self();

#ifdef ECHO_EDITOR_MODE
		if(m_objectEditor)
			m_objectEditor->editor_update_self();
#endif

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
		BIND_METHOD(Node::load,							DEF_METHOD("Node.load"));
	
		CLASS_BIND_METHOD(Node, queueFree,				DEF_METHOD("queueFree"));
		CLASS_BIND_METHOD(Node, getNode,				DEF_METHOD("getNode"));
		CLASS_BIND_METHOD(Node, addChild, 				DEF_METHOD("addChild"));
		CLASS_BIND_METHOD(Node, setParent,				DEF_METHOD("setParent"));
		CLASS_BIND_METHOD(Node, getChildNum,			DEF_METHOD("getChildNum"));
		CLASS_BIND_METHOD(Node, getChildByIndex,		DEF_METHOD("getChildByIndex"));
		CLASS_BIND_METHOD(Node, getWorldPosition,		DEF_METHOD("getWorldPosition"));
		CLASS_BIND_METHOD(Node, getWorldPositionX,		DEF_METHOD("getWorldPositionX"));
		CLASS_BIND_METHOD(Node, getWorldPositionY,		DEF_METHOD("getWorldPositionY"));
		CLASS_BIND_METHOD(Node, getWorldPositionZ,		DEF_METHOD("getWorldPositionZ"));
		CLASS_BIND_METHOD(Node, setWorldPosition,		DEF_METHOD("setWorldPosition"));
		CLASS_BIND_METHOD(Node, setWorldPositionX,		DEF_METHOD("setWorldPositionX"));
		CLASS_BIND_METHOD(Node, setWorldPositionY,		DEF_METHOD("setWorldPositionY"));
		CLASS_BIND_METHOD(Node, setWorldPositionZ,		DEF_METHOD("setWorldPositionZ"));
		CLASS_BIND_METHOD(Node, getLocalYawPitchRoll,	DEF_METHOD("getLocalYawPitchRoll"));
		CLASS_BIND_METHOD(Node, getLocalScaling,		DEF_METHOD("getLocalScaling"));
		CLASS_BIND_METHOD(Node, setLocalScaling,		DEF_METHOD("setLocalScaling"));
		CLASS_BIND_METHOD(Node, getLocalPosition,		DEF_METHOD("getLocalPosition"));
		CLASS_BIND_METHOD(Node, setLocalPosition,		DEF_METHOD("setLocalPosition"));
		CLASS_BIND_METHOD(Node, setLocalYawPitchRoll,	DEF_METHOD("setLocalYawPitchRoll"));
		CLASS_BIND_METHOD(Node, setEnable,				DEF_METHOD("setEnable"));
		CLASS_BIND_METHOD(Node, isEnable,				DEF_METHOD("isEnable"));
		CLASS_BIND_METHOD(Node, setScript,				DEF_METHOD("setScript"));
		CLASS_BIND_METHOD(Node, getScript,				DEF_METHOD("getScript"));
		CLASS_BIND_METHOD(Node, getPropertyValue,		DEF_METHOD("getPropertyValue"));
		CLASS_BIND_METHOD(Node, ch,						DEF_METHOD("ch"));

		CLASS_REGISTER_PROPERTY(Node, "Enable", Variant::Type::Bool, "isEnable", "setEnable");
		CLASS_REGISTER_PROPERTY(Node, "Position", Variant::Type::Vector3, "getLocalPosition", "setLocalPosition");
		CLASS_REGISTER_PROPERTY(Node, "Rotation", Variant::Type::Vector3, "getLocalYawPitchRoll", "setLocalYawPitchRoll");
		CLASS_REGISTER_PROPERTY(Node, "Scale",  Variant::Type::Vector3, "getLocalScaling", "setLocalScaling");
		CLASS_REGISTER_PROPERTY(Node, "Script", Variant::Type::ResourcePath, "getScript", "setScript");
	}

	// get node by path
	Node* Node::getNode(const char* path)
	{
		if (path)
		{
			int pathLen = static_cast<int>(strlen(path));
			if (!pathLen)
			{
				this->registerToScript();
				return this;
			}
			else
			{
				if (path[0] == '/')
				{
					Node* root = this;
					while (root->getParent())
					{
						root = root->getParent();
					}

					return root->getNode( path+1);
				}
				else if (path[0] == '.' && path[1] == '.' && path[2] == '/')
				{
					return getParent() ? getParent()->getNode(path + 3) : nullptr;
				}
				else
				{
					bool haveGrandson = false;
					char firstName[256];
					for (int i = 0; i <= pathLen; i++)
					{
						if (*path == '/')
						{
							firstName[i] = '\0';
							path++;
							haveGrandson = true;
							break;
						}
						else
						{
							firstName[i] = *path;
							path++;
						}
					}

					Node* child = getChild(firstName);
					if (child)
					{
						if (haveGrandson)
						{
							return child->getNode(path);
						}
						else
						{
							child->registerToScript();
							return child;
						}
					}
				}
			}
		}
			
		return nullptr;
	}

	// build node path
	String Node::getNodePath() const
	{
		String result;

		// ignore invisible root node
		const Node* node = this;
		while (node && node->getParent())
		{
			result = "/" + node->getName() + result;

			node = node->getParent();
		}

		return result;
	}

	// get relative path
	String Node::getNodePathRelativeTo(const Node* baseNode) const
	{
		int         baseNodeListCount = 0;
		const Node* baseNodeList[256];
		int         curNodeListCount = 0;
		const Node* curNodeList[256];

		// get base node path list
		const Node* node = baseNode;
		while (node)
		{
			baseNodeList[baseNodeListCount++] = node;
			node = node->getParent();
		}

		// get cur node path list
		node = this;
		while (node)
		{
			curNodeList[curNodeListCount++] = node;
			node = node->getParent();
		}

		// get the same parent
        int i = baseNodeListCount-1;
        int j = curNodeListCount-1;
		for (; i >= 0 && j >= 0; i--, j--)
		{
			if(baseNodeList[i] != curNodeList[j])
				break;
		}

		String result;
		while (i > -1)
		{
			result += "../";
			i--;
		}

		while (j > -1)
		{
			result += curNodeList[j]->getName() + (j==0 ? "" : "/");
			j--;
		}

		return result;
	}

	// queue free
	void Node::queueFree()
	{
		if (m_parent)
		{
			m_parent->removeChild(this);
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

	Variant Node::getPropertyValue(const String& propertyName)
	{
		Variant value;
		Class::getPropertyValue(this, propertyName, value);
		return value;
	}

	// ch
	Variant Node::ch(const String& path, const String& propertyName)
	{
		Node* targetNode = getNode(path.c_str());
		return targetNode ? targetNode->getPropertyValue(propertyName) : Variant();
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
        saveSignalSlotConnects(pugiNode, node, node->getClassName());
        saveChannels(pugiNode, node);

		if (recursive)
		{
			for (ui32 idx = 0; idx < node->getChildNum(); idx++)
			{
				Node* child = node->getChildByIndex(idx);
				if (child && !child->isLink())
				{
					pugi::xml_node newNode = xmlNode->append_child("node");
					saveXml(&newNode, child, true);
				}
			}
		}
	}

	// instance
	Node* Node::load(const char* path)
	{
		Node* result = loadLink(path, false);
		result->registerToScript();

		return result;
	}

	// load
	Node* Node::loadLink(const String& path, bool isLink)
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
						rootNode->getChildByIndex(idx)->setLink(true);
					}
				}
				rootNode->registerToScript();
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
			Node* node = path.empty() ? ECHO_DOWN_CAST<Node*>(instanceObject(pugiNode)) : loadLink(path, true);
			if (node)
			{
				if (!path.empty())
				{
					// overwrite property
					loadPropertyRecursive(xmlNode, node, node->getClassName());
                    loadSignalSlotConnects(xmlNode, node, node->getClassName());
                    loadChannels(xmlNode, node);
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
