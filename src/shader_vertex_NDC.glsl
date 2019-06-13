#version 330 core

layout (location = 0) in vec4 NDC_coefficients;

void main()
{
    gl_Position = NDC_coefficients;
}
