#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vec3.h"
#include "ppm.h"

typedef struct {
    vec3 color;
    double depth;
} collision_t;

typedef bool collide(vec3, vec3, void*, collision_t*);

typedef struct {
    vec3 color;
    double r;
    vec3 pos;
} sphere_t;

bool test_ray_sphere(vec3 camera, vec3 ray, void *obj, collision_t *col)
{
    sphere_t *s = (sphere_t*) obj;
    vec3 m;
    vec3_sub(camera, s->pos, &m);

    double b = vec3_dot(m, ray);
    double c = vec3_dot(m, m) - s->r*s->r;

    if (c > 0.0 && b > 0.0) return false;

    float discr = b*b - c;
    if (discr < 0.0) return false;

    float t = -b - sqrt(discr);
    if (t < 0.0) t = 0.0;

    vec3 q = ray;
    vec3_mul(q, t, &q);
    vec3_add(q, camera, &q);

    col->color.x = s->color.x;
    col->color.y = s->color.y;
    col->color.z = s->color.z;

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
                if (test(camera, ray, data + k * obj_size, &col)) {
                    ppm_write_at(ppm, i, j, 255.0 * col.color.x, 255.0 * col.color.y, 255.0 * col.color.z);
                }
            }
        }
    }

    return 0;
}


int main(int argc, char **argv)
{
    ppm_t *ppm = ppm_create("test.ppm", 1000, 1000);

    sphere_t spheres[2];
    // sphere 0
    spheres[0].r = 0.5;
    spheres[0].pos.x = -0.5;
    spheres[0].pos.y = 0.0;
    spheres[0].pos.z = 1.0;
    spheres[0].color.x = 1.0;
    spheres[0].color.y = 0.0;
    spheres[0].color.z = 0.0;
    // sphere 1
    spheres[1].r = 0.5;
    spheres[1].pos.x = 0.5;
    spheres[1].pos.y = 0.0;
    spheres[1].pos.z = 1.0;
    spheres[1].color.x = 0.0;
    spheres[1].color.y = 1.0;
    spheres[1].color.z = 0.0;

    render_job(ppm, spheres, &test_ray_sphere, sizeof(sphere_t), 2);
    ppm_close(ppm);

    return 0;
}

