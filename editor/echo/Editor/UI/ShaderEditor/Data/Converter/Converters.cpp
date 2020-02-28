#include "Converters.h"

namespace DataFlowProgramming
{
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
