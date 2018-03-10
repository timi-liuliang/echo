#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWorld;
uniform mat4 matWVP;
uniform mediump vec4 naturalColor;
uniform mediump vec4 edgeColor;
uniform mediump vec4 colorParams;
uniform mediump vec3 camPos;

varying vec2 texCoord;
varying vec4 worldPosition;
varying vec3 worldNormal;
	
void main(void)
{
	vec4 localPos = vec4(inPosition, 1.0);
	gl_Position = matWVP * localPos;
	vec3 vNormal = mat3(matWorld) * inNormal;
	worldNormal = normalize(vNormal);

	texCoord = inTexCoord;
	worldPosition = matWorld * localPos;
}