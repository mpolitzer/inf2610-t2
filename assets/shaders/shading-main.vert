#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) in vec4 v_pos; // in clip coornates

out vec2 f_tex;

void main() {
	gl_Position = v_pos;
	f_tex = 0.5*(v_pos.xy)+0.5;
}
