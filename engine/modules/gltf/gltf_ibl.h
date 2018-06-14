#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/TextureRes.h"

namespace Echo
{
	class GltfImageBasedLight : public Node
	{
		ECHO_CLASS(GltfImageBasedLight, Node)

	public:
		enum class TextureIndex
		{
			BrdfLUT = 32,
			DiffuseCube = 33,
			SpecularCube = 34
		};

	public:
		GltfImageBasedLight();
		virtual ~GltfImageBasedLight();

		static void test();

		// get brdfLUT texture
		//static const String& getbrdfLUTTexture()

		// diffuse cube texture
		const ResourcePath& getDiffusePositiveXTexture() { return m_cubeDiffuseTextureNames[0]; }
		void setDiffusePositiveXTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[0].setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeXTexture() { return m_cubeDiffuseTextureNames[1]; }
		void setDiffuseNegativeXTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[1].setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffusePositiveYTexture() { return m_cubeDiffuseTextureNames[2]; }
		void setDiffusePositiveYTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[2].setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeYTexture() { return m_cubeDiffuseTextureNames[3]; }
		void setDiffuseNegativeYTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[3].setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffusePositiveZTexture() { return m_cubeDiffuseTextureNames[4]; }
		void setDiffusePositiveZTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[4].setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeZTexture() { return m_cubeDiffuseTextureNames[5]; }
		void setDiffuseNegativeZTexture(const ResourcePath& path) { if (m_cubeDiffuseTextureNames[5].setPath(path.getPath())) buildDiffuseCube(); }

		// specular cube texture
		const ResourcePath& getSpecularPositiveXTexture() { return m_cubeSpecularTextureNames[0]; }
		void setSpecularPositiveXTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[0].setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeXTexture() { return m_cubeSpecularTextureNames[1]; }
		void setSpecularNegativeXTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[1].setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularPositiveYTexture() { return m_cubeSpecularTextureNames[2]; }
		void setSpecularPositiveYTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[2].setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeYTexture() { return m_cubeSpecularTextureNames[3]; }
		void setSpecularNegativeYTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[3].setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularPositiveZTexture() { return m_cubeSpecularTextureNames[4]; }
		void setSpecularPositiveZTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[4].setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeZTexture() { return m_cubeSpecularTextureNames[5]; }
		void setSpecularNegativeZTexture(const ResourcePath& path) { if (m_cubeSpecularTextureNames[5].setPath(path.getPath())) buildSpecularCube(); }

		// brdfLUT texture
		const ResourcePath& getbrdfLUTTexture() { return m_brdfLUTTextureName; }
		void setbrdfLUTTexture(const ResourcePath& path) { if (m_brdfLUTTextureName.setPath(path.getPath())) buildbrdfLUT(); }

	private:
		// build diffuse texture
		void buildDiffuseCube();

		// build specular texture
		void buildSpecularCube();

		// build brdfLUT texture
		void buildbrdfLUT();

	private:
		ResourcePath	m_cubeDiffuseTextureNames[6];
		TextureRes*		m_cubeDiffuseTexture;
		ResourcePath	m_cubeSpecularTextureNames[6];
		TextureRes*		m_cubeSpecularTexture;
		ResourcePath	m_brdfLUTTextureName;
		TextureRes*		m_brdfLUTTexture;
	};
}