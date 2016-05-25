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
layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 MV;
layout(location = 3) uniform mat4 iP;
layout(location = 4) uniform mat4 iVP;
layout(location = 5) uniform mat4 iMVP;


layout(location = 6) uniform int debug_flag;

layout(location = 7) uniform vec4 eye;
layout(location = 8) uniform int nlights;
layout(location = 9) uniform vec4 light[50];

layout(binding = 0) uniform sampler2D Gdep;
layout(binding = 1) uniform sampler2D Gpos;
layout(binding = 2) uniform sampler2D Gnor;
layout(binding = 3) uniform sampler2D Gdif;

in  vec2 f_tex;   // <- texture lookup
in  vec2 f_pos;
out vec4 o_color;

// in eye space
void apply_light(
		vec3 pos,
		vec3 nor,
		vec4 col,
		vec3 light,
		vec3 attenuation,
		int mi,
		inout vec4 o_color)
{
		vec3  l     = normalize(light - pos);
		float llen  = length   (light - pos);
		float att   = 1.0/dot(vec3(1, llen, llen*llen), attenuation);

		vec3 amb = vec3(0,0,0),
		     dif = vec3(0,0,0),
			 spc = vec3(0,0,0);

		//amb += vec3(.01, .01, .01);

		vec3 view = normalize(pos);
		vec3 n = normalize(nor);
		float ndotl = max(dot(l,n), 0.0);

		if (ndotl > 0) {
			dif += att * materials[mi].diffuse * ndotl;

			vec3  reflection = reflect(-l, nor);
			float angle      = max(dot(reflection,view), 0.0);
			float shi        = materials[mi].shininess;
			spc              = att * materials[mi].specular*pow(angle, shi);
		}

#if 1
		o_color.rgb += (amb+dif) * col.rgb + spc * col.a;
#else
		o_color.rgb = view;
#endif
}

void main() {
	if (texture(Gdep, f_tex).r == 1)
		discard;

#if 1
	// TODO: pass as uniform
	float f = 1000.0;
	float n = 1.0;

	// from clip -> eye
	float z = (2 * n) / (f + n - texture(Gdep, f_tex).r * (f - n));
	vec4 pos = iP *  vec4(f_pos, z, 1);

	// from world -> eye
	vec4 nor = texture(Gnor, f_tex);

#else
	vec4 pos = vec4(texture(Gpos, f_tex).xyz, 1);
	// from world -> eye
	vec4 nor = texture(Gnor, f_tex);
#endif

	vec4 col = texture(Gdif, f_tex);
	int mi = 0;

	if (debug_flag == 1) {
		o_color = nor;
		return;
	}
	if (debug_flag == 2) {
		o_color = vec4(pos.xyz, 1);
		return;
	}
	if (debug_flag == 3) {
		o_color = vec4(texture(Gpos, f_tex).rgb, 1);
		return;
	}
	if (debug_flag == 4) {
		o_color = vec4(texture(Gdep, f_tex).rrr, 1);
		return;
	}
	if (debug_flag == 5) {
		o_color = col;
		return;
	}

	o_color = vec4(0,0,0,1);
	for (int i=0; i<nlights; ++i) {
		vec4 l = MV * light[i];
		apply_light(
			pos.xyz,
			nor.xyz,
			col,
			light[i].xyz,
			vec3(0.1, 0.1, 0.01),
			mi,
			o_color);
	}
}
