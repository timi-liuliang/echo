#include "TextureCube.h"

namespace Echo
{
	TextureCube::TextureCube()
		: Texture()
	{
		m_surfaces.assign(ResourcePath("", ".png"));
	}

	TextureCube::TextureCube(const String& name)
		: Texture(name)
	{
		m_surfaces.assign(ResourcePath("", ".png"));
	}

	TextureCube::~TextureCube()
	{

	}

	// bind methods to script
	void TextureCube::bindMethods() 
	{
		CLASS_BIND_METHOD(TextureCube, getPositiveX, DEF_METHOD("getPositiveX"));
		CLASS_BIND_METHOD(TextureCube, setPositiveX, DEF_METHOD("setPositiveX"));
		CLASS_BIND_METHOD(TextureCube, getNegativeX, DEF_METHOD("getNegativeX"));
		CLASS_BIND_METHOD(TextureCube, setNegativeX, DEF_METHOD("setNegativeX"));

		CLASS_BIND_METHOD(TextureCube, getPositiveY, DEF_METHOD("getPositiveY"));
		CLASS_BIND_METHOD(TextureCube, setPositiveY, DEF_METHOD("setPositiveY"));
		CLASS_BIND_METHOD(TextureCube, getNegativeY, DEF_METHOD("getNegativeY"));
		CLASS_BIND_METHOD(TextureCube, setNegativeY, DEF_METHOD("setNegativeY"));

		CLASS_BIND_METHOD(TextureCube, getPositiveZ, DEF_METHOD("getPositiveZ"));
		CLASS_BIND_METHOD(TextureCube, setPositiveZ, DEF_METHOD("setPositiveZ"));
		CLASS_BIND_METHOD(TextureCube, getNegativeZ, DEF_METHOD("getNegativeZ"));
		CLASS_BIND_METHOD(TextureCube, setNegativeZ, DEF_METHOD("setNegativeZ"));

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveX", Variant::Type::ResourcePath, "getPositiveX", "setPositiveX");
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeX", Variant::Type::ResourcePath, "getNegativeX", "setNegativeX");

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveY", Variant::Type::ResourcePath, "getPositiveY", "setPositiveY");
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeY", Variant::Type::ResourcePath, "getNegativeY", "setNegativeY");

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveZ", Variant::Type::ResourcePath, "getPositiveZ", "setPositiveZ");
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeZ", Variant::Type::ResourcePath, "getNegativeZ", "setNegativeZ");
	}

	// positive x
	void TextureCube::setPositiveX(const ResourcePath& path)
	{
		m_surfaces[0].setPath(path.getPath());
	}

	// positive x
	void TextureCube::setNegativeX(const ResourcePath& path)
	{
		m_surfaces[1].setPath(path.getPath());
	}

	// positive x
	void TextureCube::setPositiveY(const ResourcePath& path)
	{
		m_surfaces[2].setPath(path.getPath());
	}

	// positive x
	void TextureCube::setNegativeY(const ResourcePath& path)
	{
		m_surfaces[3].setPath(path.getPath());
	}

	// positive x
	void TextureCube::setPositiveZ(const ResourcePath& path)
	{
		m_surfaces[4].setPath(path.getPath());
	}

	// positive x
	void TextureCube::setNegativeZ(const ResourcePath& path)
	{
		m_surfaces[5].setPath(path.getPath());
	}
}