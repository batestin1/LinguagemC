#ifndef level_h
#define level_h

#include "corange.h"

typedef struct {
  
  int num_tiles;
  GLuint positions_buffer;
  GLuint texcoords_buffer;
  asset_hndl mat;
  
} tile_set;

typedef struct {

  int num_tile_sets;
  int* tile_map;
  tile_set* tile_sets;

} level;

#define tiletype_none 0
#define tiletype_air 1

#define tiletype_dirt 2
#define tiletype_dirt_rock 3
#define tiletype_dirt_overhang 4

#define tiletype_surface 5
#define tiletype_grass 6
#define tiletype_grass_rock1 7
#define tiletype_grass_rock2 8

#define tiletype_grass_tree 9
#define tiletype_tree 10
#define tiletype_tree_top 11
#define tiletype_tree_top_left 12
#define tiletype_tree_top_right 13
#define tiletype_tree_topest 14
#define tiletype_tree_bot_left 15
#define tiletype_tree_bot_right 16
#define tiletype_tree_junc_left 17
#define tiletype_tree_junc_right 18
#define tiletype_tree_turn_left 19
#define tiletype_tree_turn_right 20
#define tiletype_tree_side 21

#define tiletype_house_top_left 22
#define tiletype_house_top_right 23
#define tiletype_house_bot_left 24
#define tiletype_house_bot_right 25

#define num_tile_types 26

#define TILE_SIZE 32

level* level_load_file(char* filename);
void level_delete(level* l);

void level_render_background(level* l);
void level_render_tiles(level* l, vec2 camera_position);

int level_tile_at(level* l, vec2 position);
vec2 level_tile_position(level* l, int x, int y);

bool tile_has_collision(int tiletype);

#endif
