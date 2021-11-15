#pragma once

#include "base/state/render_state.h"
#include <utility>
#include "engine/core/util/Array.hpp"
#include "engine/core/resource/Res.h"
#include "base/mesh/mesh_vertex_data.h"

namespace Echo
{
	class Renderer;
	class ShaderTemplate : public Res
	{
		ECHO_RES(ShaderTemplate, Res, ".shadertemplate", Res::create<ShaderTemplate>, Res::load);

	public:
        ShaderTemplate();
        ShaderTemplate(const ResourcePath& path);
		virtual ~ShaderTemplate();
        
        // vs code
        const String& getVsCode() const;
        void setVsCode(const String& vsCode);
        
        // ps code
        const String& getPsCode() const;
        void setPsCode(const String& psCode);  

	protected:
        String                  m_vsCode;
        String                  m_psCode;
	};
	typedef ResRef<ShaderTemplate> ShaderTemplatePtr;
}
