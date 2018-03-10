#pragma once

#include "RenderStage.h" 

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	/**
	 * ±à¼­Æ÷Ç°ÖÃ½×¶Î
	 */
	class EditorForegroundRenderStage : public RenderStage
	{
		friend class RenderStageManager;

	public:
		virtual void render();
		virtual bool initialize();
		virtual void destroy();
	protected:
		EditorForegroundRenderStage();
		virtual ~EditorForegroundRenderStage();
	};
#endif
}