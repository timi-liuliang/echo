#include "engine/core/log/Log.h"
#include "model_module.h"

namespace Echo
{
	DECLARE_MODULE(ModelModule)


	ModelModule::ModelModule()
	{
	}

	ModelModule* ModelModule::instance()
	{
		static ModelModule* inst = EchoNew(ModelModule);
		return inst;
	}

	void ModelModule::bindMethods()
	{

	}

	void ModelModule::registerTypes()
	{
	}
}