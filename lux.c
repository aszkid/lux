#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "vec3.h"
#include "ppm.h"

typedef struct {
    vec3 color;
    float depth;
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

    // Check if interscetion
    if (c > 0.0 && b > 0.0) return false;

    float discr = b*b - c;
    if (discr < 0.0) return false;

    // Calculate intersection
    float t = -b - sqrt(discr);
    if (t < 0.0) t = 0.0;

    col->color = (vec3) {
        s->color.x, s->color.y, s->color.z
    };
    col->depth = t;

    return true;
}

int render_job(ppm_t *ppm, float *depth, void* data, collide *test, size_t obj_size, size_t obj_n)
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
                    float *w = &depth[ppm->width * j + i];
                    if (*w > col.depth) {
                        *w = col.depth;
                        ppm_write_at(
                            ppm, i, j,
                            255.0 * col.color.x,
                            255.0 * col.color.y,
                            255.0 * col.color.z
                        );
                    }
                }
            }
        }
    }

    return 0;
}


int main(int argc, char **argv)
{
    const size_t WIDTH = 1000;
    const size_t HEIGHT = WIDTH;

    // open output PPM file
    ppm_t *ppm = ppm_create("test.ppm", WIDTH, HEIGHT);

    // create depth map
    float *depth = malloc(sizeof(float) * WIDTH * HEIGHT);
    for (size_t i = 0; i < HEIGHT * WIDTH; i++)
        depth[i] = FLT_MAX;

    sphere_t spheres[3];
    spheres[0] = (sphere_t) {
        .r = 0.5,
        .pos = { -0.5, 0.0, 1.0 },
        .color = { 1.0, 0.0, 0.0 }
    };
    spheres[1] = (sphere_t) {
        .r = 0.5,
        .pos = { 0.5, 0.0, 1.0 },
        .color = { 0.0, 1.0, 0.0 }
    };
    spheres[2] = (sphere_t) {
        .r = 0.5,
        .pos = { 0.0, 0.0, 1.0 },
        .color = { 0.0, 0.0, 1.0 }
    };

    render_job(ppm, depth, spheres, &test_ray_sphere, sizeof(sphere_t), 3);
    free(depth);
    ppm_close(ppm);

    return 0;
}

