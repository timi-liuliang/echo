#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matWVP;
uniform mat4 matW;
//uniform mat4 matWVPWater;

//uniform vec4 OffsetST;

uniform float OffsetST_x;
uniform float OffsetST_y;
uniform float OffsetST_xx;
uniform float OffsetST_yy;


varying vec2 vs_TEXCOORD0;
varying vec3 vs_position;
varying vec3 position;

//varying vec2 texCoord1;
void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	vec4 _MainTex_ST = vec4(OffsetST_x,OffsetST_y,OffsetST_xx,OffsetST_yy);
	vs_TEXCOORD0.xy = inTexCoord.xy * _MainTex_ST.xy + _MainTex_ST.zw;
	
	//¼ÆËã·¢Éä×ø±ê ·Àµ½ÏñËØ½×¶Î
	//mediump vec4 projectUV = matWVPWater * vec4(inPosition, 1.0);
	//projectUV = projectUV/projectUV.w;
	//texCoord1.xy = projectUV.xy;
//	texCoord1.x = (texCoord1.x + 1.0) * 0.5;
	//texCoord1.y = (texCoord1.y + 1.0) * 0.5;


	vs_position = inPosition;

	position = (matW * vec4(inPosition, 1.0)).xyz;	
	
}