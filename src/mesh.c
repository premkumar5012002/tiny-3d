#include <stdio.h>
#include <string.h>

#include "mesh.h"
#include "array.h"
#include "triangle.h"

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

int get_num_meshes(void) {
  return mesh_count;
}

mesh_t* get_mesh(int i) {
  return &meshes[i];
}

void load_mesh(
  char* obj_filepath, char* png_filepath,
  vec3_t scale, vec3_t translation, vec3_t rotation
) {
  load_mesh_obj_data(obj_filepath, &meshes[mesh_count]);
  load_mesh_png_data(png_filepath, &meshes[mesh_count]);
  meshes[mesh_count].scale = scale;
  meshes[mesh_count].rotation = rotation;
  meshes[mesh_count].translation = translation;
  mesh_count++;
}

void load_mesh_obj_data(char *obj_filepath, mesh_t* mesh) {
  FILE *file;
  file = fopen(obj_filepath, "r");

  char line[1024];

  tex2_t* texcoords = NULL;

  while (fgets(line, 512, file)) {

    // Vertex information
    if (strncmp(line, "v ", 2) == 0) {
      vec3_t vertex;
      sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
      array_push(mesh->vertices, vertex);
    }

    // Texture coordinate information
    if (strncmp(line, "vt ", 3) == 0) {
      tex2_t texcoord;
      sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v);
      array_push(texcoords, texcoord);
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
        .a = vertex_indices[0] - 1,
        .b = vertex_indices[1] - 1,
        .c = vertex_indices[2] - 1,
        .a_uv = texcoords[texture_indices[0] - 1],
        .b_uv = texcoords[texture_indices[1] - 1],
        .c_uv = texcoords[texture_indices[2] - 1],        
        .color = 0xFFFFFFFF
      };

      array_push(mesh->faces, face);
    }
  }

  array_free(texcoords);
  fclose(file);
}

void load_mesh_png_data(char *png_filepath, mesh_t* mesh) {
  upng_t* png_image = upng_new_from_file(png_filepath);
  if (png_image != NULL) {
    upng_decode(png_image);
    if (upng_get_error(png_image) == UPNG_EOK) {
      mesh->texture = png_image;
    }
  }
}

void free_meshes(void) {
  for (int i = 0; i < mesh_count; i++) {
    upng_free(meshes[i].texture);
    array_free(meshes[i].faces);
    array_free(meshes[mesh_count].vertices);
  }
}