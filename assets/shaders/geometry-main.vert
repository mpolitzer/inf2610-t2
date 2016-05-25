#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 MV;
layout(location = 2) uniform mat4 N;
layout(location = 3) uniform mat4 iVP;
layout(location = 4) uniform mat4 iP;
layout(location = 5) uniform mat4 iMVP;

layout(location = 7) uniform vec4 eye;

layout(binding  = 0) uniform sampler2D  diffuse_map;
layout(binding  = 1) uniform sampler2D   normal_map;
layout(binding  = 2) uniform sampler2D specular_map;

/* in clip coornates */
layout(location = 0) in vec4 v_pos;
layout(location = 1) in vec4 v_nor;
//layout(location = 2) in vec4 v_tan;
//layout(location = 3) in vec2 v_tex;

out vec4 f_pos;
out vec3 f_nor;
//out vec4 f_tan;
//out vec2 f_tex;
//out mat3 f_TBN;

void main() {
	gl_Position = MVP * v_pos;

	//vec3 nor   = normalize(v_nor.xyz);
	//vec3 tg    = normalize(v_tan.xyz);
	//vec3 bit   = normalize(cross(nor, tg) * v_tan.w);

	//f_TBN   = mat3(tg, bit, nor);
	f_pos = (      v_pos);
	f_nor = (N *   v_nor).xyz;
	//f_tan = v_tan;
	//f_tex = v_tex;
}
