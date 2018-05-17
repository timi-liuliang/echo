#include "gltf_ibl.h"

namespace Echo
{
	GltfImageBasedLight::GltfImageBasedLight()
		: m_positiveXDiffuseTextureName("", ".png")
		, m_negativeXDiffuseTextureName("", ".png")
		, m_positiveYDiffuseTextureName("", ".png")
		, m_negativeYDiffuseTextureName("", ".png")
		, m_positiveZDiffuseTextureName("", ".png")
		, m_negativeZDiffuseTextureName("", ".png")
		, m_cubeDiffuseTexture(nullptr)
		, m_positiveXSpecularTextureName("", ".png")
		, m_negativeXSpecularTextureName("", ".png")
		, m_positiveYSpecularTextureName("", ".png")
		, m_negativeYSpecularTextureName("", ".png")
		, m_positiveZSpecularTextureName("", ".png")
		, m_negativeZSpecularTextureName("", ".png")
		, m_cubeSpecularTexture(nullptr)
		, m_brdfLUTTextureName("", ".png")
		, m_brdfLUTTexture(nullptr)
	{

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
		CLASS_BIND_METHOD(GltfImageBasedLight, setSpecularNegativeZTexture, DEF_METHOD("setDiffuseNegativeZTexture"));

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
}