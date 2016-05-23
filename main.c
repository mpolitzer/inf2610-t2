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

//#define CLASSROM "../assets/mesh/cube.ply"
//#define CLASSROM "../assets/mesh/classrom.ply"
//#define CLASSROM "../assets/mesh/predio.ply"
#define CLASSROM "../assets/mesh/metropolis.ply"
#define EYEBOT "../assets/mesh/eyebot.ply"

#define REGULAR_VERT_NAME "../assets/shaders/regular-main.vert"
#define REGULAR_FRAG_NAME "../assets/shaders/regular-main.frag"

#define GEOMETRY_VERT_NAME "../assets/shaders/geometry-main.vert"
#define GEOMETRY_FRAG_NAME "../assets/shaders/geometry-main.frag"

#define SHADING_VERT_NAME "../assets/shaders/shading-main.vert"
#define SHADING_FRAG_NAME "../assets/shaders/shading-main.frag"

typedef struct game_info_t {
	tz_window w;

	tz_mat4 P;
	tz_mesh class, eyebot;
	tz_gpu_obj class_obj, eyebot_obj;
	tz_cam  cam;

	GLuint vao, vbo;

	GLuint fbo, depth, tex[5];

	tz_shader geometry, shading, regular;
} game_info;
static game_info _gi;
static int _use_deferred_shading=true;

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
	if (!tz_window_create(&_gi.w, w, h, false))
		die("can't create window");

	setup_deferred_shading(w, h);
	build_program(&_gi.geometry, GEOMETRY_VERT_NAME, GEOMETRY_FRAG_NAME);
	build_program(&_gi.shading, SHADING_VERT_NAME, SHADING_FRAG_NAME);
	build_program(&_gi.regular, REGULAR_VERT_NAME, REGULAR_FRAG_NAME);

	tz_mat4_perspective(&_gi.P, 90, h/(float)w, 0.1, 1000);

	tz_mesh_load_ply(&_gi.class, CLASSROM);
	tz_mesh_gen_nor(&_gi.class);
	tz_gpu_obj_load_mesh(&_gi.class_obj, &_gi.class);

	tz_mesh_load_ply(&_gi.eyebot, EYEBOT);
	tz_mesh_gen_nor(&_gi.eyebot);
	tz_gpu_obj_load_mesh(&_gi.eyebot_obj, &_gi.eyebot);

	// setup camera default position
	_gi.cam.rho   = 2;
	_gi.cam.phi   =-M_PI/4;
	_gi.cam.theta = M_PI/4;

	// setup a redraw callback
	Uint32 delay = (33 / 10) * 10;
	SDL_AddTimer(delay, tick, 0);
}

static void redraw(void) {
	if (_use_deferred_shading) {
		start_geometry_pass();
		do_geometry_pass();
		end_geometry_pass();

		start_shading_pass();
		do_shading_pass();
		end_shading_pass();
	} else {
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		tz_shader_bind(&_gi.regular);
		do_geometry_pass();
		end_geometry_pass();
	}
	tz_window_flip(&_gi.w);
}

int main(int argc, const char *argv[])
{
	init(800, 800);

	SDL_Event e;
	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_KEYUP: {
			if (e.key.keysym.sym == SDLK_q) {
				goto finalize_program;
			}
			if (e.key.keysym.sym == SDLK_v) { _use_deferred_shading = !_use_deferred_shading; }
			if (e.key.keysym.sym == SDLK_r) { build_program(&_gi.shading, SHADING_VERT_NAME, SHADING_FRAG_NAME); }
			break;
		}
		case SDL_MOUSEMOTION: {
			int x, y;
			if (SDL_GetMouseState(&x, &y)) {
				_gi.cam.theta += M_PI*e.motion.xrel/800.0;
				_gi.cam.phi   += M_PI*e.motion.yrel/800.0;
			}
			break;
		}
		case SDL_MOUSEWHEEL: {
			float inc = SDL_GetModState() ? 0.1 : 1;
			_gi.cam.rho += inc*e.wheel.y;
			printf("%f\n", _gi.cam.rho);
			printf("%d\n", e.wheel.y);
			break;
		}
		case SDL_USEREVENT: {
			redraw();
		}}
	}

finalize_program:
	SDL_Quit();
	return 0;
}

static void setup_deferred_shading(uint32_t w, uint32_t h) {

	// TODO: disable for shading pass
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);
	//glDisable(GL_BLEND);

	/* create a framebuffer to attach output textures.
	 * create the textures and bind them to the framebuffer object
	 *
	 * textures outputs:
	 * 0) pos
	 * 1) nor
	 */

	// depth
	glGenTextures(1, &_gi.depth);
	glBindTexture(GL_TEXTURE_2D, _gi.depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	// pos
	glGenTextures(1, &_gi.tex[0]);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);

	// nor
	glGenTextures(1, &_gi.tex[1]);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, NULL);


	glGenFramebuffers(1, &_gi.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _gi.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, _gi.depth,  0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _gi.tex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _gi.tex[1], 0);

	// setup G-buffers
	GLenum fbos[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		//GL_COLOR_ATTACHMENT2,
		//GL_COLOR_ATTACHMENT3,
		//GL_COLOR_ATTACHMENT4,
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
}

/* collect geometry into the "this frame's" memory.
 * split into two buckets: transparent and non transparent.
 * (transparent pass is after shading) */
static void start_geometry_pass()
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, _gi.fbo);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	tz_shader_bind(&_gi.geometry);
}

/* fill G-objects (getting scene ready for shading pass) */
static void do_geometry_pass()
{
	tz_mat4 normal_matrix, view_matrix, modelview_matrix, mvp_matrix;

	tz_cam_mkview(&_gi.cam, &view_matrix);

	modelview_matrix = view_matrix;
	tz_mat4_mul(&mvp_matrix, &_gi.P, &modelview_matrix);

	glUniformMatrix4fv(0, 1, GL_TRUE,  mvp_matrix.f);

	tz_mat4_inverse(&normal_matrix, &modelview_matrix);
	tz_mat4_transpose(&normal_matrix, &normal_matrix);
	glUniformMatrix4fv(1, 1, GL_TRUE,  normal_matrix.f);

	tz_gpu_obj_draw(&_gi.class_obj);
	tz_gpu_obj_draw(&_gi.eyebot_obj);
	//for p in programs
	//	for t in textures
	//		for m in material
	//			for u in unifomrs
}

static void end_geometry_pass()
{
}

static void start_shading_pass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(_gi.vao);
	glActiveTexture(GL_TEXTURE0+0);
	glBindTexture(GL_TEXTURE_2D, _gi.depth);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[1]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _gi.tex[2]);
}

static void do_shading_pass()
{
	tz_shader_bind(&_gi.shading);
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
	return(interval);
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

