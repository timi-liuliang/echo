#include "thirdparty/google/tensorflow/lite/c/c_api_internal.h"
#include "tflite_output.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	TFLiteOutput::TFLiteOutput()
		: Object()
	{

	}

	TFLiteOutput::~TFLiteOutput()
	{
	}

	void TFLiteOutput::bindMethods()
	{
		CLASS_BIND_METHOD(TFLiteOutput, print);
	}

	void TFLiteOutput::bindTensor(const TfLiteTensor* tensor)
	{ 
		m_tensor = tensor;
		if (m_tensor)
		{
			m_bytes = TfLiteTensorByteSize(m_tensor);

			i32 numDims = TfLiteTensorNumDims(m_tensor);
			for (i32 i = 0; i < numDims; i++)
			{
				m_dims.emplace_back(TfLiteTensorDim(m_tensor, i));
			}
		}
	}

	void TFLiteOutput::getData()
	{
		m_result.resize(m_bytes);

		TfLiteTensorCopyToBuffer(m_tensor, m_result.data(), m_bytes);
	}

	void TFLiteOutput::print()
	{
		getData();

		EchoLogInfo(StringUtil::ToString(m_result).c_str());
	}
}