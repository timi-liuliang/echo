#pragma once

#include "engine/core/scene/node.h"
#include "thirdparty/google/tensorflow/lite/c/c_api.h"

namespace Echo
{
	class TFLiteModel : public Node
	{
		ECHO_CLASS(TFLiteModel, Node)

	public:
		TFLiteModel();
		virtual ~TFLiteModel();

		// Model file
		void setModelRes(const ResourcePath& path);
		const ResourcePath& getModelRes() { return m_modelRes; }

		// Input count
		i32 getInputCount() const { return m_inputCount; }
		void setInputCount() {}

		// Output count
		i32 getOutputCount() const { return m_outputCount; }
		void setOutputCount() {}

		// Invoke
		void invoke();

	public:
		ResourcePath			m_modelRes = ResourcePath("", ".tflite");
		TfLiteModel*			m_model = nullptr;
		TfLiteInterpreter*		m_interpreter = nullptr;
		i32						m_inputCount = 0;
		i32						m_outputCount = 0;
		vector<TfLiteTensor*>	m_inputs;
		vector<TfLiteTensor*>	m_outputs;
	};
}
