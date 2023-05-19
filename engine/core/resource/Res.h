#pragma once

#include "ResRef.h"
#include "ResourcePath.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class Res : public Object
	{
		ECHO_CLASS(Res, Object)

	public:
		typedef Res*(*RES_CREATE_FUNC)();
		typedef Res*(*RES_LOAD_FUNC)(const ResourcePath&);

		struct ResFun
		{
			String				m_class;
			String				m_ext;
			RES_CREATE_FUNC		m_cfun = nullptr;
			RES_LOAD_FUNC		m_lfun = nullptr;
		};

	public:
		Res();
		Res(const ResourcePath& path);
		virtual ~Res();

		// set path
		virtual void setPath(const String& path) override;
		virtual const String& getPath() const override { return m_path.getPath(); }

		// resister res
		static void registerRes(const String& className, const String& exts, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun);

		// get res
		static Res* get(const ResourcePath& path, bool createNewIfNotExist=true);
		static std::unordered_map<String, Res*>& getAll();

		// create by extension
		static ResRef<Res> createByFileExtension(const String& extWithDot, bool ignoreError);

		// get res fun by extension
		static const ResFun* getResFunByExtension(const String& extWithDot);

		// get res fun by class
		static const ResFun* getResFunByClassName(const String& className);

		// update
		static void updateAll(float delta);

		// clear
		static void clear();

	public:
		// is a package
		virtual bool isPackage() const { return false; }

		// enum files
		virtual void enumFilesInDir(StringArray& ret, const String& rootPath, bool bIncDir = false, bool bIncSubDirs = false, bool bAbsPath = false) {}

	public:
		// ref count
		void addRefCount() { m_refCount++; }
		void subRefCount();

		// is loaded succeed
		bool isLoaded() const { return m_isLoaded; }

		// On device lost
		virtual void onLostDevice() {}
		virtual void onResetDevice() {}
	
		// save
		virtual void save();

	protected:
		// cerate fun
		template<typename T> static Res* create() { return EchoNew(T); }

		// load
		static Res* load(const ResourcePath& path);

		// Instance res
		static Res* Res::instanceRes(void* pugiNode, const ResourcePath& path);

	protected:
		int								m_refCount;
		bool							m_isLoaded;
		ResourcePath					m_path;
	};
	typedef ResRef<Res> ResPtr;

	// res factory
	template<typename T>
	struct ResFactoryT : public ObjectFactory
	{
		Res::RES_CREATE_FUNC	m_cfun;

		ResFactoryT(const String& name, const String& parent, Res::RES_CREATE_FUNC cfun)
		{
			// register class to lua
			LuaBinder::instance()->registerClass(name.c_str(), parent.c_str());

			m_name = name;
			m_classInfo.m_singleton = false;
			m_classInfo.m_virtual = false;
			m_classInfo.m_module = getCurrentRegisterModuleName();
			m_classInfo.m_parent = parent;
			m_cfun = cfun;

			Class::addClass(name, this);
			T::bindMethods();
		}

		virtual Object* create()
		{
			Object* obj = m_cfun ? (*m_cfun)() : EchoNew(T);

			return obj;
		}

		virtual Object* getDefaultObject()
		{
			return nullptr;
		}
	};
}

#define  ECHO_CREATE_RES(T) (T*)Echo::Class::create(#T);

#define ECHO_RES(m_class, m_parent, extension, CREATE_FUNC, LOAD_FUNC)							\
public:																							\
	virtual const String& getClassName() const override											\
	{																							\
		static String className=#m_class;														\
		return className;																		\
	}																							\
																								\
	static void initClassInfo()																	\
	{																							\
		static Echo::ResFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent, CREATE_FUNC);	\
		Echo::Res::registerRes( #m_class,extension, CREATE_FUNC, LOAD_FUNC);					\
	}																							\
																								\
	static void bindMethods();																	\
																								\
private:		
