#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWorld;
uniform mat4 matWVP;
uniform vec3 camPos;

varying vec2 texCoord;
varying vec3 normal;
varying vec3 Eye;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);
	texCoord 		= inTexCoord;
	normal 			= normalize(mat3(matWorld) * inNormal);
	Eye    			= camPos -  mat3(matWorld) * inPosition;
}