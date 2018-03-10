#version 100

attribute vec3 inPosition;

uniform mat4 worldViewProjMat;

varying vec3 texcoord;

void main(void)
{
	gl_Position = worldViewProjMat * vec4(inPosition.xyz, 1.0);
	gl_Position.z = gl_Position.w;
	texcoord = inPosition;
}