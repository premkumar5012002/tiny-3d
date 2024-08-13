#pragma once

#include "triangle.h"
#include "vector.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2)  // 6 cube faces, 2 triangle per face

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

typedef struct {
  face_t *faces;     // dynamic array of faces
  vec3_t *vertices;  // dynamic array of vertices
  vec3_t rotation;   // rotation of x, y & z axis
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);
void load_obj_file_data(char *filename);