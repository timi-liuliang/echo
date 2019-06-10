#include "signal.h"
#include "class.h"
#include "variant.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	void ConnectClassMethod::emitSignal(const Variant** args, int argCount)
	{
		Variant::CallError error;
		m_method->call(m_target,  args, argCount, error);
	}
    
    void ConnectLuaMethod::save(void* pugiNode)
    {
        pugi::xml_node* signalNode = (pugi::xml_node*)pugiNode;
        
        pugi::xml_node connectNode = signalNode->append_child("connect");
        connectNode.append_attribute("target").set_value(m_targetPath.c_str());
        connectNode.append_attribute("method").set_value(m_functionName.c_str());
    }
    
    void ConnectLuaMethod::emitSignal(const Variant** args, int argCount)
    {
        if(m_target)
            m_target->callLuaFunction( m_functionName, args, argCount);
    }

	bool Signal::connectClassMethod(Object* obj, ClassMethodBind* method)
	{
		m_connects.push_back(EchoNew(ConnectClassMethod(this, obj, method)));

		return true;
	}
    
    bool Signal::connectLuaMethod(const String& obj, const Echo::String& luaMethodName)
    {
        m_connects.push_back(EchoNew(ConnectLuaMethod(this, obj, luaMethodName)));
        
        return true;
    }
    
    void Signal::load(void* pugiNode)
    {
        pugi::xml_node* signalNode = (pugi::xml_node*)pugiNode;
        
        for (pugi::xml_node connectNode = signalNode->child("connect"); connectNode; connectNode = connectNode.next_sibling("connect"))
        {
            String target = connectNode.attribute("target").as_string();
            String method = connectNode.append_attribute("method").as_string();
            
            connectLuaMethod( target, method);
        }
    }
    
    void Signal::save(void* pugiNode)
    {
        for(Connect* conn : m_connects)
        {
            conn->save(pugiNode);
        }
    }
}
