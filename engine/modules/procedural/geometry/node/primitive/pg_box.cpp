#include "pg_box.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PGBox::PGBox()
	{

	}

	PGBox::~PGBox()
	{

	}

	void PGBox::bindMethods()
	{
		CLASS_BIND_METHOD(PGBox, getSize, DEF_METHOD("getSize"));
		CLASS_BIND_METHOD(PGBox, setSize, DEF_METHOD("setSize"));

		CLASS_REGISTER_PROPERTY(PGBox, "Size", Variant::Type::Real, "getSize", "setSize");
	}

	void PGBox::setSize(const Vector3& size)
	{
		m_size = size;
		m_dirtyFlag = true;
	}

	void PGBox::play(PCGData& data)
	{
		m_dirtyFlag = false;
	}
}
