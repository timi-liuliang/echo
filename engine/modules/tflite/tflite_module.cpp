#include "tflite_module.h"
#include "tflite_model.h"
#include "editor/tflite_model_editor.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	DECLARE_MODULE(TFLiteModule)

	TFLiteModule::TFLiteModule()
	{

	}

	TFLiteModule::~TFLiteModule()
	{

	}

	TFLiteModule* TFLiteModule::instance()
	{
		static TFLiteModule* inst = EchoNew(TFLiteModule);
		return inst;
	}

	void TFLiteModule::bindMethods()
	{

	}

	void TFLiteModule::registerTypes()
	{
		Class::registerType<TFLiteModel>();

		CLASS_REGISTER_EDITOR(TFLiteModel, TFLiteModelEditor)
	}
}