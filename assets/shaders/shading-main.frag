#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

/* -------------------------------------------------------------------------- */

struct material_t {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

material_t materials[] = material_t[](
	material_t( // wut
		vec3(0.05, 0.05, 0.05),
		vec3(0.50, 0.50, 0.50),
		vec3(1.00, 1.00, 1.00), 30.0),
	material_t( // copper
		vec3(0.21, 0.13, 0.05),
		vec3(0.71, 0.43, 0.18),
		vec3(0.39, 0.27, 0.17), 25.6),
	material_t( // obsidian
		vec3(0.05, 0.05, 0.06),
		vec3(0.18, 0.17, 0.22),
		vec3(0.33, 0.32, 0.34), 38.4)
);

/* -------------------------------------------------------------------------- */
layout(location = 0) uniform vec4 u_eye;
layout(location = 1) uniform int debug_flag;
layout(location = 2) uniform mat4 iVP;
layout(location = 3) uniform mat4 V;

layout(location = 9) uniform int nlights;
layout(location =10) uniform vec4 light[50];

layout(binding = 0) uniform sampler2D Gdep;
layout(binding = 1) uniform sampler2D Gnor;
layout(binding = 2) uniform sampler2D Gdif;

in  vec2 f_tex;   // <- texture lookup
in  vec2 f_pos;
out vec4 o_color;

void apply_light(vec3 pos, vec3 nor, vec4 col, vec3 light,
		         vec3 attenuation, int mi, inout vec4 o_color)
{
	vec3  n     = normalize(nor);
	vec3  l     = normalize(light - pos.xyz);
	float llen  = length   (light - pos.xyz);
	float att   = 1.0/dot(vec3(1, llen, llen*llen), attenuation);

	vec3 eye = normalize((u_eye / u_eye.w).xyz);
	vec3 dif = vec3(0,  0,  0),
		 spc = vec3(0,  0,  0);

	vec3 view = normalize(eye - normalize(pos));
	float ndotl = max(dot(n,l), 0.0);

	dif += att * materials[mi].diffuse * ndotl;

	if (ndotl > 0) {
		vec3  reflection = reflect(-l, n);
		float angle      = max(dot(reflection,view), 0.0);
		float shi        = materials[mi].shininess;
		spc              = att * materials[mi].specular*pow(angle, shi);
	}

	o_color.rgb += dif + spc;
}

void main()
{
	if (texture(Gdep, f_tex).r == 1) discard;

	vec4 pos = iVP * vec4(f_pos, texture(Gdep, f_tex).r, 1);
	vec4 nor = (2*texture(Gnor, f_tex)-1);
	vec4 col =  2*texture(Gdif, f_tex)-1;

	int mi = 1;

	if (debug_flag ==-2) { o_color = col; return; }
	if (debug_flag == 1) { o_color = nor;              return; }
	if (debug_flag == 2) { o_color = vec4(pos.xyz, 1); return; }
	if (debug_flag == 3) { o_color = vec4(texture(Gdep, f_tex).rrr, 1); return;
	}

	o_color = vec4(0,0,0,1);
	for (int i=0; i<nlights; ++i) {
		apply_light(pos.xyz / pos.w,
					nor.xyz / nor.w,
					col,
					light[i].xyz / light[i].w,
					vec3(0.1, 0.1, 0.01),
					mi,
					o_color);
	}
}
