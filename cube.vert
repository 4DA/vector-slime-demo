#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

uniform vec4 force_center;
uniform float magnitude;
uniform float T;

uniform vec3 ax1, ax2, ax3, ax4;
uniform int t1, t2, t3, t4;
uniform float dmod1, dmod2, dmod3, dmod4;

uniform vec4 basic_offset;
uniform mat4 perspectiveMatrix;
uniform vec3 light_direction;
uniform vec4 light_intensity;


smooth out vec4 interpColor;

mat4 rotationMatrix(vec3 _axis, float angle);
mat4 translationMatrix(vec4 tv);

void main()
{
	float displace = magnitude * length(position - force_center);
	float nt =  T - t1 + displace * dmod1;  
	float nt2 = T - t2 + displace * dmod2; 
	float nt3 = T - t3 + displace * dmod3; 
	float nt4 = T - t4 + displace * dmod4; 
	
	nt  = max(nt,  0.01);
	nt2 = max(nt2, 0.01);
	nt3 = max(nt3, 0.01);
	nt4 = max(nt4, 0.01);

	mat4 rot = rotationMatrix (ax1, nt * 3.14 / 360.0);
	mat4 rot2 = rotationMatrix(ax2, nt2 * 3.14 / 360.0);
	mat4 rot3 = rotationMatrix(ax3, nt3 * 3.14 / 360.0);
	mat4 rot4 = rotationMatrix(ax4, nt4 * 3.14 / 360.0);
	mat4 tr = translationMatrix(basic_offset);

	mat4 full_tr = rot4 * rot3 * rot2 * rot * tr;
		
	gl_Position =  position * full_tr * perspectiveMatrix;

	vec4 tnormal = normal * full_tr;
	
	float cosAngIncidence = dot(normalize(vec3(tnormal)),
				    normalize(vec3(-light_direction)));

	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	
	vec4 diffuseColor = vec4(0.1, 0.5, 1.0, 1.0);
	
	interpColor = cosAngIncidence * diffuseColor;
	interpColor.a = 1.0;
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

}
