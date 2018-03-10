#version 100


attribute vec3 inPosition;
attribute vec2 inTexCoord;

uniform mat4 matPosTrans;
uniform mat4 matTexTrans;


varying vec4 texCoord0;
varying vec4 texCoord1;
varying vec4 texCoord2;

uniform vec4 sunPositionCS;

void main(void)
{
	gl_Position = matPosTrans*vec4(inPosition, 1.0);
	vec2 texCoord = gl_Position.xy * vec2(0.5,0.5) + vec2(0.5,0.5) + inTexCoord.xy * 0.000001;
	
	vec2 toTC = texCoord - sunPositionCS.xy; 
	texCoord0.xy = sunPositionCS.xy;
	texCoord0.zw = sunPositionCS.xy + toTC * 0.2;

	texCoord1.xy = sunPositionCS.xy + toTC * 0.4;
	texCoord1.zw = sunPositionCS.xy + toTC * 0.6;
	
	texCoord2.xy = sunPositionCS.xy + toTC * 0.8;
	texCoord2.zw = sunPositionCS.xy + toTC;
}