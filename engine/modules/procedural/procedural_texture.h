#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class ProceduralTexture : public Node
	{
		ECHO_CLASS(ProceduralTexture, Node)

	public:
		ProceduralTexture();
		virtual ~ProceduralTexture();

	protected:
		// update self
		virtual void update_self() override;

	protected:
	};
}
