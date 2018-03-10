#version 100

attribute vec3 inPosition;
attribute vec2 inTexCoord;
varying vec2 texCoord;

void main(void)
{
	gl_Position = vec4(inPosition, 1.0);
	texCoord = gl_Position.xy * vec2(0.5,0.5) + vec2(0.5,0.5) + inTexCoord.xy * 0.000001;
}
