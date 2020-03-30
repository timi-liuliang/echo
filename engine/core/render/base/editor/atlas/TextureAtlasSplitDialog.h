#pragma once

#include "engine/core/util/Array.hpp"
#include "../../TextureAtlas.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class TextureAtlasSplitDialog
	{
	public:
		TextureAtlasSplitDialog();
		virtual ~TextureAtlasSplitDialog();

		// update
		void update();

	protected:
		class QWidget*		m_ui;
	};
}

#endif