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
		static void registerRes(const String& className, const String& ext, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun);

		// get res
		static Res* get(const ResourcePath& path);

		// create by extension
		static ResRef<Res> createByFileExtension(const String& extension);

		// create by class name
		static ResRef<Res> createByClassName(const String& className);

		// get res fun by class
		static const ResFun* getResFunByClassName(const String& className);

	public:
		// add ref count
		void addRefCount() { m_refCount++; }

		// release
		void subRefCount();

		// is loaded succeed
		bool isLoaded() const { return m_isLoaded; }

		// get path
		const ResourcePath& getPath() const { return m_path; }

		// set path
		void setPath(const ResourcePath& path) { m_path = path; }
	
		// save
		virtual void save();

	protected:
		// cerate fun
		template<typename T>
		static Res* create() { return EchoNew(T); }

		// load
		static Res* load(const ResourcePath& path);

	protected:
		int								m_refCount;
		ResourcePath					m_path;
		bool							m_isLoaded;
	};
	typedef ResRef<Res> ResPtr;

	INLINE ResRef<Res> Res::createByClassName(const String& className)
	{
		const ResFun* resFun = getResFunByClassName(className);
		if (resFun)
		{
			Res* res = resFun->m_cfun();
			if (res)
				return res;
		}

		EchoLogError("Res::create failed. Unknown class [%s]", className.c_str());
		return nullptr;
	}
}

#define  ECHO_CREATE_RES(T) (T*)Echo::Res::createByClassName(#T).ptr();

#define ECHO_RES(m_class, m_parent, extension, CREATE_FUNC, LOAD_FUNC)				\
public:																				\
	virtual const String& getClassName() const										\
	{																				\
		static String className=#m_class;											\
		return className;															\
	}																				\
																					\
	static void initClassInfo()														\
	{																				\
		static Echo::ObjectFactoryT<m_class> G_OBJECT_FACTORY(#m_class, #m_parent); \
		Echo::Res::registerRes( #m_class,extension, CREATE_FUNC, LOAD_FUNC);		\
	}																				\
																					\
	static void bindMethods();														\
																					\
private:		