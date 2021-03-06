#define TZ_STD_UTIL_INLINE
#include "tz/tz/std/util.h"

#define TZ_GFX_WINDOW_INLINE
#include "tz/tz/gfx/window.h"

#define TZ_GFX_SHADER_INLINE
#include "tz/tz/gfx/shader.h"

#define TZ_GFX_MESH_INLINE
#include "tz/tz/gfx/mesh.h"

#define TZ_GFX_GPU_OBJ_INLINE
#include "tz/tz/gfx/gpu_obj.h"

#define TZ_GFX_MESH_IO_INLINE
#include "tz/tz/gfx/mesh_io.h"

#define TZ_MATH_MAT4_INLINE
#include "tz/tz/math/mat4.h"

#define TZ_GFX_CAM_INLINE
#include "tz/tz/gfx/cam.h"

#define TZ_GFX_TGA_INLINE
#include "tz/tz/gfx/tga.h"

#define TZ_GFX_TEX_INLINE
#include "tz/tz/gfx/tex.h"

#define TZ_STD_UTIL_INLINE
#include "tz/tz/std/util.h"

// models ----------------------------------------------------------------------
#define FLOOR          "../assets/mesh/new_csie_2.ply"

// shaders ---------------------------------------------------------------------
#define GEOMETRY_VERT_NAME "../assets/shaders/geometry-main.vert"
#define GEOMETRY_FRAG_NAME "../assets/shaders/geometry-main.frag"

#define SHADING_VERT_NAME "../assets/shaders/shading-main.vert"
#define SHADING_FRAG_NAME "../assets/shaders/shading-main.frag"

typedef struct game_info_t {
	tz_window w;

	tz_vec4 light[50];
	int    nlights;
	tz_mat4 V, iV, MV, iMV, MVP, iMVP, P, iP, N;
	tz_mat4 VP, iVP;
	tz_mesh floor;
	tz_gpu_obj floor_obj;

	tz_cam  cam;

	GLuint vao, vbo;
	GLuint dbg_vao, dbg_vbo;

	GLuint fbo, tex[5];

	tz_shader geometry, shading;
} game_info;

static game_info _gi;
static int _deffered_shading_debug=0;
static int _enable_bump=0;
static int _enable_spec=0;

static uint32_t tick(uint32_t interval, void *param);
static GLchar *readfile(const char *name);
static void build_program(tz_shader *s, const char *vert, const char *frag);

static void setup_deferred_shading(uint32_t w, uint32_t h);
static void start_geometry_pass();
static void do_geometry_pass();
static void end_geometry_pass();
static void start_shading_pass();
static void do_shading_pass();
static void end_shading_pass();

static void die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	tz_assert(0);
}

static void init(uint32_t w, uint32_t h)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	// setup screen
	if (!tz_window_create(&_gi.w, w, h, true))
		die("can't create window");

	setup_deferred_shading(w, h);
	build_program(&_gi.geometry, GEOMETRY_VERT_NAME, GEOMETRY_FRAG_NAME);
	build_program(&_gi.shading, SHADING_VERT_NAME, SHADING_FRAG_NAME);

	tz_mat4_perspective(&_gi.P, 80, h/(float)w, 1, 1000);
	tz_mat4_inverse(&_gi.iP, &_gi.P);

	//tz_mesh_load_ply(&_gi.class, CLASSROM);
	//tz_mesh_gen_nor(&_gi.class);
	//tz_mesh_gen_tan(&_gi.class);
	//tz_mesh_print_prief(&_gi.class);
	//tz_gpu_obj_load_mesh(&_gi.class_obj, &_gi.class);


	// load floor ---------------------------------------------------------
	//if (!tz_tex_load_tga(&_gi.floor_dif, EYEBOT_DIFFUSE))
	//	die("Failed to open %s\n",    EYEBOT_DIFFUSE);
	//if (!tz_tex_load_tga(&_gi.floor_nor, EYEBOT_NORMAL))
	//	die("Failed to open %s\n",    EYEBOT_NORMAL);
	//if (!tz_tex_load_tga(&_gi.floor_spc, EYEBOT_SPECULAR))
	//	die("Failed to open %s\n",    EYEBOT_SPECULAR);

	tz_mesh_load_ply(&_gi.floor, FLOOR);
	tz_mesh_gen_nor(&_gi.floor);
	tz_mesh_gen_tan(&_gi.floor);
	tz_gpu_obj_load_mesh(&_gi.floor_obj, &_gi.floor);
	// ---------------------------------------------------------------------

	_gi.nlights  = 1;
	for (int i=0; i<5; ++i) {
		for (int j=0; j<5; ++j) {
			_gi.light[i+5*j]    = tz_vec4_mkp(11*i-25, 2, 8*j-19);
			_gi.light[i+5*j+25] = tz_vec4_mkp(11*i-20, 2, 8*j-14);
		}
	}

	// setup camera default position
	_gi.cam.phi   = 0.9*M_PI;
	_gi.cam.theta = 0.75*M_PI;
	_gi.cam.pos   = tz_vec4_mkp(40, 40, 40);

	// setup a redraw callback
	Uint32 delay = (33 / 10) * 10;
	SDL_AddTimer(delay, tick, 0);
}

static void redraw(void) {
	start_geometry_pass();
	do_geometry_pass();
	end_geometry_pass();

	start_shading_pass();
	do_shading_pass();
	end_shading_pass();

	tz_window_flip(&_gi.w);
}

int main(int argc, const char *argv[])
{
	float fast=1.0, fwd=0.0, right=0.0, up=0.0, dt=0.0;
	init(1920, 1080);

	SDL_Event e;
	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_KEYUP: {
			if (e.key.keysym.sym == SDLK_q) {
				goto finalize_program;
			}
			if (e.key.keysym.sym == SDLK_r) {
				build_program(&_gi.geometry, GEOMETRY_VERT_NAME, GEOMETRY_FRAG_NAME);
				build_program(&_gi.shading, SHADING_VERT_NAME, SHADING_FRAG_NAME);
				printf("reloaded shaders\n");
			}

			if (e.key.keysym.sym == SDLK_n) {
				_deffered_shading_debug++;
				printf("%d\n", _deffered_shading_debug);
			}
			if (e.key.keysym.sym == SDLK_p) {
				_deffered_shading_debug--;
				printf("%d\n", _deffered_shading_debug);
			}
			if (e.key.keysym.sym == SDLK_1) {
				_enable_bump = !_enable_bump;
				printf("bump: %d\n", _enable_bump);
			}
			if (e.key.keysym.sym == SDLK_2) {
				_enable_spec = !_enable_spec;
				printf("spec: %d\n", _enable_spec);
			}
			if (e.key.keysym.sym == SDLK_w)     { fwd  +=0.1; }
			if (e.key.keysym.sym == SDLK_s)     { fwd  -=0.1; }
			if (e.key.keysym.sym == SDLK_d)     { right+=0.1; }
			if (e.key.keysym.sym == SDLK_a)     { right-=0.1; }
			if (e.key.keysym.sym == SDLK_SPACE) { up   +=0.1; }
			if (e.key.keysym.sym == SDLK_LCTRL) { up   -=0.1; }
			if (e.key.keysym.sym == SDLK_LSHIFT){ fast  =  1; }
			break;
		}
		case SDL_KEYDOWN: {
			if (e.key.keysym.sym == SDLK_w && e.key.repeat == 0)     { fwd  -=0.1; }
			if (e.key.keysym.sym == SDLK_s && e.key.repeat == 0)     { fwd  +=0.1; }
			if (e.key.keysym.sym == SDLK_a && e.key.repeat == 0)     { right+=0.1; }
			if (e.key.keysym.sym == SDLK_d && e.key.repeat == 0)     { right-=0.1; }
			if (e.key.keysym.sym == SDLK_SPACE && e.key.repeat == 0) { up   -=0.1; }
			if (e.key.keysym.sym == SDLK_LCTRL && e.key.repeat == 0) { up   +=0.1; }
			if (e.key.keysym.sym == SDLK_LSHIFT&& e.key.repeat == 0) { fast  = 10; }

			if (e.key.keysym.sym == SDLK_EQUALS && e.key.repeat == 0) { _gi.nlights = tz_clamp_s32(_gi.nlights+1, 0, 50); }
			if (e.key.keysym.sym == SDLK_MINUS && e.key.repeat == 0)  { _gi.nlights = tz_clamp_s32(_gi.nlights-1, 0, 50); }
			break;
		}
		case SDL_MOUSEMOTION: {
			int x, y;
			if (SDL_GetMouseState(&x, &y)) {
				_gi.cam.theta += 0.5*(2*M_PI*e.motion.xrel / _gi.w.w);
				_gi.cam.phi   += 0.5*(  M_PI*e.motion.yrel / _gi.w.h);
			}
			break;
		}
		case SDL_MOUSEWHEEL: {
			//float inc = SDL_GetModState() ? 0.1 : 1;
			//_gi.cam.rho += inc*e.wheel.y;
			//printf("%f\n", _gi.cam.rho);
			//printf("%d\n", e.wheel.y);
			break;
		}
		case SDL_USEREVENT: {
			dt += 1./60;
			tz_clamp_s32(_gi.nlights, 0, 50);
			for (int i=0; i<_gi.nlights; ++i) {
				float t[4];
				tz_vec4_store4fv(_gi.light[i], t);
				t[1] = 2+sin(dt);
				_gi.light[i] = tz_vec4_load4fv(t);
			}
			redraw();

			tz_cam_fwd(&_gi.cam,    fast*fwd);
			tz_cam_up(&_gi.cam,     fast*up);
			tz_cam_strafe(&_gi.cam, fast*right);
			//tz_vec4_print(_gi.cam.pos);
		}}

		//SDL_WarpMouseGlobal(400, 400);
	}

finalize_program:
	SDL_Quit();
	return 0;
}

static void setup_deferred_shading(uint32_t w, uint32_t h)
{
	glCullFace(GL_BACK);
	glClearColor(0,0,0,1);

	/* create a framebuffer to attach output textures.
	 * create the textures and bind them to the framebuffer object
	 *
	 * textures outputs:
	 * 0) nor
	 * 1) diffuse+shi
	 */

	// depth
	glGenTextures(1, &_gi.tex[0]);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	// nor
	glGenTextures(1, &_gi.tex[1]);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	// diffuse + shininess
	glGenTextures(1, &_gi.tex[2]);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

	// framebuffer
	glGenFramebuffers(1, &_gi.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _gi.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, _gi.tex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gi.tex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gi.tex[2], 0);

	// setup G-buffers
	GLenum fbos[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
	};
	glDrawBuffers(sizeof(fbos)/sizeof(*fbos), fbos);

	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
	case GL_FRAMEBUFFER_COMPLETE:
		printf("Framebuffer is complete\n");
		break;
	default:
		printf("Framebuffer is incomplete !!!\n");
		break;
	}

	// canvas for deferred passes -------------------------------------------
	static const GLfloat strip[] = { // use the whole screen
		-1, -1,
		-1,  1,
		 1, -1,
		 1,  1,
	};
	glGenVertexArrays(1, &_gi.vao);
	glBindVertexArray(_gi.vao);

	glGenBuffers(1, &_gi.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _gi.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof strip, strip,  GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	// debug mesh
	static const GLfloat debug[] = { // use the whole screen
		-0.1, -0.1,
		-0.1,  0.1,
		 0.1, -0.1,
		 0.1,  0.1,
	};
	glGenVertexArrays(1, &_gi.dbg_vao);
	glBindVertexArray(_gi.dbg_vao);

	glGenBuffers(1, &_gi.dbg_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _gi.dbg_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof debug, debug,  GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
}

/* collect geometry into the "this frame's" memory.
 * split into two buckets: transparent and non transparent.
 * (transparent pass is after shading) */
static void start_geometry_pass()
{
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, _gi.fbo);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	tz_shader_bind(&_gi.geometry);

	tz_cam_mkview(&_gi.cam, &_gi.V);
	tz_mat4_mul(&_gi.VP, &_gi.P, &_gi.V);
}

/* fill G-objects (getting scene ready for shading pass) */
static void do_geometry_pass()
{
	float eye[4];

	tz_vec4_store4fv(_gi.cam.pos, eye);
	glUniform4fv(7, 1, eye);

	for (int i=0; i<_gi.nlights; ++i) {
		tz_mat4 model_matrix, modelview_matrix, normal_matrix, mvp_matrix;

		tz_mat4_set_translation(&model_matrix, _gi.light[i]);
		tz_mat4_mul(&modelview_matrix, &_gi.V, &model_matrix);
		tz_mat4_mul(&mvp_matrix, &_gi.VP, &model_matrix);

		tz_mat4_mul(&normal_matrix, &_gi.V, &model_matrix);
		tz_mat4_inverse(&normal_matrix, &normal_matrix);
		tz_mat4_transpose(&normal_matrix, &normal_matrix);

		glUniformMatrix4fv(0, 1, GL_TRUE,  modelview_matrix.f);
		glUniformMatrix4fv(1, 1, GL_TRUE,  mvp_matrix.f);
		glUniformMatrix4fv(2, 1, GL_TRUE,  normal_matrix.f);

		glBindVertexArray(_gi.dbg_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	tz_mat4 model_matrix, modelview_matrix, normal_matrix, mvp_matrix;

	tz_mat4_mkidentity(&model_matrix);
	tz_mat4_mul(&modelview_matrix, &_gi.V, &model_matrix);
	tz_mat4_mul(&mvp_matrix, &_gi.VP, &model_matrix);

	tz_mat4_mul(&normal_matrix, &_gi.V, &model_matrix);
	tz_mat4_inverse(&normal_matrix, &normal_matrix);
	tz_mat4_transpose(&normal_matrix, &normal_matrix);

	glUniformMatrix4fv(0, 1, GL_TRUE,  modelview_matrix.f);
	glUniformMatrix4fv(1, 1, GL_TRUE,  mvp_matrix.f);
	glUniformMatrix4fv(2, 1, GL_TRUE,  normal_matrix.f);

	tz_gpu_obj_draw(&_gi.floor_obj);
}

static void end_geometry_pass()
{
}

static void start_shading_pass()
{
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

#if 1
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[0]);

	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[1]);

	glActiveTexture(GL_TEXTURE0+2);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[2]);
#if 1
	glActiveTexture(GL_TEXTURE0+3);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[3]);
#endif
#else
	glBindTextures(...);
#endif
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(GL_ONE, GL_ONE);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _gi.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	tz_shader_bind(&_gi.shading);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	float tmp[4];

	glBindVertexArray(_gi.vao);

	tz_vec4_store4fv(_gi.cam.pos, tmp);
	glUniform4fv(0, 1, tmp);

	glUniform1i(1, _deffered_shading_debug);

	glUniformMatrix4fv(2, 1, GL_TRUE,  _gi.iP.f);
	glUniformMatrix4fv(3, 1, GL_TRUE,  _gi.iV.f);
	glUniformMatrix4fv(4, 1, GL_TRUE,  _gi.V.f);
	glUniformMatrix4fv(5, 1, GL_TRUE,  _gi.P.f);


	tz_vec4 tmp_lights[50];
	for (int i=0; i<_gi.nlights; ++i) {
		// world
		//tmp_lights[i] = _gi.light[i];

		// world -> eye
		tmp_lights[i] = tz_mat4_mulv(&_gi.V, _gi.light[i]);
	}

	glUniform1i(9,  _gi.nlights);
	glUniform4fv(10, _gi.nlights, (float *)tmp_lights);

}

static void do_shading_pass()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void end_shading_pass()
{
}

#if 0
// run after last shading_pass
static void make_transparent_pass() {}

static void do_transparent_pass() {
	//for p in programs
	//	for t in textures
	//		for m in material
	//			for u in unifomrs
}
static void end_transparent_pass() {}
#endif

static uint32_t tick(uint32_t interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return interval;
}

static GLchar *readfile(const char *name)
{
	char *data;
	size_t len;
	SDL_RWops *rw = SDL_RWFromFile(name, "rb");
	SDL_assert(rw && "failed to read file.");

	len = SDL_RWseek(rw, 0, SEEK_END);
	data = malloc(len+1);
	data[len] = '\0';

	SDL_RWseek(rw, 0, SEEK_SET);
	SDL_RWread(rw, data, len, 1);
	SDL_RWclose(rw);
	return data;
}

static void build_program(tz_shader *s, const char *vert, const char *frag)
{
	GLchar *file_contents;

	tz_shader_create(s);
	if ((file_contents = readfile(vert))) {
		tz_shader_compile(s, file_contents, GL_VERTEX_SHADER);
		free(file_contents);
	}
	else die("vertex shader `%s' not found", vert);

	if ((file_contents = readfile(frag))) {
		tz_shader_compile(s, file_contents, GL_FRAGMENT_SHADER);
		free(file_contents);
	}
	else die("fragment shader `%s' not found", frag);

	if (!tz_shader_link(s))
		die("link failed\n");

	tz_shader_bind(s);
}

