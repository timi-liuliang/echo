uniform sampler2D Mask;
uniform vec3 hsvColorRChannel;
uniform vec3 hsvColorGChannel;
uniform vec3 hsvColorBChannel;

void hsv_color_calc(inout vec3 diffuse)
{
	vec3 colorMask = texture2D(Mask, texcoord0).rgb;
	vec3 hsvColor0 = hsvColorRChannel * colorMask.rrr;
	vec3 hsvColor1 = hsvColorGChannel * colorMask.ggg;
	vec3 hsvColor2 = hsvColorBChannel * colorMask.bbb;
	hsvColor0 = hsvColor0 + vec3(1.0 - colorMask.r);
	hsvColor1 = hsvColor1 + vec3(1.0 - colorMask.g);
	hsvColor2 = hsvColor2 + vec3(1.0 - colorMask.b);
	diffuse = diffuse * hsvColor0 * hsvColor1 * hsvColor2;
}