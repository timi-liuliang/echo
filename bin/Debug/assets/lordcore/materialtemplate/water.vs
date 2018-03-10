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

uniform mediump float water_scale;
uniform mediump float water_speed;
uniform mediump vec3  wave_vec;

varying vec4 ofogColor;
varying vec2 texCoord;
varying vec2 texCoord1;
varying vec2 texCoord2;

varying vec3 texnormal;
varying vec3 vViewVec;
varying vec3 vWaveVec;

varying vec2 bumpuv0;
varying vec2 bumpuv1;

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
	
	vec4 wolrd_pos = matW*vec4(inPosition, 1.0);
	vViewVec.xzy = normalize(camPos.xyz - wolrd_pos.xyz);

	vWaveVec = vec3(0.6,0.3,0.7) * wave_vec;


	
	vec4 _WaveScale4 = vec4(0.063,0.063,0.0252,0.02835) * vec4(water_scale);
	vec4 _Offset4 = vec4(CurrentTime * 0.05 * water_speed);
	vec4 _WaveOffset = vec4(9.0,4.5,-8.0,-3.5) * _WaveScale4 * _Offset4;
	_WaveOffset = _WaveOffset - floor(_WaveOffset);
	
	vec4 temp = wolrd_pos.xzxz * _WaveScale4 + _WaveOffset;
	bumpuv0 = temp.xy;
	bumpuv1 = temp.wz;
	
	texCoord1 = inTexCoord1;
	
	vec3 fogColorLinear = fogParam[1].rgb * fogParam[1].rgb * 3.0;
	
	//È¥³ý·¨Ïß
	ofogColor = vec4(fogColorLinear+inNormal*vec3(0.00001,0.00001,0.00001), ComputeFog((matW*vec4(inPosition, 1.0)).xyz-fogParam[2].xyz, fogParam[0].xyz));
	
}
