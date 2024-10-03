#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "array.h"
#include "display.h"
#include "camera.h"
#include "mesh.h"
#include "clipping.h"
#include "matrix.h"
#include "vector.h"
#include "texture.h"
#include "triangle.h"
#include "light.h"
#include "upng.h"

bool is_paused = true;
bool is_running = false;

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

float delta_time = 0;
int previous_time_frame = 0;

int num_triangles_to_render = 0;

#define MAX_TRIANGLE_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLE_PER_MESH];

enum CULL_METHOD cull_method = CULL_BACKFACE;
enum RENDER_METHOD render_method = RENDER_WIRE_VERTEX;

bool setup(void) {
  // Allocate the required memory in bytes to hold the color buffer and z buffer.
  color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
  z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

  if (color_buffer == NULL) {
    fprintf(stderr, "Error allocating memory to color_buffer. \n");
    return false;
  }

  // Creating a SDL texture that is used to display the color
  color_buffer_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA32,
    SDL_TEXTUREACCESS_STREAMING,
    window_width, window_height
  );

  // Initialize the perspective projection matrix
  float aspect_x = (float)window_width/(float)window_height;
  float aspect_y = (float)window_height/(float)window_width;
  float fov_y = M_PI / 3; // the same as 180/3, or 60deg 
  float fov_x = atan(tan(fov_y) * aspect_x) * 2;
  float znear = 0.1;
  float zfar = 100.0;
  proj_matrix = mat4_make_perspective(fov_y, aspect_y, znear, zfar);

  // Initialize frustum planes with a point and a normal
  init_frustum_planes(fov_x, fov_y, znear, zfar);

  load_obj_file_data("../assets/cube.obj");
  load_png_texture_data("../assets/cube.png");

  return true;
}

void process_input(void) {
  SDL_Event event;

  while(SDL_PollEvent(&event)) {
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

        if (event.key.keysym.sym == SDLK_w) {
          camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
          camera.position = vec3_add(camera.position, camera.forward_velocity);
        }

        if (event.key.keysym.sym == SDLK_s) {
          camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
          camera.position = vec3_sub(camera.position, camera.forward_velocity);
        }

        if (event.key.keysym.sym == SDLK_a) {
          camera.yaw -= 1 * delta_time;
        }

        if (event.key.keysym.sym == SDLK_d) {
          camera.yaw += 1 * delta_time;
        }

        if (event.key.keysym.sym == SDLK_UP) {
          camera.position.y += 3.0 * delta_time;
        }

        if (event.key.keysym.sym == SDLK_DOWN) {
          camera.position.y -= 3.0 * delta_time;
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

        if (event.key.keysym.sym == SDLK_x) {
          cull_method = CULL_NONE;
        }
        
      break;
    }
  }
}

void update(void) {
  // Wait some time until the reach the target frame time in milliseconds
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks64() - previous_time_frame);

  // Only delay execution if we are running too fast
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
    SDL_Delay(time_to_wait);
  }

  // Get a delta time factor converted to seconds to be used to update our game objects
  delta_time = (SDL_GetTicks64() - previous_time_frame) / 1000.0;

  previous_time_frame = SDL_GetTicks64();

  // Reset the total triangles number for next render
  num_triangles_to_render = 0;

  if (is_paused == false) {
    mesh.rotation.x += 0.0 * delta_time;
    mesh.rotation.y += 0.0 * delta_time;
    mesh.rotation.z += 0.0 * delta_time;

    mesh.scale.x += 0.0 * delta_time;
    mesh.scale.y += 0.0 * delta_time;
    mesh.scale.z += 0.0 * delta_time;

    mesh.translation.x += 0.0 * delta_time;
    mesh.translation.y += 0.0 * delta_time;
    mesh.translation.z += 0.0 * delta_time;
  }

  // Initialize the target looking at the positive z-axis
  vec3_t target = { 0, 0, 1 };
  mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
  camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

  // Offset the camera position in the direction where the camera is pointing at 
  target = vec3_add(camera.position, camera.direction);

  vec3_t up_direction = { 0, 1, 0 };

  // Create the view matrix
  view_matrix = mat4_look_at(camera.position, target, up_direction);

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

    vec3_t face_vertices[3];

    face_vertices[0] = mesh.vertices[mesh_face.a];
    face_vertices[1] = mesh.vertices[mesh_face.b];
    face_vertices[2] = mesh.vertices[mesh_face.c];

    vec4_t transformed_vertices[3];

    // Loop all three vertices of this current face and apply transformations
    for (int j = 0; j < 3; j++) {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

      // Create a World Matrix combining scale, rotation, and translation matrices
      world_matrix = mat4_identity();

      world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);

      world_matrix = mat4_mul_mat4(rotation_z_matrix, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_y_matrix, world_matrix);
      world_matrix = mat4_mul_mat4(rotation_x_matrix, world_matrix);

      world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

      // Multiply the world matrix by the original vector
      transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

      // Multiply the view matrix by the vector to transform the scene to camera space
      transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

      // Save transformed vertex in the array of transformed vertices
      transformed_vertices[j] = transformed_vertex;
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
    vec3_t origin = { 0, 0, 0 };
    vec3_t camera_ray = vec3_sub(origin, vector_a);

    // Calculate how aligned the camera ray is with the face normal (using dot product)
    float dot_normal_camera = vec3_dot(normal, camera_ray);

    // Bypass the triangle that are looking away from camera
    if (dot_normal_camera < 0 && cull_method == CULL_BACKFACE) {
      continue;
    }

    // Create a polygon from the original transformed triangle to be clipped
    polygon_t polygon = create_polygon_from_triangle(
      vec3_from_vec4(transformed_vertices[0]),
      vec3_from_vec4(transformed_vertices[1]),
      vec3_from_vec4(transformed_vertices[2]),
      mesh_face.a_uv,
      mesh_face.b_uv,
      mesh_face.c_uv
    );

    // Clip the polygon and returns a new polygon with potential new vertices
    clip_polygon(&polygon);

    // Break the clipped polygon apart back into individual triangles
    triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLE]; 
    int num_triangles_after_clipping = 0;

    triangle_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);
    
    // Loop all the assembled triangles after clipping
    for (int t = 0; t < num_triangles_after_clipping; t++) {
      triangle_t triangle_after_clipping = triangles_after_clipping[t];

      // Projection
      triangle_t triangle_to_render;

      // Loop all three vertices transformed vertices of the triangle and 2D projection
      for (int j = 0; j < 3; j++) {
        // Project the current vertex
        vec4_t projected_vertex = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);
  
        // Scale the vertex into view
        projected_vertex.x *= (window_width / 2.0);
        projected_vertex.y *= (window_height/ 2.0);

        // Invert the y values to account for flipped screen y coordinate
        projected_vertex.y *= -1;

        // Translate the vertex to the middle of the screen
        projected_vertex.x += (window_width / 2.0);
        projected_vertex.y += (window_height / 2.0);

        triangle_to_render.points[j].x = projected_vertex.x;
        triangle_to_render.points[j].y = projected_vertex.y;
        triangle_to_render.points[j].z = projected_vertex.z;
        triangle_to_render.points[j].w = projected_vertex.w;
      }

      triangle_to_render.tex_coords[0].u = triangle_after_clipping.tex_coords[0].u;
      triangle_to_render.tex_coords[0].v = triangle_after_clipping.tex_coords[0].v;

      triangle_to_render.tex_coords[1].u = triangle_after_clipping.tex_coords[1].u;
      triangle_to_render.tex_coords[1].v = triangle_after_clipping.tex_coords[1].v;

      triangle_to_render.tex_coords[2].u = triangle_after_clipping.tex_coords[2].u;
      triangle_to_render.tex_coords[2].v = triangle_after_clipping.tex_coords[2].v;

      // Apply flat shading
      float light_intensity_factor = -vec3_dot(normal, light.direction);
      uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
      triangle_to_render.color = triangle_color;

      // Save the projected triangle in the array of triangle to render
      if (num_triangles_to_render < MAX_TRIANGLE_PER_MESH) {
        triangles_to_render[num_triangles_to_render] = triangle_to_render;
        num_triangles_to_render++;
      }
    }
  } 
}

void render(void) {
  draw_dots();

  // Loop all projected triangle and render then
  for (int i = 0; i < num_triangles_to_render; i++) {
    triangle_t triangle = triangles_to_render[i];

    if (
      render_method == RENDER_FILL_TRIANGLE ||
      render_method == RENDER_FILL_TRIANGLE_WIRE
    ) {
      draw_filled_triangle(
        triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
        triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
        triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
        triangle.color
      );
    }

    if (render_method == RENDER_TEXTURED || render_method == RENDERED_TEXTURED_WIRE) {
      draw_textured_triangle(
        triangle.points[0].x, triangle.points[0].y, triangle.points[0].z,
        triangle.points[0].w, triangle.tex_coords[0].u, triangle.tex_coords[0].v,

        triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, 
        triangle.points[1].w, triangle.tex_coords[1].u, triangle.tex_coords[1].v, 

        triangle.points[2].x, triangle.points[2].y, triangle.points[2].z,
        triangle.points[2].w, triangle.tex_coords[2].u, triangle.tex_coords[2].v,

        mesh_texture
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
        0xFFFF0000
      );
    }

    if (render_method == RENDER_WIRE_VERTEX) {
      draw_rect(
        triangle.points[0].x - 3, triangle.points[0].y - 3,
        6, 6,
        0xFF000000
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

  render_color_buffer();

  clear_z_buffer();

  clear_color_buffer(0xFF000000);

  SDL_RenderPresent(renderer);
}

void free_resources(void) {
  free(z_buffer);
  free(color_buffer);
  upng_free(png_texture);
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
