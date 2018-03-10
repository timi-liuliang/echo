#version 100

attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inTexCoord1;

uniform mat4 matWVP;
uniform mat4 matW;

//uniform vec4 OffsetST;

uniform float OffsetST_x;
uniform float OffsetST_y;
uniform float OffsetST_xx;
uniform float OffsetST_yy;




varying vec2 vs_TEXCOORD0;
varying mediump vec3 position;


void main(void)
{
	gl_Position = matWVP * vec4(inPosition, 1.0);

	vec4 _MainTex_ST = vec4(OffsetST_x,OffsetST_y,OffsetST_xx,OffsetST_yy);
	vs_TEXCOORD0.xy = inTexCoord.xy * _MainTex_ST.xy + _MainTex_ST.zw;

	position = (matW * vec4(inPosition, 1.0)).xyz;	
}