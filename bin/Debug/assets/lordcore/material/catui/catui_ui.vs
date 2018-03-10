#version 100

attribute vec3 inPosition;
attribute vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 color;
varying vec2 texCoord;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	color.rgba = inColor.bgra;
	texCoord = inTexCoord;
}