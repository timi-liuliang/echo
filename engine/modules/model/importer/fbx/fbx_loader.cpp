#include "fbx_loader.h"

#ifdef ECHO_EDITOR_MODE
namespace Fbx
{
	Loader::Loader()
	{
	}

	Loader::~Loader()
	{

	}

	bool Loader::load(const ResourcePath& path)
	{
		m_path = path;

		return false;
	}
}
#endif
