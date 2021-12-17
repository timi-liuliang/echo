#pragma once

#include "engine/core/io/io.h"
#include "engine/core/scene/node.h"
#include "tflite_input.h"
#include "tflite_output.h"

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
		i32 getInputCount() const { return i32(m_inputs.size()); }
		void setInputCount() {}

		// Output count
		i32 getOutputCount() const { return i32(m_outputs.size()); }
		void setOutputCount() {}

		// Input|Output
		Object* getInput(i32 index) { return index >= 0 && index < getInputCount() ? m_inputs[index] : nullptr; }
		Object* getOutput(i32 index) { return index >= 0 && index < getOutputCount() ? m_outputs[index] : nullptr; }

		// Invoke
		void invoke();

		// Reset
		void Reset();

	public:
		ResourcePath						m_modelRes = ResourcePath("", ".tflite");
		MemoryReader*						m_memoryReader = nullptr;
		TfLiteModel*						m_model = nullptr;
		TfLiteInterpreterOptions*			m_options = nullptr;
		TfLiteInterpreter*					m_interpreter = nullptr;
		vector<TFLiteInput*>::type			m_inputs;
		vector<TFLiteOutput*>::type			m_outputs;
	};
}
