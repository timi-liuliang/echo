#include "signal.h"
#include "class.h"
#include "variant.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include "engine/core/scene/node.h"

namespace Echo
{
	ConnectClassMethod::ConnectClassMethod(Signal* signal, Object* target, ClassMethodBind* method)
		: m_signal(signal)
		, m_target(target)
		, m_method(method)
	{}

	void ConnectClassMethod::emitSignal(const Variant** args, int argCount)
	{
		Variant::CallError error;
		m_method->call(m_target,  args, argCount, error);
	}


	ConnectLuaMethod::ConnectLuaMethod(Signal* signal, Object* target, const String& functionName)
		: m_signal(signal)
		, m_functionName(functionName)
        , m_target(target)
	{}

	ConnectLuaMethod::ConnectLuaMethod(Signal* signal, const String& target, const String& functionName)
		: m_signal(signal)
		, m_targetPath(target)
		, m_functionName(functionName)
	{}
    
    void ConnectLuaMethod::save(void* pugiNode)
    {
        pugi::xml_node* signalNode = (pugi::xml_node*)pugiNode;
        
        pugi::xml_node connectNode = signalNode->append_child("connect");
        connectNode.append_attribute("target").set_value(m_targetPath.c_str());
        connectNode.append_attribute("method").set_value(m_functionName.c_str());
    }
    
    void ConnectLuaMethod::emitSignal(const Variant** args, int argCount)
    {
        buildTarget();
        
		if (m_target)
		{
			if (m_target->isValid())
				m_target->callLuaFunction(m_functionName, args, argCount);
			else
				m_signal->disconnect(this);
		}
    }
    
    void ConnectLuaMethod::buildTarget()
    {
        if(!m_target && m_signal)
        {
            Node* owner = ECHO_DOWN_CAST<Node*>(m_signal->getOwner());
            if(owner)
            {
                m_target = owner->getNode(m_targetPath.c_str());
            }
        }
    }
    
    Signal::Signal(Object* owner)
        : m_owner(owner)
    {
    }
    
    Signal::~Signal()
    {
		disconnectAll();
    }

	bool Signal::connectClassMethod(Object* obj, ClassMethodBind* method)
	{
        if(!m_connects)
            m_connects = new vector<Connect*>::type;
            
		m_connects->push_back(EchoNew(ConnectClassMethod(this, obj, method)));

		return true;
	}

	bool Signal::connectLuaMethod(Object* obj, const Echo::String& luaMethodName)
	{
		if (!m_connects)
			m_connects = new vector<Connect*>::type;

		m_connects->push_back(EchoNew(ConnectLuaMethod(this, obj, luaMethodName)));

		return true;
	}
    
    bool Signal::connectLuaMethod(const String& obj, const Echo::String& luaMethodName)
    {
        if(!m_connects)
            m_connects = new vector<Connect*>::type;
        
        m_connects->push_back(EchoNew(ConnectLuaMethod(this, obj, luaMethodName)));
        
        return true;
    }

	void Signal::disconnect(Connect* connect)
	{
		if (m_connects)
		{
			for (vector<Connect*>::type::iterator it = m_connects->begin(); it != m_connects->end(); it++)
			{
				Connect* curConn = *it;
				if (curConn == connect)
				{
					EchoSafeDelete(curConn, Connect);
					m_connects->erase(it);
					break;
				}
			}
		}
	}
    
	void Signal::disconnectAll()
	{
		if (m_connects)
		{
			for (Connect* conn : *m_connects)
				EchoSafeDelete(conn, Connect);

			delete m_connects; m_connects = nullptr;
		}
	}

	void Signal::disconnectLuaMethod(Object* obj, const String& luaMethodName)
	{
		if (m_connects)
		{
			for (vector<Connect*>::type::iterator it = m_connects->begin(); it != m_connects->end(); it++)
			{
				ConnectLuaMethod* luaConn = ECHO_DOWN_CAST<ConnectLuaMethod*>(*it);
				if (luaConn && luaConn->m_target == obj && luaConn->m_functionName == luaMethodName)
				{
					EchoSafeDelete(luaConn, ConnectLuaMethod);
					m_connects->erase(it);
					break;
				}
			}
		}
	}

	void Signal::disconnectLuaMethod(const String& obj, const Echo::String& luaMethodName)
	{
		if (m_connects)
		{
			for (vector<Connect*>::type::iterator it = m_connects->begin(); it != m_connects->end(); it++)
			{
				ConnectLuaMethod* luaConn = ECHO_DOWN_CAST<ConnectLuaMethod*>(*it);
				if (luaConn && luaConn->m_targetPath == obj && luaConn->m_functionName == luaMethodName)
				{
					EchoSafeDelete(luaConn, ConnectLuaMethod);
					m_connects->erase(it);
					break;
				}
			}
		}
	}

    void Signal::load(void* pugiNode)
    {
        pugi::xml_node* signalNode = (pugi::xml_node*)pugiNode;
        
        for (pugi::xml_node connectNode = signalNode->child("connect"); connectNode; connectNode = connectNode.next_sibling("connect"))
        {
            String target = connectNode.attribute("target").as_string();
            String method = connectNode.attribute("method").as_string();
            
            connectLuaMethod( target, method);
        }
    }
    
    void Signal::save(void* pugiNode)
    {
        if(m_connects)
        {
            for(Connect* conn : *m_connects)
            {
                conn->save(pugiNode);
            }
        }
    }
}
