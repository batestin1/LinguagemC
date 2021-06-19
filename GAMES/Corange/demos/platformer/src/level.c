#include <math.h>

#include "level.h"

const int MAX_WIDTH = 512;
const int MAX_HEIGHT = 512;

/* These vast case statements are basically a nasty way of assigning properties to the tile types */

static texture* tile_get_texture(int tiletype) {
  texture* t;
  switch(tiletype) {
    case tiletype_none: t = asset_get(P("./tiles/tile_sky.dds")); break;
    case tiletype_air: t = asset_get(P("./tiles/tile_sky.dds")); break;
    case tiletype_dirt: t = asset_get(P("./tiles/tile_dirt.dds")); break;
    case tiletype_dirt_rock: t = asset_get(P("./tiles/tile_dirt_rock.dds")); break;
    case tiletype_dirt_overhang: t = asset_get(P("./tiles/tile_dirt_overhang.dds")); break;
    case tiletype_surface: t = asset_get(P("./tiles/tile_surface.dds")); break;
    case tiletype_grass: t = asset_get(P("./tiles/tile_grass.dds")); break;
    case tiletype_grass_rock1: t = asset_get(P("./tiles/tile_grass_rock1.dds")); break;
    case tiletype_grass_rock2: t = asset_get(P("./tiles/tile_grass_rock2.dds")); break;
    case tiletype_grass_tree: t = asset_get(P("./tiles/tile_grass_tree.dds")); break;
    case tiletype_tree: t = asset_get(P("./tiles/tile_tree.dds")); break;
    case tiletype_tree_top: t = asset_get(P("./tiles/tile_tree_top.dds")); break;
    case tiletype_tree_top_left: t = asset_get(P("./tiles/tile_tree_top_left.dds")); break;
    case tiletype_tree_top_right: t = asset_get(P("./tiles/tile_tree_top_right.dds")); break;
    case tiletype_tree_topest: t = asset_get(P("./tiles/tile_tree_topest.dds")); break;
    case tiletype_tree_bot_left: t = asset_get(P("./tiles/tile_tree_bot_left.dds")); break;
    case tiletype_tree_bot_right: t = asset_get(P("./tiles/tile_tree_bot_right.dds")); break;
    case tiletype_tree_junc_left: t = asset_get(P("./tiles/tile_tree_junc_left.dds")); break;
    case tiletype_tree_junc_right: t = asset_get(P("./tiles/tile_tree_junc_right.dds")); break;
    case tiletype_tree_turn_left: t = asset_get(P("./tiles/tile_tree_turn_left.dds")); break;
    case tiletype_tree_turn_right: t = asset_get(P("./tiles/tile_tree_turn_right.dds")); break;
    case tiletype_tree_side: t = asset_get(P("./tiles/tile_tree_side.dds")); break;
    case tiletype_house_bot_left: t = asset_get(P("./tiles/tile_house_bot_left.dds")); break;
    case tiletype_house_bot_right: t = asset_get(P("./tiles/tile_house_bot_right.dds")); break;
    case tiletype_house_top_left: t = asset_get(P("./tiles/tile_house_top_left.dds")); break;
    case tiletype_house_top_right: t = asset_get(P("./tiles/tile_house_top_right.dds")); break;
    default: t = NULL;
  }
  return t;
}

bool tile_has_collision(int tiletype) {
  
  switch(tiletype) {
    case tiletype_dirt: return true;
    case tiletype_dirt_rock: return true;
    case tiletype_dirt_overhang: return true;
    case tiletype_surface: return true;
    case tiletype_grass_rock1: return true;
    case tiletype_house_bot_left: return true;
    case tiletype_house_bot_right: return true;
    case tiletype_house_top_left: return true;
    case tiletype_house_top_right: return true;
  }
  
  return false;
}

/* Levels are basically stored in an ascii file, with these being the tile type characters. */

static int tile_for_char(char c) {

  switch(c) {
    case '\r': return tiletype_none; 
    case '\n': return tiletype_none; 
    case ' ': return tiletype_none;
    case '`': return tiletype_air;
    case '#': return tiletype_dirt;
    case 'R': return tiletype_dirt_rock;
    case '"': return tiletype_dirt_overhang;
    case '~': return tiletype_surface;
    case '_': return tiletype_grass;
    case '@': return tiletype_grass_rock1;
    case '.': return tiletype_grass_rock2;
    case '!': return tiletype_grass_tree;
    case '|': return tiletype_tree;
    case '\'': return tiletype_tree_top;
    case '{': return tiletype_tree_top_left;
    case '}': return tiletype_tree_top_right;
    case '^': return tiletype_tree_topest;
    case '(': return tiletype_tree_bot_left;
    case ')': return tiletype_tree_bot_right;
    case '+': return tiletype_tree_junc_right;
    case '*': return tiletype_tree_junc_left;
    case '/': return tiletype_tree_turn_right;
    case '\\': return tiletype_tree_turn_left;
    case '-': return tiletype_tree_side;
    case 'h': return tiletype_house_bot_left;
    case 'u': return tiletype_house_bot_right;
    case 'd': return tiletype_house_top_left;
    case 'b': return tiletype_house_top_right;
  }

  warning("Unknown tile type character: '%c'", c);
  return tiletype_none;
}

static int tile_counts[num_tile_types];

/* This just runs through the file and fills some vertex buffers with tile properties */

static int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDL_RWread(file, &c, 1, 1);
    
    if (status == -1) return -1;
    if (i == buffersize-1) return -1;
    if (status == 0) break;
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    buffer[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

level* level_load_file(char* filename) {
  
  for(int i = 0; i < num_tile_types; i++) {
    tile_counts[i] = 0;
  }
  
  level* l = malloc(sizeof(level));
  l->num_tile_sets = num_tile_types;
  l->tile_sets = malloc(sizeof(tile_set) * num_tile_types);
  l->tile_map = calloc(sizeof(int), MAX_WIDTH * MAX_HEIGHT);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  char line[MAX_WIDTH];
  
  int y = 0;
  int x = 0;
  while(SDL_RWreadline(file, line, 1024)) {
    
    for(x = 0; x < strlen(line); x++) {
      char c = line[x];
      int type = tile_for_char(c);
      
      l->tile_map[x + y * MAX_WIDTH] = type;
      tile_counts[type]++;
    }
    
    y++;
  }
  
  SDL_RWclose(file);
  
  /* Start from 1, type 0 is none! */
  for(int i = 1; i < num_tile_types; i++) {
    
    int num_tiles = tile_counts[i];
    
    float* position_data = malloc(sizeof(float) * 3 * 4 * num_tiles);
    float* uv_data = malloc(sizeof(float) * 2 * 4 * num_tiles);
    
    int pos_i  = 0;
    int uv_i = 0;
    
    for(x = 0; x < MAX_WIDTH; x++)
    for(y = 0; y < MAX_HEIGHT; y++) {
      int type = l->tile_map[x + y * MAX_WIDTH];
      if( type == i ) {
        
        position_data[pos_i] = x * TILE_SIZE; pos_i++;
        position_data[pos_i] = y * TILE_SIZE; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = (x+1) * TILE_SIZE; pos_i++;
        position_data[pos_i] = y * TILE_SIZE; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = (x+1) * TILE_SIZE; pos_i++;
        position_data[pos_i] = (y+1) * TILE_SIZE; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = x * TILE_SIZE; pos_i++;
        position_data[pos_i] = (y+1) * TILE_SIZE; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        uv_data[uv_i] = 0; uv_i++;
        uv_data[uv_i] = 0; uv_i++;
        
        uv_data[uv_i] = 1; uv_i++;
        uv_data[uv_i] = 0; uv_i++;
        
        uv_data[uv_i] = 1; uv_i++;
        uv_data[uv_i] = 1; uv_i++;
        
        uv_data[uv_i] = 0; uv_i++;
        uv_data[uv_i] = 1; uv_i++;
        
      }
    }
    
    l->tile_sets[i].num_tiles = num_tiles;
    
    glGenBuffers(1, &l->tile_sets[i].positions_buffer);
    glGenBuffers(1, &l->tile_sets[i].texcoords_buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].positions_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4 * num_tiles, position_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].texcoords_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4 * num_tiles, uv_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(position_data);
    free(uv_data);
  
  }
    
  return l;
}

void level_delete(level* l) {
  
  /* Start from 1 as 0 is none tile set */
  for(int i = 1; i < l->num_tile_sets; i++) {
    glDeleteBuffers(1 , &l->tile_sets[i].positions_buffer);
    glDeleteBuffers(1 , &l->tile_sets[i].texcoords_buffer);
  }
  
  free(l->tile_map);
  free(l->tile_sets);
  free(l);
  
}


/* Just renders a full screen quad with background texture */

void level_render_background(level* l) {
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, graphics_viewport_width(), 0, graphics_viewport_height(), -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_TEXTURE_2D);
  
  texture* background = asset_get_load(P("./backgrounds/bluesky.dds"));
  glBindTexture(GL_TEXTURE_2D, texture_handle(background));
  
  glBegin(GL_QUADS);
    
    glVertex3f(0, graphics_viewport_height(), 0.0);
    glTexCoord2f(1, 0);
    glVertex3f(graphics_viewport_width(), graphics_viewport_height(), 0.0);
    glTexCoord2f(1, 1);
    glVertex3f(graphics_viewport_width(), 0, 0.0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 0, 0.0);
    glTexCoord2f(0, 0);
    
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}

/* Renders each tileset in one go. Uses vertex buffers. */

void level_render_tiles(level* l, vec2 camera_position) {
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(camera_position.x - graphics_viewport_width() / 2, 
          camera_position.x + graphics_viewport_width() / 2,
          -camera_position.y + graphics_viewport_height() / 2,
          -camera_position.y - graphics_viewport_height() / 2
          , -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_TEXTURE_2D);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  /* Start from 1, 0 is no tiles! */
  
  for(int i = 1; i < l->num_tile_sets; i++) {
    
    texture* tile_tex = tile_get_texture(i);
    glBindTexture(GL_TEXTURE_2D, texture_handle(tile_tex));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].positions_buffer);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].texcoords_buffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    
    glDrawArrays(GL_QUADS, 0, l->tile_sets[i].num_tiles * 4);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    glDisableClientState(GL_VERTEX_ARRAY);
    
  }
  
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);  
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}

int level_tile_at(level* l, vec2 position) {
  
  int x = floor( position.x / TILE_SIZE );
  int y = floor( position.y / TILE_SIZE );
  
  assert(x >= 0);
  assert(y >= 0);
  assert(x < MAX_WIDTH);
  assert(y < MAX_HEIGHT);
  
  return l->tile_map[x + y * MAX_WIDTH];
}

vec2 level_tile_position(level* l, int x, int y) {
  
  return vec2_new(x * TILE_SIZE, y * TILE_SIZE);
  
}
