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
		i32				m_designWidth;
		i32				m_designHeight;
		ResourcePath	m_launchScene;
	};
}
