#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWorld;
uniform mat4 matWVP;
uniform mediump vec4 naturalColor;
uniform vec2 UVScale;
//uniform vec3 litDir;
//uniform vec4 litColor;
//uniform vec4 ambtColor;
uniform vec4 litParam[3];

varying vec4 color;
varying vec2 texCoord;

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);
	texCoord = inTexCoord+UVScale;

	vec3 vNormal = mat3(matWorld) * inNormal;
	color = litParam[1] * max(dot(normalize(vNormal), litParam[0].xyz), 0.0) + litParam[2] + naturalColor;
	color *= vec4(1.0, 1.0, 1.0, naturalColor.a);
}