#version 100

uniform vec2 EyeToSourceUVScale;
uniform vec2 EyeToSourceUVOffset;
uniform mat4 EyeRotationStart;
uniform mat4 EyeRotationEnd;

attribute vec2 ScreenPosNDC;
attribute vec4 inColor;
attribute vec2 TAN_EYE_ANGLE_R;
attribute vec2 TAN_EYE_ANGLE_G;
attribute vec2 TAN_EYE_ANGLE_B;

varying vec4 oColor;
varying vec2 oTexCoord0;
varying vec2 oTexCoord1;
varying vec2 oTexCoord2;
void main()
{
   gl_Position.x = ScreenPosNDC.x;
   gl_Position.y = ScreenPosNDC.y;
   gl_Position.z = 0.0;
   gl_Position.w = 1.0;
   vec3 TanEyeAngleR = vec3 ( TAN_EYE_ANGLE_R.x, TAN_EYE_ANGLE_R.y, 1.0 );
   vec3 TanEyeAngleG = vec3 ( TAN_EYE_ANGLE_G.x, TAN_EYE_ANGLE_G.y, 1.0 );
   vec3 TanEyeAngleB = vec3 ( TAN_EYE_ANGLE_B.x, TAN_EYE_ANGLE_B.y, 1.0 );
   vec3 TransformedRStart = (EyeRotationStart * vec4(TanEyeAngleR, 0)).xyz;
   vec3 TransformedGStart = (EyeRotationStart * vec4(TanEyeAngleG, 0)).xyz;
   vec3 TransformedBStart = (EyeRotationStart * vec4(TanEyeAngleB, 0)).xyz;
   vec3 TransformedREnd   = (EyeRotationEnd * vec4(TanEyeAngleR, 0.0)).xyz;
   vec3 TransformedGEnd   = (EyeRotationEnd * vec4(TanEyeAngleG, 0.0)).xyz;
   vec3 TransformedBEnd   = (EyeRotationEnd * vec4(TanEyeAngleB, 0.0)).xyz;
   vec3 TransformedR = mix ( TransformedRStart, TransformedREnd, inColor.a );
   vec3 TransformedG = mix ( TransformedGStart, TransformedGEnd, inColor.a );
   vec3 TransformedB = mix ( TransformedBStart, TransformedBEnd, inColor.a );
   float RecipZR = 1.0 / TransformedR.z;
   float RecipZG = 1.0 / TransformedG.z;
   float RecipZB = 1.0 / TransformedB.z;
   vec2 FlattenedR = vec2 ( TransformedR.x * RecipZR, TransformedR.y * RecipZR );
   vec2 FlattenedG = vec2 ( TransformedG.x * RecipZG, TransformedG.y * RecipZG );
   vec2 FlattenedB = vec2 ( TransformedB.x * RecipZB, TransformedB.y * RecipZB );
   vec2 SrcCoordR = FlattenedR * EyeToSourceUVScale + EyeToSourceUVOffset;
   vec2 SrcCoordG = FlattenedG * EyeToSourceUVScale + EyeToSourceUVOffset;
   vec2 SrcCoordB = FlattenedB * EyeToSourceUVScale + EyeToSourceUVOffset;
   oTexCoord0 = SrcCoordR;
   oTexCoord1 = SrcCoordG;
   oTexCoord2 = SrcCoordB;
   oColor = vec4(inColor.r, inColor.r, inColor.r, inColor.r);
}