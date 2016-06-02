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

in  vec2 f_pos;
out vec4 o_color;

void main()
{
	vec2 tex = 0.5*f_pos+0.5;
	if (texture(Gdep, tex).r == 1) discard;

	float dep = texture(Gdep, tex).r*2-1;
	vec4  pos = iP * vec4(f_pos, dep, 1); // clip -> eye
	vec4  nor = texture(Gnor, tex)*2-1;
	vec4  col = texture(Gdif, tex);

	int mi = 1;

	vec3 color = vec3(0,0,0);;
	for (int i=0; i<nlights; ++i) {
		vec3 lcolor = vec3(1,1,1);
		if (i == 0) {
			lcolor = vec3(1,0,0);
		} else if (i == 1) {
			lcolor = vec3(0,1,0);
		}
		vec3 p     = (pos.xyz/pos.w);
		vec3 view  = normalize(p);
		vec3 n     = normalize(nor.xyz/nor.w);

		vec3 l_    = (light[i].xyz/light[i].w) - p;
		vec3 l     = normalize(l_);
		float llen = length(l_);

		// area of influence (This is a HACK)
		if (llen > 100) continue;

		float att  = 1.0 / dot(vec3(1, llen, llen*llen), vec3(0.1, 0.1, 0.5));

		vec3 dif_mat = vec3(1,1,1),
		     spc_mat = vec3(1,1,1);

		vec3 dif = vec3(0,0,0),
		     spc = vec3(0,0,0);

		float ndotl = max(dot(n,l), 0);
		if (ndotl > 0) {
			dif = att * lcolor * ndotl * dif_mat;
			vec3 reflected = reflect(-l,n);
			float angle    = clamp(dot(reflected,view), 0.0, 1.0);
			float shi      = 38.5;
			spc = att * lcolor * pow(angle, shi) * spc_mat; // <- insert material specular + shininess
		}

		if (debug_flag == -1) {
			color += l;
		} else {
			color += dif + spc;
		}
	}

	o_color.rgb = color;
	o_color.a   = 1;

	//for (int i=0; i<nlights; ++i) {
	//	apply_light(pos.xyz / pos.w,
	//				nor.xyz / nor.w,
	//				col,
	//				light[i].xyz / light[i].w,
	//				vec3(0.1, 0.1, 0.05),
	//				mi,
	//				o_color);
	//}
}
