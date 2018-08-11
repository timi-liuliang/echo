#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/render/render/Color.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/resource/Res.h"

namespace Echo
{
	class GameSettings : public Object
	{
		ECHO_SINGLETON_CLASS(GameSettings, Object);

	public:
		enum class KeepAspectType
		{
			None,
			Stretch,
			Width,
			Height,
		};

	public:
		GameSettings();
		~GameSettings();

		// instance
		static GameSettings* instance();

		// design window width
		void setDesignWidth(i32 width);
		i32 getDesignWidth() const { return m_designWidth; }

		// design window height
		void setDesignHeight(i32 height);
		i32 getDesignHeight() const { return m_designHeight; }

		// aspect
		const StringOption& getAspect() const { return m_aspect; }
		void setAspect(const StringOption& option);

		// set launch scene
		void setLaunchScene(const ResourcePath& path) { m_launchScene.setPath(path.getPath()); }

		// get launch scene
		const ResourcePath& getLaunchScene() const { return m_launchScene; }

		// get bg color
		const Color& getBackgroundColor() const;

		// set bg color
		void setBackgroundColor(const Color& color);

		// on size
		void onSize(ui32 windowWidth, ui32 windowHeight);

	private:
		// keep aspect
		void keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type);

	private:
		i32				m_designWidth;
		i32				m_designHeight;
		StringOption	m_aspect;
		ResourcePath	m_launchScene;
	};
}
