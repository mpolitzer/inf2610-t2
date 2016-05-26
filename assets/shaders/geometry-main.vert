#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 M;
layout(location = 1) uniform mat4 VP;
layout(location = 2) uniform mat4 N;

layout(location = 0) in vec4 v_pos;
layout(location = 1) in vec4 v_nor;

out vec4 f_pos;
out vec4 f_nor;

void main() {
	gl_Position = VP * M * v_pos;
	f_nor = (N   * v_nor);
}
