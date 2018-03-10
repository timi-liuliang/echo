#version 100

attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;

uniform mat4 matWVP;

uniform float CurrentTime;  // seconds

uniform vec2  SA_UV_segment;
uniform float SA_speed;
uniform vec2 SA_texCoordAdjust;

varying vec2  SA_inTexCoord;

void main ()
{
	vec4 P = vec4(inPosition.xyz, 1.0);
	
	gl_Position = matWVP * P;

	float texId = floor(CurrentTime / SA_speed);
	texId = mod(texId,SA_UV_segment.x*SA_UV_segment.y);
	SA_inTexCoord.x = mod(texId,SA_UV_segment.x);
	SA_inTexCoord.y = floor(texId/SA_UV_segment.x);
	SA_inTexCoord.x += inTexCoord.x;
	SA_inTexCoord.y += inTexCoord.y / 2.0; //1 - (1 - inTexCoord.y) / 2; // upper half part of the imagery
	SA_inTexCoord.x += SA_texCoordAdjust.x;
	SA_inTexCoord.y += SA_texCoordAdjust.y / 2.0;
	SA_inTexCoord /= SA_UV_segment;
}



