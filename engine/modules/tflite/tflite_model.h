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

		// model file
		void setModelRes(const ResourcePath& path);
		const ResourcePath& getModelRes() { return m_modelRes; }

	public:
		ResourcePath			m_modelRes = ResourcePath("", ".tflite");
		TfLiteModel*			m_model = nullptr;
		TfLiteInterpreter*		m_interpreter = nullptr;
		vector<TfLiteTensor*>	m_inputs;
		vector<TfLiteTensor*>	m_outputs;
	};
}
