#include "scratch.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	Scratch::Scratch()
	{

	}

	Scratch::Scratch(const ResourcePath& path)
		: Res(path)
	{

	}

	Scratch::~Scratch()
	{

	}

	void Scratch::bindMethods()
	{
		CLASS_BIND_METHOD(Scratch, getParentClass, DEF_METHOD("getParentClass"));
		CLASS_BIND_METHOD(Scratch, setParentClass, DEF_METHOD("setParentClass"));

		CLASS_REGISTER_PROPERTY(Scratch, "Parent", Variant::Type::String, "getParentClass", "setParentClass");
	}

	Res* Scratch::load(const ResourcePath& path)
	{
		MemoryReader reader(path.getPath());
		if (reader.getSize())
		{
			Scratch* res = EchoNew(Scratch(path));
			return res;
		}

		return nullptr;
	}
}