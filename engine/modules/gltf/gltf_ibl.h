#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/TextureRes.h"

namespace Echo
{
	class GltfImageBasedLight : public Node
	{
		ECHO_CLASS(GltfImageBasedLight, Node)

	public:
		GltfImageBasedLight();
		virtual ~GltfImageBasedLight();

		// bind class methods to script
		static void bindMethods();

		// diffuse cube texture
		const ResourcePath& getDiffusePositiveXTexture() { return m_positiveXDiffuseTextureName; }
		void setDiffusePositiveXTexture(const ResourcePath& path) { if (m_positiveXDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeXTexture() { return m_negativeXDiffuseTextureName; }
		void setDiffuseNegativeXTexture(const ResourcePath& path) { if (m_negativeXDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffusePositiveYTexture() { return m_positiveYDiffuseTextureName; }
		void setDiffusePositiveYTexture(const ResourcePath& path) { if (m_positiveYDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeYTexture() { return m_negativeYDiffuseTextureName; }
		void setDiffuseNegativeYTexture(const ResourcePath& path) { if (m_negativeYDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffusePositiveZTexture() { return m_positiveZDiffuseTextureName; }
		void setDiffusePositiveZTexture(const ResourcePath& path) { if (m_positiveZDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }
		const ResourcePath& getDiffuseNegativeZTexture() { return m_negativeZDiffuseTextureName; }
		void setDiffuseNegativeZTexture(const ResourcePath& path) { if (m_negativeZDiffuseTextureName.setPath(path.getPath())) buildDiffuseCube(); }

		// specular cube texture
		const ResourcePath& getSpecularPositiveXTexture() { return m_positiveXSpecularTextureName; }
		void setSpecularPositiveXTexture(const ResourcePath& path) { if (m_positiveXSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeXTexture() { return m_negativeXSpecularTextureName; }
		void setSpecularNegativeXTexture(const ResourcePath& path) { if (m_negativeXSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularPositiveYTexture() { return m_positiveYSpecularTextureName; }
		void setSpecularPositiveYTexture(const ResourcePath& path) { if (m_positiveYSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeYTexture() { return m_negativeYSpecularTextureName; }
		void setSpecularNegativeYTexture(const ResourcePath& path) { if (m_negativeYSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularPositiveZTexture() { return m_positiveZSpecularTextureName; }
		void setSpecularPositiveZTexture(const ResourcePath& path) { if (m_positiveZSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }
		const ResourcePath& getSpecularNegativeZTexture() { return m_negativeZSpecularTextureName; }
		void setSpecularNegativeZTexture(const ResourcePath& path) { if (m_negativeZSpecularTextureName.setPath(path.getPath())) buildSpecularCube(); }

		// brdfLUT texture
		const ResourcePath& getbrdfLUTTexture() { return m_brdfLUTTextureName; }
		void setbrdfLUTTexture(const ResourcePath& path) { if (m_brdfLUTTextureName.setPath(path.getPath())) buildbrdfLUT(); }

	private:
		// build diffuse texture
		void buildDiffuseCube(){}

		// build specular texture
		void buildSpecularCube(){}

		// build brdfLUT texture
		void buildbrdfLUT() {}

	private:
		ResourcePath	m_positiveXDiffuseTextureName;
		ResourcePath	m_negativeXDiffuseTextureName;
		ResourcePath	m_positiveYDiffuseTextureName;
		ResourcePath	m_negativeYDiffuseTextureName;
		ResourcePath	m_positiveZDiffuseTextureName;
		ResourcePath	m_negativeZDiffuseTextureName;
		TextureRes*		m_cubeDiffuseTexture;
		ResourcePath	m_positiveXSpecularTextureName;
		ResourcePath	m_negativeXSpecularTextureName;
		ResourcePath	m_positiveYSpecularTextureName;
		ResourcePath	m_negativeYSpecularTextureName;
		ResourcePath	m_positiveZSpecularTextureName;
		ResourcePath	m_negativeZSpecularTextureName;
		TextureRes*		m_cubeSpecularTexture;
		ResourcePath	m_brdfLUTTextureName;
		TextureRes*		m_brdfLUTTexture;
	};
}