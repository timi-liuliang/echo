#pragma once

#include "engine/core/base/object.h"
#include "thirdparty/google/tensorflow/lite/c/c_api.h"

namespace Echo
{
	class TFLiteTensor : public Object
	{
		ECHO_CLASS(TFLiteTensor, Object)

	public:
		TFLiteTensor();
		virtual ~TFLiteTensor();

	public:
	};
}
