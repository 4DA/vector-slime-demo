#version 330

layout(location = 0) in vec4 position;
uniform vec4 basic_offset;
uniform mat4 perspectiveMatrix;

uniform vec4 force_center;
uniform float magnitude;
uniform float T;
uniform vec3 axis;

mat4 rotationMatrix(vec3 _axis, float angle);
mat4 translationMatrix(vec4 tv);

void main()
{
	float displace = length(position - force_center) * magnitude;
	float nt = T - displace;
	mat4 rot = rotationMatrix(axis, nt * 3.14 / 360.0);
	mat4 tr = translationMatrix(basic_offset);
	
	gl_Position =  position * rot * tr * perspectiveMatrix;
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
