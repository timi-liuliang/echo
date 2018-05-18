#include "gltf_ibl.h"
#include "engine/core/render/TextureResManager.h"

namespace Echo
{
	GltfImageBasedLight::GltfImageBasedLight()
		: m_cubeDiffuseTexture(nullptr)
		, m_cubeSpecularTexture(nullptr)
		, m_brdfLUTTextureName("", ".png")
		, m_brdfLUTTexture(nullptr)
	{
		for (int i = 0; i < 6; i++)
			m_cubeDiffuseTextureNames[i] = ResourcePath("", ".png|.tga|.jpg");

		for (int i = 0; i < 6; i++)
			m_cubeSpecularTextureNames[i] = ResourcePath("", ".png|.tga|.jpg");
	}

	GltfImageBasedLight::~GltfImageBasedLight()
	{

	}

	void GltfImageBasedLight::bindMethods()
	{
		// diffuse method
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffusePositiveXTexture, DEF_METHOD("getDiffusePositiveXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffusePositiveXTexture, DEF_METHOD("setDiffusePositiveXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffusePositiveYTexture, DEF_METHOD("getDiffusePositiveYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffusePositiveYTexture, DEF_METHOD("setDiffusePositiveYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffusePositiveZTexture, DEF_METHOD("getDiffusePositiveZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffusePositiveZTexture, DEF_METHOD("setDiffusePositiveZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffuseNegativeXTexture, DEF_METHOD("getDiffuseNegativeXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffuseNegativeXTexture, DEF_METHOD("setDiffuseNegativeXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffuseNegativeYTexture, DEF_METHOD("getDiffuseNegativeYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffuseNegativeYTexture, DEF_METHOD("setDiffuseNegativeYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getDiffuseNegativeZTexture, DEF_METHOD("getDiffuseNegativeZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setDiffuseNegativeZTexture, DEF_METHOD("setDiffuseNegativeZTexture"));

		// specular method
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularPositiveXTexture, DEF_METHOD("getSpecularPositiveXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularPositiveXTexture, DEF_METHOD("setSpecularPositiveXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularPositiveYTexture, DEF_METHOD("getSpecularPositiveYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularPositiveYTexture, DEF_METHOD("setSpecularPositiveYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularPositiveZTexture, DEF_METHOD("getSpecularPositiveZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularPositiveZTexture, DEF_METHOD("setSpecularPositiveZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularNegativeXTexture, DEF_METHOD("getSpecularNegativeXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularNegativeXTexture, DEF_METHOD("setSpecularNegativeXTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularNegativeYTexture, DEF_METHOD("getSpecularNegativeYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularNegativeYTexture, DEF_METHOD("setSpecularNegativeYTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, getSpecularNegativeZTexture, DEF_METHOD("getSpecularNegativeZTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularNegativeZTexture, DEF_METHOD("setSpecularNegativeZTexture"));

		// brdfLUT
		CLASS_BIND_METHOD(GltfImageBasedLight, getbrdfLUTTexture, DEF_METHOD("getbrdfLUTTexture"));
		CLASS_BIND_METHOD(GltfImageBasedLight, setbrdfLUTTexture, DEF_METHOD("setbrdfLUTTexture"));

		// properties
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace+X", Variant::Type::ResourcePath, "getDiffusePositiveXTexture", "setDiffusePositiveXTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace-X", Variant::Type::ResourcePath, "getDiffuseNegativeXTexture", "setDiffuseNegativeXTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace+Y", Variant::Type::ResourcePath, "getDiffusePositiveYTexture", "setDiffusePositiveYTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace-Y", Variant::Type::ResourcePath, "getDiffuseNegativeYTexture", "setDiffuseNegativeYTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace+Z", Variant::Type::ResourcePath, "getDiffusePositiveZTexture", "setDiffusePositiveZTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "DiffuseCubeFace-Z", Variant::Type::ResourcePath, "getDiffuseNegativeZTexture", "setDiffuseNegativeZTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace+X", Variant::Type::ResourcePath, "getSpecularPositiveXTexture", "setSpecularPositiveXTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace-X", Variant::Type::ResourcePath, "getSpecularNegativeXTexture", "setSpecularNegativeXTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace+Y", Variant::Type::ResourcePath, "getSpecularPositiveYTexture", "setSpecularPositiveYTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace-Y", Variant::Type::ResourcePath, "getSpecularNegativeYTexture", "setSpecularNegativeYTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace+Z", Variant::Type::ResourcePath, "getSpecularPositiveZTexture", "setSpecularPositiveZTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "SpecularCubeFace-Z", Variant::Type::ResourcePath, "getSpecularNegativeZTexture", "setSpecularNegativeZTexture");
		CLASS_REGISTER_PROPERTY(GltfImageBasedLight, "brdfLUT", Variant::Type::ResourcePath, "getbrdfLUTTexture", "setbrdfLUTTexture");
	}

	void GltfImageBasedLight::test()
	{
	}

	// build diffuse texture
	void GltfImageBasedLight::buildDiffuseCube()
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_cubeDiffuseTextureNames[i].getPath().empty())
				return;
		}

		m_cubeDiffuseTexture = TextureResManager::instance()->createTextureCubeFromFiles(
			m_cubeDiffuseTextureNames[0].getPath(), 
			m_cubeDiffuseTextureNames[1].getPath(),
			m_cubeDiffuseTextureNames[2].getPath(),
			m_cubeDiffuseTextureNames[3].getPath(),
			m_cubeDiffuseTextureNames[4].getPath(),
			m_cubeDiffuseTextureNames[5].getPath());
		m_cubeDiffuseTexture->prepareLoad();

		Texture::setGlobal(ui32(TextureIndex::DiffuseCube), m_cubeDiffuseTexture->getTexture());
	}

	// build specular texture
	void GltfImageBasedLight::buildSpecularCube()
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_cubeSpecularTextureNames[i].getPath().empty())
				return;
		}

		m_cubeSpecularTexture = TextureResManager::instance()->createTextureCubeFromFiles(
			m_cubeSpecularTextureNames[0].getPath(),
			m_cubeSpecularTextureNames[1].getPath(),
			m_cubeSpecularTextureNames[2].getPath(),
			m_cubeSpecularTextureNames[3].getPath(),
			m_cubeSpecularTextureNames[4].getPath(),
			m_cubeSpecularTextureNames[5].getPath());
		m_cubeSpecularTexture->prepareLoad();

		Texture::setGlobal(ui32(TextureIndex::SpecularCube), m_cubeSpecularTexture->getTexture());
	}

	// build brdfLUT texture
	void GltfImageBasedLight::buildbrdfLUT()
	{
		if (!m_brdfLUTTextureName.getPath().empty())
		{
			m_brdfLUTTexture = TextureResManager::instance()->createTexture( m_brdfLUTTextureName.getPath());
			m_brdfLUTTexture->prepareLoad();

			Texture::setGlobal(ui32(TextureIndex::BrdfLUT), m_brdfLUTTexture->getTexture());
		}
	}
}