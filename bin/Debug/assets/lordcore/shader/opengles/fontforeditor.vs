#version 100

attribute vec3 inPosition;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec2 texCoord;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	texCoord = inTexCoord;
}