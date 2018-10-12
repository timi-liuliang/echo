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

		// resister res
		static void registerRes(const String& className, const String& exts, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun);

		// get res
		static Res* get(const ResourcePath& path);

		// create by extension
		static ResRef<Res> createByFileExtension(const String& extWithDot);

		// get res fun by extension
		static const ResFun* getResFunByExtension(const String& extWithDot);

		// get res fun by class
		static const ResFun* getResFunByClassName(const String& className);

	public:
		// add ref count
		void addRefCount() { m_refCount++; }

		// release
		void subRefCount();

		// is loaded succeed
		bool isLoaded() const { return m_isLoaded; }
	
		// save
		virtual void save();

	protected:
		// cerate fun
		template<typename T> static Res* create() { return EchoNew(T); }

		// load
		static Res* load(const ResourcePath& path);

	protected:
		int								m_refCount;
		bool							m_isLoaded;
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
			m_classInfo.m_parent = parent;
			m_cfun = cfun;

			Class::addClass(name, this);
			T::bindMethods();
		}

		virtual Object* create()
		{
			Object* obj = m_cfun ? (*m_cfun)() : EchoNew(T);

#ifdef ECHO_EDITOR_MODE
			initEditor(obj);
#endif

			return obj;
		}
	};
}

#define  ECHO_CREATE_RES(T) (T*)Echo::Class::create(#T);

#define ECHO_RES(m_class, m_parent, extension, CREATE_FUNC, LOAD_FUNC)							\
public:																							\
	virtual const String& getClassName() const													\
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