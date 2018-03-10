#version 100

attribute vec3 inPosition;
attribute vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 color;
varying vec4 texCoord;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	color = inColor;
	texCoord.xy = inTexCoord * vec2(1, 0.5);
	texCoord.zw = texCoord.xy + vec2(0.0,0.5);
}