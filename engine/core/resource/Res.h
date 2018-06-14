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
			RES_CREATE_FUNC		m_cfun = nullptr;
			RES_LOAD_FUNC		m_lfun = nullptr;
		};

	public:
		Res();
		Res(const ResourcePath& path);
		virtual ~Res();

		// resister res
		static void registerRes(const String& ext, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun);

		// get res
		static Res* get(const ResourcePath& path);

		// create by extension
		static ResRef<Res> create(const String& extension);

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
		void save();

	protected:
		// cerate fun
		template<typename T>
		static Res* create() { return EchoNew(T); }

		// load
		static Res* load(const ResourcePath& path);

		// save all property
		void savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className);

	protected:
		int								m_refCount;
		ResourcePath					m_path;
		bool							m_isLoaded;
	};
	typedef ResRef<Res> ResPtr;
}

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
		Echo::Res::registerRes(extension, CREATE_FUNC, LOAD_FUNC);					\
	}																				\
																					\
	static void bindMethods();														\
																					\
private:		