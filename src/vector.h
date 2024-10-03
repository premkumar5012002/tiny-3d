#pragma once

typedef struct {
  float x, y;
} vec2_t;

typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y, z, w;
} vec4_t;

// Vector 2D
vec2_t vec2_add(vec2_t v1, vec2_t v2);
vec2_t vec2_sub(vec2_t v1, vec2_t v2);
vec2_t vec2_mul(vec2_t v, float factor);
vec2_t vec2_div(vec2_t v, float factor);

float vec2_length(vec2_t* v);

void vec2_normalize(vec2_t* v);
float vec2_dot(vec2_t v1, vec2_t v2);


// Vector 3D
vec3_t vec3_new(float x, float y, float z);
vec3_t vec3_clone(vec3_t* v);

vec3_t vec3_add(vec3_t v1, vec3_t v2);
vec3_t vec3_sub(vec3_t v1, vec3_t v2);
vec3_t vec3_mul(vec3_t v, float factor);
vec3_t vec3_div(vec3_t v, float factor);

float vec3_dot(vec3_t v1, vec3_t v2);
vec3_t vec3_cross(vec3_t v1, vec3_t v2);

float vec3_length(vec3_t* v);
void vec3_normalize(vec3_t* v);

void vec3_rotate_x(vec3_t* v, float angle);
void vec3_rotate_y(vec3_t* v, float angle);
void vec3_rotate_z(vec3_t* v, float angle);

// Convertion
vec2_t vec2_from_vec4(vec4_t v);
vec3_t vec3_from_vec4(vec4_t v);
vec4_t vec4_from_vec3(vec3_t v);