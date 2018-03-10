#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec4 inColor;

uniform mat4 worldMat;
uniform mat4 viewProjMat;
uniform vec3 lightDir;
uniform vec4 diffuseColor;

varying vec4 color;
varying vec3 normal;

void main(void)
{
	gl_Position = worldMat * vec4(inPosition, 1.0);
	gl_Position = viewProjMat * gl_Position;
	
	vec3 aNormal = normalize( inNormal );
	vec4 worldNormal = worldMat * vec4(aNormal, 0);

	float fDotVP = max(0.0, dot(worldNormal.xyz, lightDir));
	vec3 diffuse = diffuseColor.xyz * fDotVP;

	vec3 ambient = vec3( 0.6, 0.6, 0.6 );

	// final color
	color = vec4( (diffuse+ambient) * inColor.xyz, inColor.w);
}