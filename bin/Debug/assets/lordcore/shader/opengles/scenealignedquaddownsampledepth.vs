#version 300 es

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

out vec2 texCoord;

void main(void)
{
	gl_Position = vec4(inPosition, 1.0);
	texCoord = inTexCoord;
}