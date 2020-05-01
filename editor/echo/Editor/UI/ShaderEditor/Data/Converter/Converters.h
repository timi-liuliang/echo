#pragma once

#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"
#include "DataAny.h"

namespace DataFlowProgramming
{
	class FloatToAny
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class Vector2ToAny
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class Vector3ToAny
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class Vector4ToAny
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class ColorToAny
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};
}


