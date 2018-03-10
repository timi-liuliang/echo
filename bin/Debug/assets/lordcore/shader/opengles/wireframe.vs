#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec4 inColor;

uniform mat4 worldMat;
uniform mat4 viewProjMat;

varying vec4 color;
varying vec3 normal;

void main(void)
{
	gl_Position = worldMat * vec4(inPosition, 1.0);
	gl_Position = viewProjMat * gl_Position;
	color = inColor + vec4(inNormal * 0.00001, 0.0);
}