#pragma once

#include <engine/core/base/object.h>

namespace Echo
{
	class Light : public Object
	{
		ECHO_SINGLETON_CLASS(Light, Object);

	public:
		// instance
		static Light* instance();

		// image based lighting
		bool isIBLEnable() const { return m_isIBLEnable; }
		void setIBLEnable(bool isEnable) { m_isIBLEnable = isEnable; }

	private:
		Light();
		virtual ~Light();

	protected:
		bool		m_isIBLEnable;
	};
}