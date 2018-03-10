#version 100

attribute vec3 inPosition;
attribute vec4 inColor;

uniform mat4 worldMat;
uniform mat4 viewProjMat;
uniform vec3 viewPos;
varying vec4 color;
varying vec3 normal;
varying vec3 view_dir;
varying vec3 world_pos;
void main(void)
{
	gl_Position = worldMat * vec4(inPosition, 1.0);
	world_pos = gl_Position.xyz;
	vec3 world_view_pos = (worldMat * vec4(viewPos, 1.0)).xyz;
	view_dir = (world_view_pos - world_pos);
	gl_Position = viewProjMat * gl_Position;
	normal = inPosition.xyz;
	color = inColor;

}