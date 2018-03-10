#version 100
attribute vec3 inPosition;
attribute vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 color;
varying vec2 texCoord;

void main(void)
{
	vec4 position = matWVP * vec4(inPosition, 1.0);
	gl_Position = vec4( position.xy, 0.5, 1.0);

	color = inColor;
	texCoord = inTexCoord;
}