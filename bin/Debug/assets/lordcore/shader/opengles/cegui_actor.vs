#version 100

attribute vec3 inPosition;
attribute vec2 inTexCoord;

varying vec2 texCoord;

void main(void)
{
	gl_Position = vec4(inPosition.xy, 0.5, 1.0);

	texCoord = inTexCoord;
}