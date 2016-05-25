#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 MV;
layout(location = 3) uniform mat4 iP;
layout(location = 4) uniform mat4 iVP;
layout(location = 5) uniform mat4 iMVP;



layout(location = 6) uniform int debug_flag;

layout(location = 7) uniform vec4 eye;
layout(location = 8) uniform int nlights;
layout(location = 9) uniform vec4 light[50];

layout(binding = 0) uniform sampler2D dep;
layout(binding = 1) uniform sampler2D pos;
layout(binding = 2) uniform sampler2D nor;
layout(binding = 3) uniform sampler2D dif;

layout(location = 0) in vec4 v_pos; // in clip coornates
out vec2 f_tex;
out vec2 f_pos;

void main() {
	gl_Position = v_pos;
	f_pos = v_pos.xy;
	f_tex = 0.5*(v_pos.xy)+0.5;
}
