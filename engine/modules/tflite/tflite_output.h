#pragma once

#include "engine/core/base/object.h"
#include "thirdparty/google/tensorflow/lite/c/c_api.h"

namespace Echo
{
	class TFLiteOutput : public Object
	{
		ECHO_CLASS(TFLiteOutput, Object)

	public:
		TFLiteOutput();
		virtual ~TFLiteOutput();

		// Internal tensor
		void bindTensor(const TfLiteTensor* tensor);
		const TfLiteTensor* getTensor() { return m_tensor; }

		// Get Data
		void getData();

	public:
		const TfLiteTensor*	m_tensor = nullptr;
		i32					m_bytes = 0;
		vector<i32>::type	m_dims;
		vector<ui8>::type	m_result;
	};
}
