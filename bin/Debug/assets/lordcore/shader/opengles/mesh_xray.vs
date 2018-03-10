#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0)
		+ vec4( inNormal, 0.0) + vec4( inTexCoord, 0.0, 0.0) 
		- vec4( inNormal, 0.0) - vec4( inTexCoord, 0.0, 0.0);;
}