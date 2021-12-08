#include "texture_cube.h"
#include "engine/core/render/base/renderer.h"

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

	Res* TextureCube::create()
	{
		return Renderer::instance()->createTextureCube("");
	}

	void TextureCube::bindMethods() 
	{
		CLASS_BIND_METHOD(TextureCube, getPositiveX);
		CLASS_BIND_METHOD(TextureCube, setPositiveX);
		CLASS_BIND_METHOD(TextureCube, getNegativeX);
		CLASS_BIND_METHOD(TextureCube, setNegativeX);

		CLASS_BIND_METHOD(TextureCube, getPositiveY);
		CLASS_BIND_METHOD(TextureCube, setPositiveY);
		CLASS_BIND_METHOD(TextureCube, getNegativeY);
		CLASS_BIND_METHOD(TextureCube, setNegativeY);

		CLASS_BIND_METHOD(TextureCube, getPositiveZ);
		CLASS_BIND_METHOD(TextureCube, setPositiveZ);
		CLASS_BIND_METHOD(TextureCube, getNegativeZ);
		CLASS_BIND_METHOD(TextureCube, setNegativeZ);

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveX", Variant::Type::ResourcePath, getPositiveX, setPositiveX);
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeX", Variant::Type::ResourcePath, getNegativeX, setNegativeX);

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveY", Variant::Type::ResourcePath, getPositiveY, setPositiveY);
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeY", Variant::Type::ResourcePath, getNegativeY, setNegativeY);

		CLASS_REGISTER_PROPERTY(TextureCube, "PositiveZ", Variant::Type::ResourcePath, getPositiveZ, setPositiveZ);
		CLASS_REGISTER_PROPERTY(TextureCube, "NegativeZ", Variant::Type::ResourcePath, getNegativeZ, setNegativeZ);
	}

	void TextureCube::setPositiveX(const ResourcePath& path)
	{
		m_surfaces[0].setPath(path.getPath());

		load();
	}

	void TextureCube::setNegativeX(const ResourcePath& path)
	{
		m_surfaces[1].setPath(path.getPath());

		load();
	}

	void TextureCube::setPositiveY(const ResourcePath& path)
	{
		m_surfaces[2].setPath(path.getPath());

		load();
	}

	void TextureCube::setNegativeY(const ResourcePath& path)
	{
		m_surfaces[3].setPath(path.getPath());

		load();
	}

	void TextureCube::setPositiveZ(const ResourcePath& path)
	{
		m_surfaces[4].setPath(path.getPath());

		load();
	}

	void TextureCube::setNegativeZ(const ResourcePath& path)
	{
		m_surfaces[5].setPath(path.getPath());

		load();
	}
}