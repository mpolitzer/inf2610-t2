#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 1) uniform int debug_flag;
layout(location = 2) uniform mat4 iP;
layout(location = 3) uniform mat4 iV;
layout(location = 4) uniform mat4 V;
layout(location = 5) uniform mat4 P;

layout(location = 9) uniform int nlights;
layout(location =10) uniform vec4 light[50];

layout(binding = 0) uniform sampler2D Gdep;
layout(binding = 1) uniform sampler2D Gnor;
layout(binding = 2) uniform sampler2D Gdif;
layout(binding = 3) uniform sampler2D Gpos;

layout(location = 0) in vec4 v_pos; // in clip coornates

out vec2 f_pos;

void main() {
	gl_Position = v_pos;
	f_pos       = v_pos.xy;
}
