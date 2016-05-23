#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 N;

/* in clip coornates */
layout(location = 0) in vec4 v_pos;
layout(location = 1) in vec4 v_nor;
//layout(location = 2) in vec4 v_tan;
//layout(location = 3) in vec2 v_tex;

out vec4 f_pos;
out vec4 f_nor;
//out vec4 f_tan;
//out vec2 f_tex;

void main() {
	gl_Position = MVP * v_pos;
	f_pos = v_pos;
	f_nor = v_nor;
}
