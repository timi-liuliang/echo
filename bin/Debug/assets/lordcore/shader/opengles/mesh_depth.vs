#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

varying vec4 vScreenPos;

void main(void)
{
	gl_Position  = matWVP * vec4(inPosition, 1.0);
	vScreenPos   = gl_Position;
	vScreenPos.z+= (inNormal.x + inTexCoord.x)*0.00001; 
}