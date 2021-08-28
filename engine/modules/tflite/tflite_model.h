#pragma once

#include "engine/core/scene/node.h"
#include "rvo2/RVO.h"

namespace Echo
{
	class TFLiteModel : public Node
	{
		ECHO_CLASS(TFLiteModel, Node)

	public:
		TFLiteModel();
		virtual ~TFLiteModel();

	public:
	};
}
