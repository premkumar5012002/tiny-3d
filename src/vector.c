#include <math.h>

#include "vector.h"

vec2_t vec2_add(vec2_t v1, vec2_t v2) {
  vec2_t result = {
    .x = v1.x + v2.x,
    .y = v1.y + v2.y,
  };
  return result;
}
vec2_t vec2_sub(vec2_t v1, vec2_t v2) {
  vec2_t result = {
    .x = v1.x - v2.x,
    .y = v1.y - v2.y,
  };
  return result;
}

vec2_t vec2_mul(vec2_t v, float factor) {
  vec2_t result = {
    .x = v.x * factor,
    .y = v.y * factor,
  };
  return result;
}

vec2_t vec2_div(vec2_t v, float factor) {
  vec2_t result = {
    .x = v.x / factor,
    .y = v.y / factor,
  };
  return result;
}

float vec2_length(vec2_t *v) {
  return sqrt(pow(v->x, 2) + pow(v->y, 2));
}

void vec2_normalize(vec2_t *v) {
  float length = vec2_length(v);
  v->x /= length;
  v->y /= v->y / length;
}

float vec2_dot(vec2_t v1, vec2_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

vec3_t vec3_add(vec3_t v1, vec3_t v2) {
  vec3_t result = {
    .x = v1.x + v2.x,
    .y = v1.y + v2.y,
    .z = v1.z + v2.z,
  };
  return result;
}

vec3_t vec3_sub(vec3_t v1, vec3_t v2) {
  vec3_t result = {
    .x = v1.x - v2.x,
    .y = v1.y - v2.y,
    .z = v1.z - v2.z,
  };
  return result;
}

vec3_t vec3_mul(vec3_t v, float factor) {
  vec3_t result = {
    .x = v.x * factor,
    .y = v.y * factor,
    .z = v.z * factor,
  };
  return result;
}

vec3_t vec3_div(vec3_t v, float factor) {
  vec3_t result = {
    .x = v.x / factor,
    .y = v.y / factor,
    .z = v.z / factor,
  };
  return result;
}

vec3_t vec3_cross(vec3_t v1, vec3_t v2) {
  vec3_t result = {
    .x = v1.y * v2.z - v1.z * v2.y,
    .y = v1.z * v2.x - v1.x * v2.z,
    .z = v1.x * v2.y - v1.y * v2.x,
  };
  return result;
}

float vec3_dot(vec3_t v1, vec3_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

float vec3_length(vec3_t *v) {
  return sqrt(pow(v->x, 2) + pow(v->y, 2) + pow(v->z, 2));
}

void vec3_normalize(vec3_t *v) {
  float length = vec3_length(v);
  v->x /= length;
  v->y /= length;
  v->z /= length;
}

void vec3_rotate_x(vec3_t *v, float angle) {
  float new_y = v->y * cos(angle) - v->z * sin(angle);
  float new_z = v->y * sin(angle) + v->z * cos(angle);
  v->y = new_y;
  v->z = new_z;
}

void vec3_rotate_y(vec3_t *v, float angle) {
  float new_x = v->x * cos(angle) + v->z * sin(angle);
  float new_z = -v->x * sin(angle) + v->z * cos(angle);
  v->x = new_x;
  v->z = new_z;
}

void vec3_rotate_z(vec3_t *v, float angle) {
  float new_x = v->x * cos(angle) - v->y * sin(angle);
  float new_y = v->x * sin(angle) + v->y * cos(angle);
  v->x = new_x;
  v->y = new_y;
}

vec2_t vec2_from_vec4(vec4_t v) {
  vec2_t result = { v.x, v.y };
  return result;
}

vec3_t vec3_from_vec4(vec4_t v) {
  vec3_t result = {
    .x = v.x,
    .y = v.y,
    .z = v.z,
  };
  return result;
}

vec4_t vec4_from_vec3(vec3_t v) {
  vec4_t result = {
    .x = v.x,
    .y = v.y,
    .z = v.z,
    .w = 1
  };
  return result;
}

