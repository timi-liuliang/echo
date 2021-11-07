#pragma once

#include "../shader_data_float.h"
#include "../shader_data_vector2.h"
#include "../shader_data_vector3.h"
#include "../shader_data_vector4.h"
#include "../shader_data_any.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class FloatToVector2
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class FloatToVector3
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

	class FloatToVector4
	{
	public:
		std::shared_ptr<NodeData> operator()(std::shared_ptr<NodeData> data);

	private:
		std::shared_ptr<ShaderData> m_result;
	};

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

#endif
