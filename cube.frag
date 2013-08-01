/* ; -*- mode: c;-*- */
#version 330

out vec4 outputColor;
smooth in vec4 gsInterpColor;	

void main()
{
	outputColor = gsInterpColor;
	/* outputColor = vec4(1.0f, 0.3f, 0.3f, 0.5f); */
}
