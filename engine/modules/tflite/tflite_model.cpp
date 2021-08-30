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
		CLASS_BIND_METHOD(TFLiteModel, getModelRes, DEF_METHOD("getModelRes"));
		CLASS_BIND_METHOD(TFLiteModel, setModelRes, DEF_METHOD("setModelRes"));

		CLASS_REGISTER_PROPERTY(TFLiteModel, "Model", Variant::Type::ResourcePath, "getModelRes", "setModelRes");
	}

	void TFLiteModel::setModelRes(const ResourcePath& path)
	{
		if (m_modelRes.setPath(path.getPath()))
		{
			MemoryReader memReader(path.getPath());
			if (memReader.getSize())
			{
				m_model;// = TfLiteModelCreate(memReader.getData<const char*>(), memReader.getSize());
				if (m_model)
				{

				}
				else
				{
					EchoLogError("Failed to load tflite model [%s]", path.getPath().c_str());
				}
			}
		}
	}
}