#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 VP;
layout(location = 2) uniform mat4 N;

in vec4 f_pos;
in vec4 f_nor;
out vec4 o_data[2];

void main()
{
	o_data[0] = f_nor;
	o_data[1] = vec4(1,1,1,1); // <- material
}
