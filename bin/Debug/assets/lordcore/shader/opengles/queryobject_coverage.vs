#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec4 inColor;

uniform mat4 worldMat;
uniform mat4 viewProjMat;

uniform mat4 matWVP;

//varying mediump vec4 vScreenPos;
varying vec4 Color;

void main(void)
{
	gl_Position = viewProjMat * worldMat * vec4(inPosition, 1.0);
//	vScreenPos = gl_Position;
	Color = inColor;
}
