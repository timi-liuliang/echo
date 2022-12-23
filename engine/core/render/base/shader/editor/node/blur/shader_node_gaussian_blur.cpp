#include "shader_node_gaussian_blur.h"

#ifdef ECHO_EDITOR_MODE

// https://en.wikipedia.org/wiki/Gaussian_blur
static const char* gaussianBlur2D =R"(vec3 GaussianBlur(sampler2D tex, vec2 size, vec2 uv)
{
	float radius = 3;
	float sigma = 0.84089642;
	float twoSigmaSigma = 2 * sigma * sigma;
	float twoPiSigmaSigma = 1.0 / (twoSigmaSigma * 3.1415926);
	vec2  texSizeInv = 1.0 / size;
	
	vec4 color = vec4(0.0);
	float weights = 0.0;
	for (float w = -radius; w <= radius; w++)
	{
		for (float h = -radius; h <= radius; h++)
		{
			vec2 offset = vec2(w, h) * texSizeInv;
			float weight = 1.0 / twoPiSigmaSigma * exp(-(w*w+h*h) / twoSigmaSigma);

			color += texture(tex, uv + offset) * weight;
			weights += weight;
		}
	}

	return color.xyz / weights;
})";

namespace Echo
{
	ShaderNodeGaussianBlur::ShaderNodeGaussianBlur()
		: ShaderNodeGLSL()
	{
		setCode(gaussianBlur2D);
	}

	ShaderNodeGaussianBlur::~ShaderNodeGaussianBlur()
	{
	}

	void ShaderNodeGaussianBlur::bindMethods()
	{
	}
}

#endif
