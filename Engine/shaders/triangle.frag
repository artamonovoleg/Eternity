#version 450
#extension GL_ARB_separate_shader_objects : enable

//shader input
layout (location = 0) in vec3 inColor;

//output write
layout (location = 0) out vec4 outFragColor;


void main() 
{
	//return color
	outFragColor = vec4(inColor,1.0f);
}