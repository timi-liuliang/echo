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

	void Scratch::save(const char* pathName)
	{
		const char* content = "";
		if (content)
		{
			String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());
			std::ofstream f(fullPath.c_str());

			f << content;

			f.flush();
			f.close();
		}
	}
}