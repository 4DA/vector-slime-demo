#version 330

in vec3 mynormal;
out vec4 outputColor;
smooth in vec4 interpColor;	

void main()
{
	outputColor = interpColor;
	/* outputColor = vec4(1.0f, 0.3f, 0.3f, 0.5f); */
}
