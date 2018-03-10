#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matW;
uniform mat4 matWVP;
uniform mat4 matWVPWater;
uniform vec3 camPos;


uniform vec4 fogParam[3];

uniform mediump float CurrentTime;
uniform mediump float water_speed;
uniform mediump float water_scale;


varying vec4 ofogColor;
varying vec2 texCoord;
varying vec2 texCoord1;
varying vec2 texCoord2;
varying vec3 texnormal;
varying vec3 xlv_TEXCOORD0;
varying vec4 xlv_TEXCOORD1;

float ComputeFog(vec3 camToWorldPos, vec3 param)
{
	float fdist = max(length(camToWorldPos) - param.x, 0.0);
	float density = clamp(clamp(fdist/param.y, 0.0, 1.0) * param.z, 0.0, 1.0);
	return 1.0 - density;
}

void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);
	texCoord = inTexCoord;
	
	mediump vec4 projectUV = matWVPWater * vec4(inPosition, 1.0);
	projectUV = projectUV/projectUV.w;
	texCoord.xy = projectUV.xy;
	texCoord.x = (texCoord.x + 1.0) * 0.5;
	texCoord.y = (texCoord.y + 1.0) * 0.5;
	
	projectUV = matWVP * vec4(inPosition, 1.0);
	projectUV = projectUV/projectUV.w;
	texCoord2.xy = projectUV.xy;
	texCoord2.x = (texCoord2.x + 1.0) * 0.5;
	texCoord2.y = (texCoord2.y + 1.0) * 0.5;
	
	texnormal = inNormal;
	
	
	mediump vec4 _Time = vec4(0.05 * CurrentTime) * vec4(water_speed);
	mediump vec4 _BumpDirection = vec4(0.2,1.0,10.0,-4.0);
	mediump vec4 _BumpTiling = vec4(0.1,0.1,0.05,0.05);
	vec4 wolrd_pos = matW*vec4(inPosition, 1.0);
	xlv_TEXCOORD0 = wolrd_pos.xyz - camPos.xyz;
	xlv_TEXCOORD1 = ((wolrd_pos.xzxz + (_Time.xxxx * _BumpDirection)) * _BumpTiling) * vec4(water_scale);

	
	vec3 fogColorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
	
	//È¥³ý·¨Ïß
	ofogColor = vec4(fogColorLinear+inNormal*vec3(0.00001,0.00001,0.00001), ComputeFog((matW*vec4(inPosition, 1.0)).xyz-fogParam[2].xyz, fogParam[0].xyz));
	
}