#version 330

layout(location = 0) in vec4 position;
uniform vec4 basic_offset;
uniform mat4 perspectiveMatrix;
uniform vec4 force_center;
uniform float magnitude;
uniform float T;

void main()
{
    gl_Position = perspectiveMatrix * (position + basic_offset);
}
