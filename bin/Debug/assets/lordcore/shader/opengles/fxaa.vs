#version 100

attribute vec3 inPosition;
attribute vec2 inTexCoord;


varying vec2 texCoord; 

void main(void)
{
    gl_Position = vec4( inPosition.x, inPosition.y, 0.0, 1.0);
    texCoord    = inTexCoord;
}
