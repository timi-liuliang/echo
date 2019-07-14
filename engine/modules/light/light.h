#pragma once

#include <engine/core/base/object.h>
#include "engine/core/render/interface/Texture.h"

namespace Echo
{
	class Light : public Object
	{
		ECHO_SINGLETON_CLASS(Light, Object);

		friend class CubeLightCustom;
		friend class CubeLightCapture;

	public:
        virtual ~Light();
        
		// instance
		static Light* instance();

		// image based lighting
		bool isIBLEnable() const { return m_isIBLEnable; }
		void setIBLEnable(bool isEnable) { m_isIBLEnable = isEnable; }

		// cube map
		const ResourcePath& getIBLBrdfPath() const { return m_iblBrdfPath; }
		void setIBLBrdfPath(const ResourcePath& brdf);

		// query get ibl textuer
		Texture* getIBLDiffuseTexture();
		Texture* getIBLSpecularTexture();
		Texture* getIBLBrdfTexture();

	private:
		Light();

	protected:
		bool			m_isIBLEnable;
		ResourcePath	m_iblBrdfPath;
		Texture*		m_iblDiffuseTexture;
		Texture*		m_iblSpecularTexture;
		Texture*		m_iblBrdfTexture;
	};
}
