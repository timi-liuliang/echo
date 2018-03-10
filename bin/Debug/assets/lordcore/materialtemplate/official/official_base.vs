#version 100

/**
 * shader所有函数都是内联编译，无须担心函数效率
 */

attribute vec3 inPosition;
attribute vec2 inTexCoord;

varying vec2 texCoord;

void main(void)
{
	gl_Position = vec4(inPosition, 1.0);
	texCoord = inTexCoord;
}