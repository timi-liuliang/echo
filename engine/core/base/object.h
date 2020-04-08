#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/base/class.h"
#include "engine/core/base/property_info.h"
#include "engine/core/base/channel.h"
#include "engine/core/editor/object_editor.h"

namespace Echo
{
	class Object
	{
		ECHO_EDITOR_INTERFACE
															
	public:
		Object();
		virtual ~Object();

		// get by id
		static Object* getById(i32 id);

		// is valid
		bool isValid();

		// get id
		i32 getId() const { return m_id; }

		// path
		const String& getPath() const { return m_path.getPath(); }
		virtual void setPath(const String& path) { m_path.setPath(path); }
        
        // register to script
        bool isRegisteredToScript() { return m_isRegisteredToScript; }
        virtual void registerToScript();
        virtual void unregisterFromScript();

		// free this object from memory
		virtual void queueFree() { ECHO_DELETE_T(this, Object); }

	public:
		// properties (script property or dynamic property)
		virtual const PropertyInfos& getPropertys();

		// clear properties
		void clearPropertys();

		// register property
		bool registerProperty(const String& className, const String& propertyName, const Variant::Type type, const PropertyHintArray& hints=PropertyHintArray());

		// property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar) { return false; }
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue) { return false; }

		// property flags
		virtual i32 getPropertyFlag(const String& propertyName) { return PropertyFlag::All; }

	public:
		// connect|disconnect signal slot
		static bool connect(Object* from, const char* signalName, Object* to, const char* luaFunName);
		static bool disconnect(Object* from, const char* signalName, Object* to, const char* luaFunName);

		// call lua function of this node
		virtual void callLuaFunction(const String& funName, const Variant** args, int argCount) {}
        
    public:
        // get channels
		Channel* getChannel(const String& propertyName);
        ChannelsPtr getChannels() { return m_chanels; }
        
        // clear channels
        void unregisterChannel(const String& propertyName);
        void unregisterChannels();
        
        // is channel exist
        bool isChannelExist(const String& propertyName);
        
        // register channel
        bool registerChannel(const String& propertyName, const String& expression);

	public:
		// instance object
		static Object* instanceObject(void* pugiNode);

		// load/save property recursive
        static void loadPropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);
		static void savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

		// load properties value
		static void loadPropertyValue(void* pugiNode, Echo::Object* classPtr, const Echo::String& className, i32 flag);
        
        // load/save signal slot connects
        static void loadSignalSlotConnects(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);
        static void saveSignalSlotConnects(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);
        
        // load/save channels
        static void loadChannels(void* pugiNode, Echo::Object* classPtr);
        static void saveChannels(void* pugiNode, Echo::Object* classPtr);

	public:
		// get class name
		virtual const String& getClassName() const;

		// init class info
		static void initClassInfo() { static Echo::ObjectFactoryT<Object> G_OBJECT_FACTORY("Object", ""); }

		// bind methods
		static void bindMethods();

	protected:
		i32				m_id;
		ResourcePath	m_path;
		PropertyInfos	m_propertys;
        ChannelsPtr     m_chanels = nullptr;
        bool			m_isRegisteredToScript = false;
	};
}
