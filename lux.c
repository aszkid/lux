#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vec3.h"
#include "ppm.h"

typedef struct {
    vec3 color;
    double depth;
} collision_t;

typedef bool collide(vec3, void*, collision_t*);

typedef struct {
    double r;
    vec3 pos;
} sphere_t;

bool test_ray_sphere(vec3 ray, void *obj, collision_t *col)
{
    sphere_t *s = (sphere_t*) obj;
    vec3 p = {0.0, 0.0, -1.0};
    vec3 m;
    vec3_sub(p, s->pos, &m);

    double b = vec3_dot(m, ray);
    double c = vec3_dot(m, m) - s->r*s->r;

    if (c > 0.0 && b > 0.0) return false;

    float discr = b*b - c;
    if (discr < 0.0) return false;

    float t = -b - sqrt(discr);
    if (t < 0.0) t = 0.0;

    vec3 q = ray;
    vec3_mul(q, t, &q);
    vec3_add(q, p, &q);
    return true;
}


int render_job(ppm_t *ppm, void* data, collide *test, size_t obj_size, size_t obj_n)
{
    vec3 camera = {0.0, 0.0, -1.0};
    collision_t col;
    for (size_t j = 0; j < ppm->height; j++) {
        for (size_t i = 0; i < ppm->width; i++) {
            // project pixel (i, j) to R^3 screen coordinates
            // TODO we are assuming 1:1 aspect ratio, and
            //      camera at (0.0, 0.0, -1.0)
            vec3 proj = {
                2.0 * (double) i / (double) ppm->width - 1.0,
                -2.0 * (double) j / (double) ppm->height + 1.0,
                0.0
            };

            // compute normalized ray from camera to projected pixel coordinates
            vec3 ray;
            vec3_sub(proj, camera, &ray);
            vec3_normalize(ray, &ray);

            // for each object given, test ray
            for (size_t k = 0; k < obj_n; k++) {
                if (test(ray, data + k * obj_size, &col)) {
                    ppm_write_at(ppm, i, j, 255, 0, 0);
                }
            }
        }
    }

    return 0;
}


int main(int argc, char **argv)
{
    vec3 a = {1., 5., 0.};
    vec3 b = {3., -1., 0.};
    printf("a = (%f, %f, %f)\n", VEC3_UNPACK(a));
    printf("b = (%f, %f, %f)\n", VEC3_UNPACK(b));
    printf("dot(a, b) = %f\n", vec3_dot(a, b));
    vec3 c;
    vec3_add(a, b, &c);
    printf("add(a, b) = (%f, %f, %f)\n", VEC3_UNPACK(c));

    vec3 i = {1., 0., 0.};
    vec3 j = {0., 1., 0.};
    vec3 k = {0., 0., 1.};
    printf("i = (%f, %f, %f)\n", VEC3_UNPACK(i));
    printf("j = (%f, %f, %f)\n", VEC3_UNPACK(j));
    printf("k = (%f, %f, %f)\n", VEC3_UNPACK(k));
    vec3 k_q;
    vec3_cross(i, j, &k_q);
    printf("cross(i, j) = (%f, %f, %f) = k? %s\n", VEC3_UNPACK(k_q), vec3_eq(k_q, k) ? "YES" : "NO");

    ppm_t *ppm = ppm_create("test.ppm", 1000, 1000);

    sphere_t spheres[2];
    // sphere 0
    spheres[0].r = 0.5;
    spheres[0].pos.x = -0.5;
    spheres[0].pos.y = 0.0;
    spheres[0].pos.z = 1.0;
    // sphere 1
    spheres[1].r = 0.5;
    spheres[1].pos.x = 0.5;
    spheres[1].pos.y = 0.0;
    spheres[1].pos.z = 1.0;

    render_job(ppm, spheres, &test_ray_sphere, sizeof(sphere_t), 2);
    ppm_close(ppm);

    return 0;
}

