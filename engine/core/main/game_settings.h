#pragma once

#include "engine/core/base/variant.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/math/color.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/resource/Res.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"

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
        
        // full screen
        void setFullScreen(bool fullScreen);
        bool isFullScreen() const { return m_fullScreen; }

		// design window width
		void setDesignWidth(i32 width);
		i32 getDesignWidth() const { return m_designWidth; }

		// design window height
		void setDesignHeight(i32 height);
		i32 getDesignHeight() const { return m_designHeight; }

		// design window width
		void setWindowWidth(i32 width) { m_windowWidth = (width==m_designWidth) ? -1 :width; }
		i32 getWindowWidth() const { return m_windowWidth>0 ? m_windowWidth : m_designWidth; }

		// design window height
		void setWindowHeight(i32 height) { m_windowHeight = (height==m_designHeight) ? -1 : height; }
		i32 getWindowHeight() const { return m_windowHeight >0 ? m_windowHeight : m_designHeight; }

		// aspect
		const StringOption& getAspect() const { return m_aspect; }
		void setAspect(const StringOption& option);

		// set scene
		void setLaunchScene(const ResourcePath& path) { m_launchScene.setPath(path.getPath()); }
		const ResourcePath& getLaunchScene() const { return m_launchScene; }

		// render pipeline
		void setRenderPipeline(const ResourcePath& path);
		const ResourcePath& getRenderPipeline() const;

		// on size
		void onSize(ui32 windowWidth, ui32 windowHeight);

	private:
		// keep aspect
		void keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type);
		void keepAspect(ui32 windowWidth, ui32 windowHeight, KeepAspectType type, class Camera* camera);

	private:
        bool                m_fullScreen = false;
		i32					m_designWidth;
		i32					m_designHeight;
		i32					m_windowWidth;
		i32					m_windowHeight;
		StringOption		m_aspect;
		ResourcePath		m_launchScene;
		ResourcePath		m_renderPipelinePath;
	};
}
