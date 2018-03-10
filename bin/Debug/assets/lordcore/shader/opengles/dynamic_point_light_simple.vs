#version 100

attribute vec3 inPosition;

uniform mat4 matW;
uniform mat4 matWVP;

varying vec3 worldPosition;

void main(void)
{
	worldPosition = ( matW * vec4(inPosition, 1.0)).xyz;

	gl_Position = matWVP * vec4(inPosition, 1.0);
}