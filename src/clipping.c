#include <math.h>

#include "clipping.h"
#include "texture.h"

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

///////////////////////////////////////////////////////////////////////////////
// Frustum planes are defined by a point and a normal vector
///////////////////////////////////////////////////////////////////////////////
// Near plane   :  P=(0, 0, znear), N=(0, 0,  1)
// Far plane    :  P=(0, 0, zfar),  N=(0, 0, -1)
// Top plane    :  P=(0, 0, 0),     N=(0, -cos(fov/2), sin(fov/2))
// Bottom plane :  P=(0, 0, 0),     N=(0, cos(fov/2), sin(fov/2))
// Left plane   :  P=(0, 0, 0),     N=(cos(fov/2), 0, sin(fov/2))
// Right plane  :  P=(0, 0, 0),     N=(-cos(fov/2), 0, sin(fov/2))
///////////////////////////////////////////////////////////////////////////////
//
//           /|\
//         /  | | 
//       /\   | |
//     /      | |
//  P*|-->  <-|*|   ----> +z-axis
//     \      | |
//       \/   | |
//         \  | | 
//           \|/
//
///////////////////////////////////////////////////////////////////////////////
void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far) {
	float cos_half_fov_x = cos(fov_x / 2);
	float sin_half_fov_x = sin(fov_x / 2);

	float cos_half_fov_y = cos(fov_y / 2);
	float sin_half_fov_y = sin(fov_y / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov_x;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov_x;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov_y;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov_y;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(
	vec3_t v0, vec3_t v1, vec3_t v2,
	tex2_t t0, tex2_t t1, tex2_t t2
) {
	polygon_t polygon = {
		.vertices = { v0, v1, v2 },
		.texcoords = { t0, t1, t2 },
		.num_vertices = 3
	};
	return polygon;
}

float float_lerp(float a, float b, float t) {
	return a + (t * (b - a));
}

void clip_polygon_against_plane(polygon_t* polygon, int plane) {
	vec3_t plane_point = frustum_planes[plane].point;
	vec3_t plane_normal = frustum_planes[plane].normal;

	// Declare a static array of inside vertices that will be part of the final polygon returned via parameter
	vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
	tex2_t inside_texcoords[MAX_NUM_TEXCOORDS];

	int num_inside_vertices = 0;

	// Start the current vertex with the first polygon vertex with first texture coordinates
	vec3_t* current_vertex = &polygon->vertices[0];
	tex2_t* current_texcoord = &polygon->texcoords[0];

	// Start the previous vertex with the last polygon vertex and texture coordinates
	vec3_t* previous_vertex = &polygon->vertices[polygon->num_vertices - 1];
	tex2_t* previous_texcoord = &polygon->texcoords[polygon->num_vertices - 1];

	// Calculate the dot product of the previous vertex
	float previous_dot = vec3_dot(plane_normal, vec3_sub(*previous_vertex, plane_point));

	// Loop all the polygon vertices while the current is different than the last one
	while (current_vertex != &polygon->vertices[polygon->num_vertices]) {
		float current_dot = vec3_dot(plane_normal, vec3_sub(*current_vertex, plane_point));
		
		// If we changed from inside to outsite or from outside to inside
		if (current_dot * previous_dot < 0) {
			// find the interpolation factor t
			float t = previous_dot / (previous_dot - current_dot);

			// Calculate the intersection point I = Q1 + t(Q2 - Q1)
			vec3_t intersection_point = vec3_add(
				*previous_vertex,
				vec3_mul(vec3_sub(*current_vertex, *previous_vertex), t)
			);

			// Use the lerp formula to get the interpolated U and V texture coordinates
			tex2_t interpolated_texcoord = {
				.u = float_lerp(previous_texcoord->u, current_texcoord->u, t),
				.v = float_lerp(previous_texcoord->v, current_texcoord->v, t),
			};

			// Insert the intersection point to the list of "inside vertices"
			inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
			inside_texcoords[num_inside_vertices] = tex2_clone(&interpolated_texcoord);
			num_inside_vertices++;
		}

		// If current vertex is inside the plane
		if (current_dot > 0) {
			// Insert the current vertex and texture coordinates to the list of "inside vertices"
			inside_vertices[num_inside_vertices] = vec3_clone(current_vertex);
			inside_texcoords[num_inside_vertices] = tex2_clone(current_texcoord);

			num_inside_vertices++;
		}

		// Move to the next vertex
		previous_dot = current_dot;

		previous_vertex = current_vertex;
		previous_texcoord = current_texcoord;

		current_vertex++;
		current_texcoord++;
	}

	// At the end, copy the list of inside vertices into the destination polygon (out parameter)
	for (int i = 0; i < num_inside_vertices; i++) {
		polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
		polygon->texcoords[i] = tex2_clone(&inside_texcoords[i]);
	}

	polygon->num_vertices = num_inside_vertices;
}

void clip_polygon(polygon_t* polygon) {
	clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}

void triangle_from_polygon(polygon_t* polygon, triangle_t triangle[], int*  num_triangles) {
	for (int i = 0; i < polygon->num_vertices - 2; i++) {
		int v1 = 0;
		int v2 = i + 1;
		int v3 = i + 2;

		triangle[i].points[0] = vec4_from_vec3(polygon->vertices[v1]);
		triangle[i].points[1] = vec4_from_vec3(polygon->vertices[v2]);
		triangle[i].points[2] = vec4_from_vec3(polygon->vertices[v3]);

		triangle[i].tex_coords[0] = polygon->texcoords[v1];
		triangle[i].tex_coords[1] = polygon->texcoords[v2];
		triangle[i].tex_coords[2] = polygon->texcoords[v3];
	}
	*num_triangles = polygon->num_vertices - 2;
}