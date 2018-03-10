#version 100


attribute vec3 inPosition;
attribute vec2 inTexCoord;

uniform mat4 matPosTrans;
uniform mat4 matTexTrans;


varying vec4 texCoord0;
varying vec4 texCoord1;
varying vec4 texCoord2;
varying vec4 texCoord3;
varying vec4 texCoord4;
//varying vec4 texCoord5;


uniform vec4 sunPositionCS;

void main(void)
{
	gl_Position = matPosTrans*vec4(inPosition, 1.0);
	vec2 texCoord = gl_Position.xy * vec2(0.5,0.5) + vec2(0.5,0.5) + inTexCoord.xy * 0.000001;
	 vec2 sunScreenPos = sunPositionCS.xy;
	 float loopNum = 6.0;
	vec2 dirVec =  (sunScreenPos.xy - texCoord.xy) / (loopNum*2.0); 
	
	
	vec4 tex = texCoord.xyxy;
	tex.zw += dirVec;
	vec4 offset = dirVec.xyxy * 2.0;
	
	texCoord0 = tex;
	tex+=offset;
	texCoord1 = tex;
	tex+=offset;
	texCoord2 = tex;	
		tex+=offset;
	texCoord3 = tex;
		tex+=offset;
	texCoord4 = tex;

}