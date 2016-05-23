#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform sampler2D depth;
layout(location = 1) uniform sampler2D pos;
layout(location = 2) uniform sampler2D nor;

in  vec2 f_tex;   // <- texture lookup
out vec4 o_color;

void main() {
	o_color = vec4(texture(depth, f_tex).xyz, 1);
	//o_color = vec4(texture(pos, f_tex).xyz, 1);
	//o_color = vec4(texture(nor, f_tex).xyz, 1);
}
