#include "thirdparty/google/tensorflow/lite/c/c_api_internal.h"
#include "tflite_tensor.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	TFLiteTensor::TFLiteTensor()
		: Object()
	{

	}

	TFLiteTensor::~TFLiteTensor()
	{
	}

	void TFLiteTensor::bindMethods()
	{

	}

	void TFLiteTensor::setTensor(TfLiteTensor* tensor) 
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

	void TFLiteTensor::setImage(const String& resPath)
	{
		Image* image = Image::loadFromFile(resPath);

		i32 bytes = image->getWidth() * image->getHeight() * PixelUtil::GetPixelBytes(image->getPixelFormat());
		TfLiteTensorCopyFromBuffer(m_tensor, image->getData(), bytes);

		EchoSafeDelete(image, Image);
	}
}