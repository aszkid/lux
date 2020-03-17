#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "vec3.h"
#include "camera.h"
#include "ppm.h"
#include "utlist.h"

typedef struct {
    vec3 color;
    float depth;
} collision_t;

typedef bool collide(vec3, vec3, void*, collision_t*);

typedef struct job {
    uint8_t *data;
    size_t obj_size;
    size_t obj_num;
    collide *test;
    struct job *next;
} job_t;

typedef struct {
    ppm_t *ppm;
    float *depth;
    camera_t camera;
    vec3 light;
    job_t *jobs;
} lux_t;

////////////////////////////////////
// GEOMETRY
////////////////////////////////////

typedef struct {
    vec3 color;
    vec3 u, v; // orientation
    vec3 p; // any point on the plane
} plane_t;

bool test_ray_plane(vec3 camera, vec3 ray, void *obj, collision_t *col)
{
    plane_t *plane = (plane_t*) obj;

    vec3 m;
    vec3_sub(camera, plane->p, &m);
    vec3 n;
    vec3_cross(plane->u, plane->v, &n);

    double nm = vec3_dot(n, m);
    double nray = vec3_dot(n, ray);

    if (nm * nray < 0.0) {
        col->depth = - nm / nray;
        col->color = plane->color;
        return true;
    }

    return false;
}

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

    col->color = s->color;
    col->depth = t;

    return true;
}

/*
 * [render_objects] render pixel (i, j) given object array, pixel depth, and ray to test on
 *   lux: lux context
 *   w: pointer to pixel depth (float)
 *   i, j: pixel coordinates
 *   ray: ray to test for
 *   data: pointer to object data array
 *   test: object test function function
 *   obj_size: sizeof object type
 *   obj_n: number of objects in data array
 */
void render_objects(lux_t *lux, float *w, size_t i, size_t j, vec3 ray, job_t *job)
{
    collision_t col;
    for (size_t k = 0; k < job->obj_num; k++) {
        // test if ray hits object
        if (job->test(lux->camera.p, ray, job->data + k * job->obj_size, &col) && *w > col.depth) {
            // ray hits object in this position
            vec3 source;
            vec3_mul(ray, col.depth, &source);
            vec3_add(source, lux->camera.p, &source);
            *w = col.depth;

            // check if light source hits this
            vec3 light_ray = lux->light;
            vec3_sub(light_ray, source, &light_ray);
            vec3_normalize(light_ray, &light_ray);

            // nudge a bit
            vec3 lil = light_ray;
            vec3_mul(lil, 0.001, &lil);
            vec3_add(source, lil, &source);

            double r, g, b;
            r = 255.0 * col.color.x;
            g = 255.0 * col.color.y;
            b = 255.0 * col.color.z;

            // check if some object obstructs the direct path towards our light source
            job_t *job2;
            LL_FOREACH(lux->jobs, job2) {
                for (size_t s = 0; s < job2->obj_num; s++) {
                    if (job2->test(source, light_ray, job2->data + s * job2->obj_size, &col)) {
                        r *= 0.2; g *= 0.2; b *= 0.2;
                        break;
                    }
                }
            }
            ppm_write_at(lux->ppm, i, j, r, g, b);
        }
    }
}

int lux_render(lux_t *lux)
{
    for (size_t j = 0; j < lux->ppm->height; j++) {
        for (size_t i = 0; i < lux->ppm->width; i++) {
            // compute ray corresponding to pixel (i, j)
            vec3 ray = camera_pixel_to_ray(
                &lux->camera,
                (double) i / (double) lux->ppm->width,
                (double) j / (double) lux->ppm->height, 
                ((double) lux->ppm->width) / lux->ppm->height
            );

            // render all jobs on this ray
            job_t *job;
            LL_FOREACH(lux->jobs, job) {
                render_objects(lux, &lux->depth[lux->ppm->width * j + i], i, j, ray, job);
            }
        }
    }

    return 0;
}

void lux_submit_job(lux_t *lux, job_t *job)
{
    LL_APPEND(lux->jobs, job);
}

int main(int argc, char **argv)
{
    const size_t WIDTH = 2000;
    const size_t HEIGHT = WIDTH;

    lux_t lux = {
        .ppm = ppm_create("out.ppm", WIDTH, HEIGHT),
        .depth = malloc(sizeof(float) * WIDTH * HEIGHT),
        .camera = {
            .p = (vec3) { 5.0, 1.0, -3.0 },
            .fov = 15.0
        },
        .light = { 5.0, 5.0, 0.0 },
        .jobs = NULL,
    };

    camera_look_at((vec3) { 0.0, 0.0, 0.0 }, &lux.camera);

    for (size_t i = 0; i < HEIGHT * WIDTH; i++)
        lux.depth[i] = FLT_MAX;

    sphere_t spheres[3];
    spheres[0] = (sphere_t) {
        .r = 0.25,
        .pos = { -0.5, 0.2, 0.0 },
        .color = { 1.0, 0.0, 0.0 }
    };
    spheres[1] = (sphere_t) {
        .r = 0.25,
        .pos = { 0.5, 0.1, 0.0 },
        .color = { 0.0, 1.0, 0.0 }
    };
    spheres[2] = (sphere_t) {
        .r = 0.25,
        .pos = { 0.0, 0.0, 0.0 },
        .color = { 0.0, 0.0, 1.0 }
    };

    plane_t xz = {
        .p = { 0.0, -0.25, 0.0 },
        .u = { 1.0, 0.0, 0.0 },
        .v = { 0.0, 0.0, 1.0 },
        .color = { 1.0, 0.4, 0.7 }
    };

    job_t *job;
    
    job = malloc(sizeof(job_t));
    job->data = (uint8_t*) &xz;
    job->test = &test_ray_plane;
    job->obj_size = sizeof(plane_t);
    job->obj_num = 1;
    lux_submit_job(&lux, job);

    job = malloc(sizeof(job_t));
    job->data = (uint8_t*) spheres;
    job->test = &test_ray_sphere;
    job->obj_size = sizeof(sphere_t);
    job->obj_num = 3;
    lux_submit_job(&lux, job);
    
    lux_render(&lux);

    job_t *tmp;
    LL_FOREACH_SAFE(lux.jobs, job, tmp) {
        free(job);
    }
    ppm_close(lux.ppm);
    free(lux.depth);

    return 0;
}

