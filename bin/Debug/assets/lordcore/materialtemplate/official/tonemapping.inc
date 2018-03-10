
vec3 hv(vec3 c)
{
	vec3 hA = sqrt(c);
	return(hA - hA*c) + c*(0.4672*c + vec3(0.5328));
}

float linear_to_srgb(float c)
{
	return sqrt(c);
}

float srgb_to_linear(float c)
{
	return c * c;
}

vec3 linear_to_srgb(vec3 rgb)
{
	return vec3(linear_to_srgb(rgb.x),linear_to_srgb(rgb.y),linear_to_srgb(rgb.z));
}

vec3 srgb_to_linear(vec3 srgb)
{
	return vec3(srgb_to_linear(srgb.x),srgb_to_linear(srgb.y),srgb_to_linear(srgb.z));
}

vec3 toneExp(vec3 color)
{
	return vec3(1.0, 1.0, 1.0) - exp(-color);
}

vec3 toneACESlum(vec3 color, float adaptedLum)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	
	color *= adaptedLum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

vec3 toneACES(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

vec3 toneHejl( vec3 c )
{
	// Hejl / Burgess-Dawson tone mapping.
	// Thanks to John Hable and Chris Perrella for guidance.
	const float	A = 0.22, B = 0.3, C = .1, D = 0.2, E = .01, F = 0.3;
	const float Scl = 1.25;

	vec3 h = max( vec3(0.0,0.0,0.0), c - vec3(.004,.004,.004) );
	return	(h*((Scl*A)*h+Scl*vec3(C*B,C*B,C*B))+Scl*vec3(D*E,D*E,D*E)) /
			(h*(A*h+vec3(B,B,B))+vec3(D*F,D*F,D*F)) -
			Scl*vec3(E/F,E/F,E/F);
}

vec3 toneMapping(vec3 color)
{
	return toneHejl(color);
}

vec4 texture2d_linear( sampler2D sampler, vec2 uv)
{
	vec4 color = texture2D(sampler, uv);
	color.xyz = srgb_to_linear(color.xyz);
	return color;
} 

float rgb_to_gray(float r, float g, float b)
{
	float gray = r * 0.299 + g * 0.587 + b * 0.114;
	return gray;
}

