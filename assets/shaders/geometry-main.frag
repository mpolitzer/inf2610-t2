#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 MV;
layout(location = 2) uniform mat4 N;
layout(location = 3) uniform mat4 iVP;
layout(location = 4) uniform mat4 iP;
layout(location = 5) uniform mat4 iMVP;

layout(binding  = 0) uniform sampler2D  diffuse_map;
layout(binding  = 1) uniform sampler2D   normal_map;
layout(binding  = 2) uniform sampler2D specular_map;

in vec4 f_pos;
in vec3 f_nor;
//in vec4 f_tan;
//in vec2 f_tex;
//in mat3 f_TBN;
out vec4 o_data[3];

void main()
{
	o_data[0] = f_pos / f_pos.w;

	vec3 n = f_nor;
	//if (enable_bump != 0) {
	//	vec4 normal  = texture(normal_map, f_tex);
	//	n += f_TBN * (2*normal.rgb-1);
	//}
	o_data[1] = vec4(normalize(n), 1);
	
	vec4 col = vec4(1,1,1,1);
	//if (enable_spec != 0) {
	//	vec4 diffuse = texture( diffuse_map, f_tex);
	//	vec4 specular= texture(specular_map, f_tex);
	//	c.rgba = vec4(diffuse.rgb, specular.r);
	//}

	o_data[2] = col;
}
