#include "Converters.h"

namespace DataFlowProgramming
{
	std::shared_ptr<NodeData> FloatToVector2::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataVector2>(shaderData->getDataModel(), "vec2");
			m_result->setVariableName(Echo::StringUtil::Format("vec2(%s)", shaderData->getVariableName().c_str()));
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> FloatToVector3::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataVector3>(shaderData->getDataModel(), "vec3");
			m_result->setVariableName(Echo::StringUtil::Format("vec3(%s)", shaderData->getVariableName().c_str()));
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> FloatToVector4::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataVector4>(shaderData->getDataModel(), "vec4");
			m_result->setVariableName(Echo::StringUtil::Format("vec4(%s)", shaderData->getVariableName().c_str()));
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> FloatToAny::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataAny>(shaderData);
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> Vector2ToAny::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataAny>(shaderData);
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> Vector3ToAny::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataAny>(shaderData);
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> Vector4ToAny::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataAny>(shaderData);
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}

	std::shared_ptr<NodeData> ColorToAny::operator()(std::shared_ptr<NodeData> data)
	{
		auto shaderData = std::dynamic_pointer_cast<ShaderData>(data);
		if (shaderData)
		{
			m_result = std::make_shared<DataAny>(shaderData);
		}
		else
		{
			m_result.reset();
		}

		return m_result;
	}
}
