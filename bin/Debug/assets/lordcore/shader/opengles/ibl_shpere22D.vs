attribute vec2 inTexCoord;
attribute vec3 inPosition;
uniform mat4 matWVP;
uniform vec3 planeNormal;
varying vec2 fCoord;

mat2 mirror(vec2 n)
{
	return mat2(
			1.0-2.0*n.x*n.x,    -2.0*n.x*n.y,
			   -2.0*n.x*n.y, 1.0-2.0*n.y*n.y
		 );
}

//旋转90度
mat2 rotation =  mat2(
						0.0,  1.0,
					   -1.0,  0.0
					 );

void main(void)
{
	vec3 p1 = vec3(0,1,0);
	vec3 normal = planeNormal;
	if(inPosition.y < 0.0)
	{
		normal.y = -normal.y;
		p1.y = -1.0; 
	}
		
	
	normal = normalize(normal);
	vec3 pos =  normalize(inPosition);
	pos = dot(normal,p1)/dot(normal,pos) * pos;

	if(pos.y < 0.0)
	{	
		pos.xz = mirror(normalize(planeNormal.xz)) * pos.xz + planeNormal.xz;
	}
	pos.xz  = mirror(vec2(1.0,0.0)) * rotation * pos.xz; 
	pos.y = pos.z;
	pos.z = 0.0;
	
	gl_Position = vec4(pos, 1.0);
	fCoord = inTexCoord;
}

