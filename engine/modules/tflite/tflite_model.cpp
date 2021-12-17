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
		Reset();
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
		CLASS_BIND_METHOD(TFLiteModel, getInput);
		CLASS_BIND_METHOD(TFLiteModel, getOutput);

		CLASS_REGISTER_PROPERTY(TFLiteModel, "InputCount", Variant::Type::Int, getInputCount, setInputCount);
		CLASS_REGISTER_PROPERTY(TFLiteModel, "OutputCount", Variant::Type::Int, getOutputCount, setOutputCount);
		CLASS_REGISTER_PROPERTY(TFLiteModel, "Model", Variant::Type::ResourcePath, getModelRes, setModelRes);
	}

	void TFLiteModel::setModelRes(const ResourcePath& path)
	{
		Reset();

		if (m_modelRes.setPath(path.getPath()))
		{
			m_memoryReader = EchoNew(MemoryReader(path.getPath()));
			if (m_memoryReader->getSize())
			{
				m_model = TfLiteModelCreate(m_memoryReader->getData<const char*>(), m_memoryReader->getSize());
				if (m_model)
				{
					m_options = TfLiteInterpreterOptionsCreate();
					TfLiteInterpreterOptionsSetNumThreads(m_options, 2);

					m_interpreter = TfLiteInterpreterCreate(m_model, m_options);
					if (m_interpreter)
					{
						TfLiteInterpreterAllocateTensors(m_interpreter);

						i32 inputCount = TfLiteInterpreterGetInputTensorCount(m_interpreter);
						for (i32 i = 0; i < inputCount; i++)
						{
							TFLiteInput* input = EchoNew(TFLiteInput);
							input->bindTensor(TfLiteInterpreterGetInputTensor(m_interpreter, i));

							m_inputs.emplace_back(input);
						}

						i32 outputCount = TfLiteInterpreterGetOutputTensorCount(m_interpreter);
						for (i32 i = 0; i < outputCount; i++)
						{
							TFLiteOutput* output = EchoNew(TFLiteOutput);
							output->bindTensor(TfLiteInterpreterGetOutputTensor(m_interpreter, i));

							m_outputs.emplace_back(output);
						}
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
			TfLiteStatus status = TfLiteInterpreterInvoke(m_interpreter);
			if (kTfLiteOk != status)
			{
				EchoLogError("TfLiteModel invoke failed");
			}
		}
	}

	void TFLiteModel::Reset()
	{
		EchoSafeDelete(m_memoryReader, MemoryReader);

		if (m_interpreter)
		{
			TfLiteInterpreterDelete(m_interpreter);
			m_interpreter = nullptr;
		}

		if (m_options)
		{
			TfLiteInterpreterOptionsDelete(m_options);
			m_options = nullptr;
		}

		if (m_model)
		{
			TfLiteModelDelete(m_model);
			m_model = nullptr;
		}
	}
}