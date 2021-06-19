#include "corange.h"

#include <math.h>

static int mouse_x;
static int mouse_y;
static int mouse_down;
static int mouse_right_down;

static renderer* g_dr;

static ellipsoid test_ellipsoid;
static vec3 test_velocity;
static vec3 test_point;
static cmesh* test_cmesh;
static mat4 test_cmesh_world;
static mat3 test_cmesh_world_normal;

void sea_init() {
  
  graphics_viewport_set_title("Sea");
  graphics_viewport_set_size(1280, 720);
  
  folder_load_recursive(P("./assets"));
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(20, 20, 10);
  cam->target =  vec3_new(0, 0, 0);
  
  asset_hndl opt_graphics = asset_hndl_new_load(P("./assets/graphics.cfg"));
  
  g_dr = renderer_new(opt_graphics);
  renderer_set_camera(g_dr, cam);
  renderer_set_tod(g_dr, 0.15, 0);
  renderer_set_sea_enabled(g_dr, true);
  
  folder_load(P("./assets/corvette/"));
  
  renderable* r_corvette = asset_get(P("./assets/corvette/corvette.bmf"));
  r_corvette->material = asset_hndl_new_load(P("./assets/corvette/corvette.mat"));
  
  static_object* s_corvette = entity_new("corvette", static_object);
  s_corvette->renderable = asset_hndl_new_ptr(r_corvette);
  s_corvette->rotation = quat_angle_axis(1.0, vec3_new(0.1, 1.0, 0.31));
  s_corvette->scale = vec3_new(2.0, 2.0, 2.0);
  
  ui_button* framerate = ui_elem_new("framerate", ui_button);
  ui_button_move(framerate, vec2_new(10,10));
  ui_button_resize(framerate, vec2_new(30,25));
  ui_button_set_label(framerate, "");
  ui_button_disable(framerate);
  
  test_ellipsoid = ellipsoid_new(vec3_new(0, 20, 0), vec3_new(1,2,1));
  test_velocity = vec3_zero();
  test_cmesh = asset_get(P("./assets/corvette/corvette.col"));
  test_cmesh_world = static_object_world(s_corvette);
  test_cmesh_world_normal = static_object_world_normal(s_corvette);
  
  test_point = vec3_new(5, 5, 5);

  s_corvette->collision_body = asset_hndl_new(P("./assets/corvette/corvette.col"));
  
}

static float wave_time = 0.0f;

void sea_update() {

  camera* cam = entity_get("camera");
  
  wave_time += frame_time();
  static_object* corvette = entity_get("corvette");
  //corvette->position.y = (sin(wave_time) + 1) / 2 - 1;
  //corvette->rotation = quat_rotation_x(sin(wave_time * 1.123) / 50);
  //corvette->rotation = quat_mul_quat(corvette->rotation, quat_rotation_y(sin(wave_time * 1.254) / 25));
  //corvette->rotation = quat_mul_quat(corvette->rotation, quat_rotation_z(sin(wave_time * 1.355) / 100));
  
  ui_button* framerate = ui_elem_get("framerate");
  ui_button_set_label(framerate, frame_rate_string());
  
  const Uint8* keystate = SDL_GetKeyboardState(NULL);
  
  vec3 right  = vec3_normalize(vec3_cross(camera_direction(cam), vec3_up()));
  vec3 left = vec3_neg(right);
  vec3 front = vec3_cross(left, vec3_up());
  vec3 back  = vec3_neg(front);
  vec3 movement = vec3_zero();
  
  if (keystate[SDL_SCANCODE_W]) {
    movement = vec3_add(movement, front);
  }
  if (keystate[SDL_SCANCODE_A]) {
    movement = vec3_add(movement, left);
  }
  if (keystate[SDL_SCANCODE_S]) {
    movement = vec3_add(movement, back);
  }
  if (keystate[SDL_SCANCODE_D]) {
    movement = vec3_add(movement, right);
  }
  
  /*
  if (keystate[SDLK_UP]) {
    test_velocity = vec3_add(test_velocity, vec3_mul(vec3_up(), frame_time()));
  }
  if (keystate[SDLK_DOWN]) {
    test_velocity = vec3_add(test_velocity, vec3_mul(vec3_neg(vec3_up()), frame_time()));
  }
  */
  
  float top_speed = 0.25;
  
  vec3 gravity_update  = vec3_add(test_velocity, vec3_mul(vec3_gravity(), frame_time()));
  vec3 movement_update = vec3_lerp(test_velocity, vec3_mul(movement, top_speed), saturate(5 * frame_time()));
  
  collision col = ellipsoid_collide_mesh(test_ellipsoid, vec3_new(0, -1, 0), test_cmesh, test_cmesh_world, test_cmesh_world_normal);
  float closeness = col.collided ? clamp((1 - col.time), 0.01, 0.99) : 0.01;
  
  test_velocity = vec3_lerp(gravity_update, movement_update, closeness);
  
  /* Collision Detection and response routine */
  
  collision collision_test_ellipsoid(void* x, vec3* position, vec3* velocity) {
    return ellipsoid_collide_mesh(test_ellipsoid, *velocity, test_cmesh, test_cmesh_world, test_cmesh_world_normal);
  }
  
  renderer_add(g_dr, render_object_ellipsoid(test_ellipsoid));
  renderer_add(g_dr, render_object_static(entity_get("corvette")));
  
  collision_response_slide(g_dr, &test_ellipsoid.center, &test_velocity, collision_test_ellipsoid);
  
  /* End */
  
}

void sea_render() {
  
  //renderer_add(g_dr, render_object_cmesh(test_cmesh, test_cmesh_trans));
  //renderer_add(g_dr, render_object_ellipsoid(test_ellipsoid));
  //renderer_add(g_dr, render_object_static(entity_get("corvette")));
  renderer_render(g_dr);
  
}

static int ball_count = 0;
void sea_event(SDL_Event e) {

  camera* c = entity_get("camera");
  
  //camera_control_orbit(c, e);
  
  float a1 = 0;
  float a2 = 0;
  vec3 axis = vec3_zero();
  
  vec3 translation = c->target;
  vec3 position = vec3_sub(c->position, translation);
  vec3 target = vec3_sub(c->target, translation);
  
  switch(e.type) {
    
    case SDL_MOUSEMOTION:
      if (e.motion.state & SDL_BUTTON(1)) {
        a1 = e.motion.xrel * -0.005;
        a2 = e.motion.yrel *  0.005;
        position = mat3_mul_vec3(mat3_rotation_y(a1), position);
        axis = vec3_normalize(vec3_cross( vec3_sub(position, target) , vec3_up() ));
        position = mat3_mul_vec3(mat3_rotation_angle_axis(a2, axis), position );
      }
    break;
    
    case SDL_MOUSEWHEEL:
      position = vec3_add(position, vec3_mul(vec3_normalize(position), -e.wheel.y));
    break;

  }
  
  position = vec3_add(position, translation);
  target   = vec3_add(target,   translation);
  
  c->target = target;
  
  vec3 velocity = vec3_sub(position, c->position);
  
  collision collision_camera(void* x, vec3* position, vec3* velocity) {
    return sphere_collide_mesh(sphere_new(*position, 0.25), *velocity, test_cmesh, test_cmesh_world, test_cmesh_world_normal);
  }
  
  collision_response_slide(g_dr, &c->position, &velocity, collision_camera);
  
}

void sea_finish() {
  renderer_delete(g_dr);
}

int main(int argc, char **argv) {
  
  #ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
  #endif
  
  corange_init("../../assets_core");
  
  sea_init();
  
  int running = 1;
  SDL_Event e;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&e)) {
      switch(e.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (e.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (e.key.keysym.sym == SDLK_PRINTSCREEN) { graphics_viewport_screenshot(); }
        if (e.key.keysym.sym == SDLK_r &&
            e.key.keysym.mod == KMOD_LCTRL) {
            asset_reload_all();
        }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      sea_event(e);
      ui_event(e);
    }
    
    sea_update();
    ui_update();
    
    sea_render();
    ui_render();
    
    graphics_swap();
    
    frame_end();
  }
  
  sea_finish();
  corange_finish();
  
  return 0;
  
}
