#include "mesh.h"

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "triangle.h"

mesh_t mesh = {
    .faces = NULL,
    .vertices = NULL,
    .scale = { .x = 1.0, .y = 1.0, .z = 1.0 },
    .rotation = {.x = 0, .y = 0, .z = 0},
    .translation = { .x = 0, .y = 0, .z = 0 }
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, {.x = -1, .y = 1, .z = -1},
    {.x = 1, .y = 1, .z = -1},   {.x = 1, .y = -1, .z = -1},
    {.x = 1, .y = 1, .z = 1},    {.x = 1, .y = -1, .z = 1},
    {.x = -1, .y = 1, .z = 1},   {.x = -1, .y = -1, .z = 1},
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    {.a = 1, .b = 2, .c = 3, .color = 0xFFF0F0FF},
    {.a = 1, .b = 3, .c = 4, .color = 0xFFF35FFF},
    // right
    {.a = 4, .b = 3, .c = 5, .color = 0xFFF5FFFF},
    {.a = 4, .b = 5, .c = 6, .color = 0xF78FFFFF},
    // back
    {.a = 6, .b = 5, .c = 7, .color = 0xFF56FFFF},
    {.a = 6, .b = 7, .c = 8, .color = 0xFF78FFFF},
    // left
    {.a = 8, .b = 7, .c = 2, .color = 0xFF89FFFF},
    {.a = 8, .b = 2, .c = 1, .color = 0xFF37FFFF},
    // top
    {.a = 2, .b = 7, .c = 5, .color = 0xFF48FFFF},
    {.a = 2, .b = 5, .c = 3, .color = 0xFFFF89FF},
    // bottom
    {.a = 6, .b = 8, .c = 1, .color = 0xFFFF36FF},
    {.a = 6, .b = 1, .c = 4, .color = 0xFFFF37FF},
};

void load_cube_mesh_data(void) {
  // For cube vertices
  for (int i = 0; i < N_CUBE_VERTICES; i++) {
    vec3_t cube_vertex = cube_vertices[i];
    array_push(mesh.vertices, cube_vertex);
  }

  // For cube faces
  for (int i = 0; i < N_CUBE_FACES; i++) {
    face_t cube_face = cube_faces[i];
    array_push(mesh.faces, cube_face);
  }
}

void load_obj_file_data(char *filename) {
  FILE *file;
  file = fopen(filename, "r");

  char line[512];

  while (fgets(line, 512, file)) {
    // Vertex information
    if (strncmp(line, "v ", 2) == 0) {
      vec3_t vertex;
      sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      array_push(mesh.vertices, vertex);
    }

    // Face information
    if (strncmp(line, "f ", 2) == 0) {
      int vertex_indices[3];
      int texture_indices[3];
      int normal_indices[3];

      sscanf(
        line,
        "f %d/%d/%d %d/%d/%d %d/%d/%d",
        &vertex_indices[0], &texture_indices[0], &normal_indices[0],
        &vertex_indices[1], &texture_indices[1], &normal_indices[1],
        &vertex_indices[2], &texture_indices[2], &normal_indices[2]
      );

      face_t face = {
        .a = vertex_indices[0],
        .b = vertex_indices[1],
        .c = vertex_indices[2]
      };

      array_push(mesh.faces, face);
    }
  }
}