uniform vec4 heightFogParam;
varying vec4 varHeightFog;

float ComputeHeightFog(float worldHeight)
{
	float density = clamp((worldHeight - heightFogParam.x) / -heightFogParam.y, 0.01, 1.0) * heightFogParam.z;
	return density;
}
