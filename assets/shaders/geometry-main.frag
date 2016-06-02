#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MV;
layout(location = 1) uniform mat4 MVP;
layout(location = 2) uniform mat4 N;

in  vec4 f_nor;
in  vec4 f_pos;
out vec4 o_data[2];

void main()
{
	o_data[0] = vec4(f_nor.xyz*.5+.5, 1);
	o_data[1] = vec4(1,1,1,1); // <- material
}
