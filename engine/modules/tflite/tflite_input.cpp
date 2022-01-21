#include "thirdparty/google/tensorflow/lite/c/c_api_internal.h"
#include "tflite_input.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	TFLiteInput::TFLiteInput()
		: Object()
	{

	}

	TFLiteInput::~TFLiteInput()
	{
	}

	void TFLiteInput::bindMethods()
	{
		CLASS_BIND_METHOD(TFLiteInput, setImage);
		//CLASS_BIND_METHOD(TFLiteInput, setBuffer);
	}

	void TFLiteInput::bindTensor(TfLiteTensor* tensor)
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

	void TFLiteInput::setImage(const String& resPath)
	{
		Image* image = Image::loadFromFile(resPath);

		i32 bytes = image->getWidth() * image->getHeight() * PixelUtil::GetPixelBytes(image->getPixelFormat());
		if (bytes == m_bytes)
		{
			setBuffer(image->getData(), bytes);
		}
		else
		{
			EchoLogError("TFLiteInput set iamge failed, image size not right");
		}

		EchoSafeDelete(image, Image);
	}

	void TFLiteInput::setBuffer(void* buffer, i32 bytes)
	{
		TfLiteTensorCopyFromBuffer(m_tensor, buffer, bytes);
	}
}