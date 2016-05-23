#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 N;

in  vec4 f_pos;
in  vec4 f_nor;
out vec3 o_color;

void main() {
	o_color = normalize(f_nor.xyz);
}
