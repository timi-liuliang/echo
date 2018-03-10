#version 100
attribute vec3 inPosition;

void main(void)
{
	gl_Position = vec4(inPosition.xy, 0.5, 1.0);
}