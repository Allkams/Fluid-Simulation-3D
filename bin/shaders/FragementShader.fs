#version 330 core
#extension GL_ARB_separate_shader_objects : enable
layout (location=0) in vec4 color;

out vec4 FragColor;

void main()
{
	FragColor = color;
}