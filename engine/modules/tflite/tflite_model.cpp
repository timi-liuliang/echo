#include "thirdparty/google/tensorflow/lite/c/c_api_internal.h"
#include "tflite_model.h"
#include "tflite_module.h"
#include "engine/core/io/io.h"
#include "engine/core/main/engine.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	TFLiteModel::TFLiteModel()
	{

	}

	TFLiteModel::~TFLiteModel()
	{
	}

	void TFLiteModel::bindMethods()
	{
		CLASS_BIND_METHOD(TFLiteModel, invoke);
		CLASS_BIND_METHOD(TFLiteModel, getModelRes);
		CLASS_BIND_METHOD(TFLiteModel, setModelRes);
		CLASS_BIND_METHOD(TFLiteModel, getInputCount);
		CLASS_BIND_METHOD(TFLiteModel, setInputCount);
		CLASS_BIND_METHOD(TFLiteModel, getOutputCount);
		CLASS_BIND_METHOD(TFLiteModel, setOutputCount);

		CLASS_REGISTER_PROPERTY(TFLiteModel, "InputCount", Variant::Type::Int, getInputCount, setInputCount);
		CLASS_REGISTER_PROPERTY(TFLiteModel, "OutputCount", Variant::Type::Int, getOutputCount, setOutputCount);
		CLASS_REGISTER_PROPERTY(TFLiteModel, "Model", Variant::Type::ResourcePath, getModelRes, setModelRes);
	}

	void TFLiteModel::setModelRes(const ResourcePath& path)
	{
		if (m_modelRes.setPath(path.getPath()))
		{
			MemoryReader memReader(path.getPath());
			if (memReader.getSize())
			{
				m_model = TfLiteModelCreate(memReader.getData<const char*>(), memReader.getSize());
				if (m_model)
				{
					TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
					TfLiteInterpreterOptionsSetNumThreads(options, 1);

					m_interpreter = TfLiteInterpreterCreate(m_model, options);
					if (m_interpreter)
					{
						TfLiteInterpreterAllocateTensors(m_interpreter);

						i32 inputCount = TfLiteInterpreterGetInputTensorCount(m_interpreter);
						for (i32 i = 0; i < inputCount; i++)
						{
							m_inputs.emplace_back(TfLiteInterpreterGetInputTensor(m_interpreter, i));
						}

						i32 outputCount = TfLiteInterpreterGetOutputTensorCount(m_interpreter);
						for (i32 i = 0; i < outputCount; i++)
						{
							m_outputs.emplace_back(TfLiteInterpreterGetOutputTensor(m_interpreter, i));
						}

						invoke();
					}
					else
					{
						EchoLogError("TFLiteModel Failed to create interpreter");
					}
				}
				else
				{
					EchoLogError("TFLiteModel Failed to load tflite model [%s]", path.getPath().c_str());
				}
			}
		}
	}

	void TFLiteModel::invoke()
	{
		if (m_interpreter)
		{
			i32 inByteSize = TfLiteTensorByteSize(m_inputs[0]);
			i32 inNumDims = TfLiteTensorNumDims(m_inputs[0]);
			vector<int>::type inSizes;
			for (i32 i = 0; i < inNumDims; i++)
			{
				inSizes.emplace_back(TfLiteTensorDim(m_inputs[0], i));
			}

			// Set
			{
				Image* image = Image::loadFromFile("Res://model/banana.jpg");

				i32 bytes = image->getWidth() * image->getHeight() * PixelUtil::GetPixelBytes(image->getPixelFormat());
				TfLiteTensorCopyFromBuffer(m_inputs[0], image->getData(), bytes);
			}

			if (kTfLiteOk == TfLiteInterpreterInvoke(m_interpreter))
			{
				i32 outByteSize = TfLiteTensorByteSize(m_outputs[0]);
				i32 outNumDims = TfLiteTensorNumDims(m_outputs[0]);
				vector<int>::type outSizes;
				for (i32 i = 0; i < outNumDims; i++)
				{
					outSizes.emplace_back(TfLiteTensorDim(m_outputs[0], i));
				}

				vector<ui8>::type result;
				result.resize(outByteSize);
				
				TfLiteTensorCopyToBuffer(m_outputs[0], result.data(), outByteSize);

				int a = 10;
			}
		}
	}
}