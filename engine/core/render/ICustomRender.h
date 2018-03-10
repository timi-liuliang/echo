#pragma once

namespace Echo
{
	class RenderInput;
	class ShaderProgram;

	class ICustomRender
	{
	public:
		virtual ~ICustomRender() {};

		virtual void render(RenderInput* renderInput) = 0;

		virtual Echo::ShaderProgram* getProgram() = 0;
	};
}