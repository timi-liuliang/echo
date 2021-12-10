#include "editor_render_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/main/Engine.h>
#include <thirdparty/pugixml/pugixml.hpp>
#include <QProcess>
#include <engine/core/main/module.h>
#include <engine/core/render/base/renderer.h>

namespace Echo
{
	EditorRenderSettings::EditorRenderSettings()
    {
    }

	EditorRenderSettings::~EditorRenderSettings()
    {

    }

	EditorRenderSettings* EditorRenderSettings::instance()
    {
        static EditorRenderSettings* inst = EchoNew(EditorRenderSettings);
        return inst;
    }

    void EditorRenderSettings::bindMethods()
    {
		CLASS_BIND_METHOD(EditorRenderSettings, getPolygonMode);
		CLASS_BIND_METHOD(EditorRenderSettings, setPolygonMode);

		CLASS_REGISTER_PROPERTY(EditorRenderSettings, "PolygonMode", Variant::Type::StringOption, getPolygonMode, setPolygonMode);
    }

    void EditorRenderSettings::setPolygonMode(const StringOption& option) 
    { 
        if (m_polygonMode.setValue(option.getValue()))
        {
            Renderer::Settings settings = Renderer::instance()->getSettings();
            settings.m_polygonMode = m_polygonMode.getValue()=="Fill" ? RasterizerState::PM_FILL : RasterizerState::PM_LINE;

            Renderer::instance()->setSettings(settings);
        }
    }
}
