#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "array.h"
#include "display.h"
#include "mesh.h"
#include "matrix.h"
#include "vector.h"
#include "texture.h"
#include "triangle.h"
#include "light.h"

bool is_paused = false;
bool is_running = false;

mat4_t proj_matrix;
vec3_t camera_position = { 0, 0, 0 };

int previous_time_frame = 0;

triangle_t *triangle_to_render = NULL;

enum CULL_METHOD cull_method = CULL_BACKFACE;
enum RENDER_METHOD render_method = RENDER_WIRE_VERTEX;

bool setup(void) {
  // Allocate the required memory in bytes to hold the color buffer.
  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

  if (color_buffer == NULL) {
    fprintf(stderr, "Error allocating memory to color_buffer. \n");
    return false;
  }

  // Creating a SDL texture that is used to display the color
  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    window_width, window_height
  );

  float fov = M_PI / 3; // the same as 180/3, or 60deg 
  float ascept = (float)window_height / (float)window_width;
  float znear = 0.1;
  float zfar = 100.0;
  proj_matrix = mat4_make_perspective(fov, ascept, znear, zfar);

  mesh_texture = (uint32_t*)REDBRICK_TEXTURE;

  // load_cube_mesh_data();

  load_obj_file_data("../assets/f22.obj");

  return true;
}

void process_input(void) {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
    case SDL_QUIT:
      is_running = false;
      break;

    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      }

      if (event.key.keysym.sym == SDLK_SPACE) {
        is_paused = !is_paused;
      }

      if (event.key.keysym.sym == SDLK_1) {
        render_method = RENDER_WIRE_VERTEX;
      }

      if (event.key.keysym.sym == SDLK_2) {
        render_method = RENDER_WIRE;
      }

      if (event.key.keysym.sym == SDLK_3) {
        render_method = RENDER_FILL_TRIANGLE;
      }

      if (event.key.keysym.sym == SDLK_4) {
        render_method = RENDER_FILL_TRIANGLE_WIRE;
      }

      if (event.key.keysym.sym == SDLK_5) {
        render_method = RENDER_TEXTURED;
      }

      if (event.key.keysym.sym == SDLK_6) {
        render_method = RENDERED_TEXTURED_WIRE;
      }

      if (event.key.keysym.sym == SDLK_c) {
        cull_method = CULL_BACKFACE;
      }

      if (event.key.keysym.sym == SDLK_d) {
        cull_method = CULL_NONE;
      }
      
    break;
  }
}

void update(void) {
  // Wait some time until the reach the target frame time in milliseconds
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks64() - previous_time_frame);

  // Only delay execution if we are running too fast
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
    SDL_Delay(time_to_wait);
  }

  previous_time_frame = SDL_GetTicks64();

  // Initialize the array of triangles to render
  triangle_to_render = NULL;

  if (is_paused == false) {
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    mesh.scale.x += 0.00;
    mesh.scale.y += 0.00;
    mesh.scale.z += 0.00;

    mesh.translation.x += 0.00;
    mesh.translation.y += 0.00;
    mesh.translation.z = 5;
  }

  mat4_t scale_matrix = mat4_make_scale(
    mesh.scale.x,
    mesh.scale.y,
    mesh.scale.z
  );

  mat4_t translation_matrix = mat4_make_translation(
    mesh.translation.x,
    mesh.translation.y,
    mesh.translation.z
  );

  mat4_t rotation_x_matrix = mat4_make_rotation_x(mesh.rotation.x);
  mat4_t rotation_y_matrix = mat4_make_rotation_y(mesh.rotation.y);
  mat4_t rotation_z_matrix = mat4_make_rotation_z(mesh.rotation.z);

  // Loop all triangle faces of our mesh
  int num_faces = array_length(mesh.faces);

  for (int i = 0; i < num_faces; i++) {
    face_t mesh_face = mesh.faces[i];

    // Due to index of array starts with 0, descreasing mesh_face index with -1
    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    vec4_t transformed_vertices[3];

    // Loop all three vertices of this current face and apply transformations
    for (int j = 0; j < 3; j++) {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

      // Create a World Matrix combining scale, rotation, and translation matrices
      mat4_t world_matrix = mat4_identity();

      world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);

      world_matrix = mat4_mul_mat4(rotation_x_matrix, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_y_matrix, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_z_matrix, world_matrix);

      world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

      transformed_vertices[j] = mat4_mul_vec4(world_matrix, transformed_vertex);
    }

    // Check backface culling
    vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
    vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
    vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

    // Get the vector subtraction of B-A and C-A
    vec3_t vector_ab = vec3_sub(vector_b, vector_a);
    vec3_normalize(&vector_ab);

    vec3_t vector_ac = vec3_sub(vector_c, vector_a);
    vec3_normalize(&vector_ac);

    // Compute the face normal using cross product to find penpendicular vector
    vec3_t normal = vec3_cross(vector_ab, vector_ac);

    // Normalize the face normal vector
    vec3_normalize(&normal);

    // Find the vector between a point in the triangle and the camera origin
    vec3_t camera_ray = vec3_sub(camera_position, vector_a);

    // Calculate how aligned the camera ray is with the face normal (using dot product)
    float dot_normal_camera = vec3_dot(normal, camera_ray);

    // Bypass the triangle that are looking away from camera
    if (dot_normal_camera < 0 && cull_method == CULL_BACKFACE) {
      continue;
    }
    
    triangle_t projected_triangle;

    // Loop all three vertices transformed vertices and 2D projection
    for (int j = 0; j < 3; j++) {
      // Project the current vertex
      vec4_t projected_vertex = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);
 
      // Scale the vertex into view
      projected_vertex.x *= (window_width / 2.0);
      projected_vertex.y *= (window_height/ 2.0);

      // Invert the y values to account for flipped screen y coordinate
      projected_vertex.y *= -1;

      // Translate the vertex to the middle of the screen
      projected_vertex.x += (window_width / 2.0);
      projected_vertex.y += (window_height / 2.0);

      projected_triangle.points[j].x = projected_vertex.x;
      projected_triangle.points[j].y = projected_vertex.y;
    }

    projected_triangle.tex_coords[0].u = mesh_face.a_uv.u;
    projected_triangle.tex_coords[0].v = mesh_face.a_uv.v;

    projected_triangle.tex_coords[1].u = mesh_face.b_uv.u;
    projected_triangle.tex_coords[1].v = mesh_face.b_uv.v;

    projected_triangle.tex_coords[2].u = mesh_face.c_uv.u;
    projected_triangle.tex_coords[2].v = mesh_face.c_uv.v;

    float light_intensity_factor = -vec3_dot(normal, light.direction);
    uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
    
    projected_triangle.color = triangle_color;

    projected_triangle.avg_depth = (
      transformed_vertices[0].z +
      transformed_vertices[1].z +
      transformed_vertices[2].z
    ) / 3;

    // Save the projected triangle in the array of triangle to render
    array_push(triangle_to_render, projected_triangle);
  }

  // Sort the triangles to render by their avg_depth
  int num_triangles = array_length(triangle_to_render);

  for (int i = 0; i < num_triangles; i++) {
    for (int j = i; j < num_triangles; j++) {
      if (triangle_to_render[i].avg_depth < triangle_to_render[j].avg_depth) {
        // Swap the triangles positions in the array
        triangle_t temp = triangle_to_render[i];
        triangle_to_render[i] = triangle_to_render[j];
        triangle_to_render[j] = temp;
      }
    }
  }  
}

void render(void) {
  draw_dots();

  // Loop all projected triangle and render then
  int num_triangle = array_length(triangle_to_render);

  for (int i = 0; i < num_triangle; i++) {
    triangle_t triangle = triangle_to_render[i];

    if (
      render_method == RENDER_FILL_TRIANGLE ||
      render_method == RENDER_FILL_TRIANGLE_WIRE
    ) {
      draw_filled_triangle(
        triangle.points[0].x, triangle.points[0].y,
        triangle.points[1].x, triangle.points[1].y,
        triangle.points[2].x, triangle.points[2].y,
        triangle.color
      );
    }

    if (
      render_method == RENDER_WIRE ||
      render_method == RENDER_WIRE_VERTEX ||
      render_method == RENDERED_TEXTURED_WIRE ||
      render_method == RENDER_FILL_TRIANGLE_WIRE
    ) {
      draw_triangle(
        triangle.points[0].x, triangle.points[0].y,
        triangle.points[1].x, triangle.points[1].y,
        triangle.points[2].x, triangle.points[2].y,
        triangle.color
      );
    }

    if (render_method == RENDER_TEXTURED || render_method == RENDERED_TEXTURED_WIRE) {
      draw_textured_triangle(
        triangle.points[0].x, triangle.points[0].y, triangle.tex_coords[0].u, triangle.tex_coords[0].v,
        triangle.points[1].x, triangle.points[1].y, triangle.tex_coords[1].u, triangle.tex_coords[1].v, 
        triangle.points[2].x, triangle.points[2].y, triangle.tex_coords[2].u, triangle.tex_coords[2].v,
        mesh_texture
      );
    }

    if (render_method == RENDER_WIRE_VERTEX) {
      draw_rect(
        triangle.points[0].x - 3, triangle.points[0].y - 3,
        6, 6,
        0xFFFF0000
      );
      draw_rect(
        triangle.points[1].x - 3, triangle.points[1].y - 3,
        6, 6,
        0xFFFF0000
      );
      draw_rect(
        triangle.points[2].x - 3, triangle.points[2].y - 3,
        6, 6,
        0xFFFF0000
      );
    }
  }

  array_free(triangle_to_render);

  render_color_buffer();

  clear_color_buffer(0xFF000000);

  SDL_RenderPresent(renderer);
}

void free_resources(void) {
  free(color_buffer);
  array_free(mesh.faces);
  array_free(mesh.vertices);
}

int main(int argc, char *argv[]) {
  is_running = initialize_window();

  setup();

  while (is_running) {
    process_input();
    update();
    render();
  }

  destroy_window();
  free_resources();

  return 0;
}
