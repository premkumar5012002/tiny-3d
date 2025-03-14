#pragma once

#include "vector.h"
#include "triangle.h"

#define MAX_NUM_POLY_VERTICES 10
#define MAX_NUM_POLY_TRIANGLE 10
#define MAX_NUM_TEXCOORDS 10

enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE
};

typedef struct {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec3_t vertices[MAX_NUM_POLY_VERTICES];
    tex2_t texcoords[MAX_NUM_TEXCOORDS];
    int num_vertices;
} polygon_t;

void init_frustum_planes(
    float fov_x, float fov_y,
    float z_near, float z_far
);

polygon_t create_polygon_from_triangle(
    vec3_t v0, vec3_t v1, vec3_t v2,
    tex2_t t0, tex2_t t1, tex2_t t2
);

void clip_polygon(polygon_t* polygon);

void triangle_from_polygon(
    polygon_t* polygon,
    triangle_t triangle[], int*  num_triangles
);
 

