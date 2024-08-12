#pragma once

typedef struct
{
    float x, y;
} vec2_t;

typedef struct
{
    float x, y, z;
} vec3_t;

vec2_t vec2_add(vec2_t v1, vec2_t v2);
vec2_t vec2_sub(vec2_t v1, vec2_t v2);
vec2_t vec2_mul(vec2_t v, float factor);
float vec2_length(vec2_t v);

vec3_t vec3_add(vec3_t v1, vec3_t v2);
vec3_t vec3_sub(vec3_t v1, vec3_t v2);
vec3_t vec3_mul(vec3_t v, float factor);
float vec3_length(vec3_t v);
vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);