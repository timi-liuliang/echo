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

		// Internal tensor
		void setTensor(TfLiteTensor* tensor);
		TfLiteTensor* getTensor() { return m_tensor; }

		// Set image data
		void setImage(const String& resPath);

	public:
		TfLiteTensor*		m_tensor = nullptr;
		i32					m_bytes = 0;
		vector<i32>::type	m_dims;
	};
}
