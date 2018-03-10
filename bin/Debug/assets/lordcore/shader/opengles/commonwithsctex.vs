#version 100

attribute vec3 inPosition;
attribute vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 color;
varying vec2 texCoord;

varying vec4 clipPos;

void main(void)
{
	clipPos = matWVP * vec4(inPosition.xyz, 1.0);
	gl_Position = clipPos;
	color = inColor;
	texCoord = inTexCoord;
}