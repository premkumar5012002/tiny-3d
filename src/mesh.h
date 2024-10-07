#pragma once

#include "triangle.h"
#include "vector.h"
#include "upng.h"

typedef struct {
  face_t* faces;        // mesh dynamic array of faces
  vec3_t* vertices;     // mesh dynamic array of vertices
  upng_t* texture;      // mesh PNG texture pointer 
  vec3_t scale;         // mesh scale with x, y, & z axis
  vec3_t rotation;      // mesh rotation of x, y & z axis
  vec3_t translation;   // mesh translation with x, y & z axis
} mesh_t;

void load_mesh(
  char* obj_filepath,
  char* png_filepath,
  vec3_t scale,
  vec3_t translation, 
  vec3_t rotation
);

int get_num_meshes(void);
mesh_t* get_mesh(int i);

void load_mesh_obj_data(char *obj_filepath, mesh_t* mesh);
void load_mesh_png_data(char *png_filepath, mesh_t* mesh);

void free_meshes(void);