#pragma once

#include "engine/core/base/object.h"
#include "thirdparty/google/tensorflow/lite/c/c_api.h"

namespace Echo
{
	class TFLiteInput : public Object
	{
		ECHO_CLASS(TFLiteInput, Object)

	public:
		TFLiteInput();
		virtual ~TFLiteInput();

		// Internal tensor
		void bindTensor(TfLiteTensor* tensor);
		TfLiteTensor* getTensor() { return m_tensor; }

		// Set data
		void setImage(const String& resPath);
		void setBuffer(void* buffer, i32 bytes);

	public:
		TfLiteTensor*		m_tensor = nullptr;
		i32					m_bytes = 0;
		vector<i32>::type	m_dims;
	};
}
