#include "vec3.h"
#include <stdio.h>
#include <math.h>

#define EPSILON 0.0000001
#define WITHIN(a, b) (fabs(a - b) < EPSILON)

void vec3_print(vec3 v)
{
    printf("(%f, %f, %f)", v.x, v.y, v.z);
}

void vec3_add(vec3 a, vec3 b, vec3 *res)
{
    res->x = a.x + b.x;
    res->y = a.y + b.y;
    res->z = a.z + b.z;
}

void vec3_sub(vec3 a, vec3 b, vec3 *res)
{
    res->x = a.x - b.x;
    res->y = a.y - b.y;
    res->z = a.z - b.z;
}

void vec3_mul(vec3 a, double s, vec3 *res)
{
    res->x = s * a.x;
    res->y = s * a.y;
    res->z = s * a.z;
}

void vec3_cross(vec3 a, vec3 b, vec3 *res)
{
    res->x = a.y * b.z - a.z * b.y;
    res->y = a.z * b.x - a.x * b.z;
    res->z = a.x * b.y - a.y * b.x;
}

double vec3_dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

bool vec3_eq(vec3 a, vec3 b)
{
    return WITHIN(a.x, b.x) && WITHIN(a.y, b.y) && WITHIN(a.z, b.z);
}

void vec3_normalize(vec3 a, vec3 *res)
{
    double norm = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
    res->x = a.x / norm;
    res->y = a.y / norm;
    res->z = a.z / norm;
}

