#version 330

layout(location = 0) in vec4 position;
uniform vec4 basic_offset;
uniform mat4 perspectiveMatrix;

uniform vec4 force_center;
uniform float magnitude;
uniform float T;


mat4 rotationMatrix(vec3 _axis, float angle);
mat4 translationMatrix(vec4 tv);

void main()
{
	float displace = magnitude * length(position - force_center);
	float nt = T - 500 + displace;
	float nt2 = T - 750 + displace/4;
	float nt3 = T - 700 + displace;
	float nt4 = T - 870 + displace/4;
	
	nt  = clamp(nt,  0.01, 720);
	nt2 = clamp(nt2, 0.01, 720);
	nt3 = clamp(nt3, 0.01, 720);
	nt4 = clamp(nt4, 0.01, 720);

	vec3 axis = vec3(0, 1,-0.2);
	vec3 axis2 = vec3(0.4, 0.4,0.0);
	vec3 axis3 = vec3(0, -0.6, 0.2);
	vec3 axis4 = vec3(-0.2, -0.3, -0.0);
	
	mat4 rot = rotationMatrix(axis, nt * 3.14 / 360.0);
	mat4 rot2 = rotationMatrix(axis2, nt2 * 3.14 / 360.0);
	mat4 rot3 = rotationMatrix(axis3, nt3 * 3.14 / 360.0);
	mat4 rot4 = rotationMatrix(axis4, nt4 * 3.14 / 360.0);
	mat4 tr = translationMatrix(basic_offset);
	
	gl_Position =  position * rot4 * rot3 * rot2 * rot * tr * perspectiveMatrix;
}

mat4 rotationMatrix(vec3 _axis, float angle)
{
	vec3 axis = normalize(_axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
		    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
		    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
		    0.0, 0.0, 0.0, 1.0);
}

mat4 translationMatrix(vec4 tv)
{
	return mat4(1.0, 0.0, 0.0, tv.x,
		    0.0, 1.0, 0.0, tv.y,
		    0.0, 0.0, 1.0, tv.z,
		    0.0, 0.0, 0.0, 1.0);

	/* return mat4(1.0, 0.0, 0.0, 0.19, */
	/* 	    0.0, 1.0, 0.0, 0, */
	/* 	    0.0, 0.0, 1.0, -6.0, */
	/* 	    0.0, 0.0, 0.0, 1.0); */

}
