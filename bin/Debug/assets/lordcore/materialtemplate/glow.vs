#version 100

#include <fog_vs.inc>

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matW;
uniform mat4 matWVP;

varying vec2 texCoord;

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
 
#ifdef GROW
attribute vec2 inTexCoord1;
varying vec2 texCoord1;
#endif

void main(void)
{
	texCoord = inTexCoord;
	vec4 transform = vec4(inPosition, 1.0);
	
	vec3 position = (matW * transform).xyz;
	gl_Position = matWVP * transform;
	
#ifdef RECEIVE_FOG
    ofogColor = vec4(fogParam[1].rgb, ComputeFog(position-fogParam[2].xyz, fogParam[0].xyz));
	varHeightFog = vec4(fogParam[1].rgb, position.y);
#endif

#ifdef GROW
	texCoord1 = inTexCoord1;
#endif

}
