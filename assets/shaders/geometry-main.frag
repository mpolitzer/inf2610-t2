#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 N;

in vec4 f_pos;
in vec4 f_nor;
//in vec4 f_tan;
//in vec2 f_tex;
//out vec4 o_color;

void main() {
	gl_FragData[0]    = f_pos;
	gl_FragData[1]    = vec4(f_nor.xyz, 0);
	//gl_FragData[2]    = f_tan;
	//gl_FragData[4].xy = f_tex;
}
