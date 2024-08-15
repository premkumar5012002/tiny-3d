#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "array.h"
#include "display.h"
#include "mesh.h"
#include "vector.h"

triangle_t *triangle_to_render = NULL;

vec3_t camera_position = {0, 0, 0};

float fov_factor = 640;

bool is_paused = false;
bool is_running = false;

enum CULL_METHOD cull_method = CULL_BACKFACE;
enum RENDER_METHOD render_method = RENDER_WIRE;

int previous_time_frame = 0;

bool setup(void) {
  // Allocate the required memory in bytes to hold the color buffer.
  color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

  if (color_buffer == NULL) {
    fprintf(stderr, "Error allocating memory to color_buffer. \n");
    return false;
  }

  // Creating a SDL texture that is used to display the color
  color_buffer_texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                        window_width, window_height);

  load_obj_file_data("../assets/cube.obj");

  return true;
}

void process_input(void) {
  SDL_Event event;

  // Store the current event in the event variable to process system events
  // (e.g., window close, resize) and keyboard input (e.g., key presses,
  // releases).
  SDL_PollEvent(&event);

  switch (event.type) {
    // Triggered when the user attempts to close the application, such as
    // clicking the window's close button or pressing Alt+F4.
    case SDL_QUIT:
      is_running = false;
      break;

    // Triggered when key is pressed on the keyboard.
    case SDL_KEYDOWN:
      // SDLK stands for SDL Keycode
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

      if (event.key.keysym.sym == SDLK_c) {
        cull_method = CULL_BACKFACE;
      }

      if (event.key.keysym.sym == SDLK_d) {
        cull_method = CULL_NONE;
      }

      break;
  }
}

vec2_t project(vec3_t point) {
  vec2_t projected_point = {
      .x = (fov_factor * point.x) / point.z,
      .y = (fov_factor * point.y) / point.z,
  };
  return projected_point;
}

void update(void) {
  // Wait some time until the reach the target frame time in milliseconds
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks64() - previous_time_frame);

  // Only delay execution if we are running too fast
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
    SDL_Delay(time_to_wait);
  }

  if (is_paused == false) {
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
  }

  previous_time_frame = SDL_GetTicks64();

  // Initialize the array of triangles to render
  triangle_to_render = NULL;

  // Loop all triangle faces of our mesh
  int num_faces = array_length(mesh.faces);

  for (int i = 0; i < num_faces; i++) {
    face_t mesh_face = mesh.faces[i];

    // Due to index of array starts with 0, descreasing mesh_face index with -1
    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    vec3_t transformed_vertices[3];

    // Loop all three vertices of this current face and apply transformations
    for (int j = 0; j < 3; j++) {
      vec3_t transformed_vertex = face_vertices[j];

      vec3_rotate_x(&transformed_vertex, mesh.rotation.x);
      vec3_rotate_y(&transformed_vertex, mesh.rotation.y);
      vec3_rotate_z(&transformed_vertex, mesh.rotation.z);

      // Translate the vertices away from the camera
      transformed_vertex.z += 5;

      // Save transformed vertex in the array of transformed vertices
      transformed_vertices[j] = transformed_vertex;
    }

    if (cull_method == CULL_BACKFACE) {
      // Check backface culling
      vec3_t vector_a = transformed_vertices[0];
      vec3_t vector_b = transformed_vertices[1];
      vec3_t vector_c = transformed_vertices[2];

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

      // Calculate how aligned the camera ray is with the face normal (using dot
      // product)
      float dot_normal_camera = vec3_dot(normal, camera_ray);

      // Bypass the triangle that are looking away from camera
      if (dot_normal_camera < 0) {
        continue;
      }
    }

    triangle_t projected_triangle;

    // Loop all three vertices transformed vertices and 2D projection
    for (int j = 0; j < 3; j++) {
      // Project the current vertex
      vec2_t projected_vertex = project(transformed_vertices[j]);

      // Translate the vertex to the middle of the screen
      projected_vertex.x += (window_width / 2);
      projected_vertex.y += (window_height / 2);

      projected_triangle.points[j] = projected_vertex;
    }

    // Save the projected triangle in the array of triangle to render
    array_push(triangle_to_render, projected_triangle);
  }
}

void render(void) {
  draw_dots();

  // Loop all projected triangle and render then
  int num_triangle = array_length(triangle_to_render);
  for (int i = 0; i < num_triangle; i++) {
    triangle_t triangle = triangle_to_render[i];

    if (render_method == RENDER_FILL_TRIANGLE ||
        render_method == RENDER_FILL_TRIANGLE_WIRE) {
      draw_filled_triangle(triangle.points[0].x, triangle.points[0].y,
                           triangle.points[1].x, triangle.points[1].y,
                           triangle.points[2].x, triangle.points[2].y, 0xFF555555);
    }

    if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX ||
        render_method == RENDER_FILL_TRIANGLE_WIRE) {
      draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x,
                    triangle.points[1].y, triangle.points[2].x, triangle.points[2].y,
                    0xFFFFFFFF);
    }

    if (render_method == RENDER_WIRE_VERTEX) {
      draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
      draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
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
