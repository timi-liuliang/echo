#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
uniform mat4 matWVP;

varying vec3 normal;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	normal = inNormal;
}