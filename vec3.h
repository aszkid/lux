#ifndef VEC3_H
#define VEC3_H

#include <stdbool.h>

typedef struct {
    double x, y, z;
} vec3;

void vec3_print(vec3 v);
void vec3_add(vec3 a, vec3 b, vec3 *res);
void vec3_sub(vec3 a, vec3 b, vec3 *res);
void vec3_mul(vec3 a, double s, vec3 *res);
void vec3_cross(vec3 a, vec3 b, vec3 *res);
double vec3_dot(vec3 a, vec3 b);
bool vec3_eq(vec3 a, vec3 b);
void vec3_normalize(vec3 a, vec3 *res);

#define VEC3_UNPACK(v) v.x, v.y, v.z

#endif
