#version 100
#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVPSky;

varying vec2 texCoord;
varying vec3 normal;
varying vec3 position;

#ifdef RECEIVE_FOG
	uniform vec4 fogParam[3];
	varying vec4 ofogColor;

	float ComputeFog(vec3 camToWorldPos, vec3 param)
	{
		float fdist = max(length(camToWorldPos) - param.x, 0.0);
		float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
		return 1.0 - density;
	}
#endif

void main(void)
{
	texCoord = inTexCoord;
	vec4 transform = vec4(inPosition, 1.0);
	
	normal = inNormal;
	position = inPosition;
	
	gl_Position   = matWVPSky * transform;
	
	// modify depth.
	gl_Position.z = gl_Position.w * 0.999999;

#ifdef RECEIVE_FOG
	vec3 fogcolorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
    ofogColor = vec4(fogcolorLinear, ComputeFog(position-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogcolorLinear, position.y);
#endif
}
