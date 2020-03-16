#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "vec3.h"
#include "ppm.h"

typedef struct {
    /* up direction (tilt) */
    vec3 u;
    /* camera direction (watch) */
    vec3 v;
    /* camera position */
    vec3 p;
    /* horizontal fov in degrees */
    double fov;
} camera_t;

camera_t camera_build(vec3 up, vec3 watch, vec3 pos, double fov)
{
    camera_t cam;

    vec3_normalize(up, &cam.u);
    vec3_normalize(watch, &cam.v);
    cam.p = pos;
    cam.fov = fov;
    
    return cam;
}

typedef struct {
    ppm_t *ppm;
    float *depth;
    camera_t camera;
} lux_t;

vec3 pixel_to_ray(camera_t *camera, double i, double j, double aspect_ratio)
{
    double h = 2 * tan(camera->fov * (M_PI / 180.0));
    double w = aspect_ratio * h;

    // center of plane
    vec3 center;
    vec3_add(camera->p, camera->v, &center);
    vec3 l;
    vec3_cross(camera->v, camera->u, &l);

    vec3 world = { 0.0, 0.0, 0.0 };
    vec3_add(world, center, &world);
    vec3 au;
    vec3_mul(camera->u, (h / 2) * (1 - 2 * j), &au);
    vec3 bl;
    vec3_mul(l, (w / 2) * (1 - 2 * i), &bl);
    vec3_add(world, au, &world);
    vec3_add(world, bl, &world);
    
    vec3 ray;
    vec3_sub(world, camera->p, &ray);

    return ray;
}

typedef struct {
    vec3 color;
    float depth;
} collision_t;

typedef bool collide(vec3, vec3, void*, collision_t*);


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
void render_objects(lux_t *lux, float *w, size_t i, size_t j, vec3 ray, void *data, collide *test, size_t obj_size, size_t obj_n)
{
    collision_t col;
    for (size_t k = 0; k < obj_n; k++) {
        if (test(lux->camera.p, ray, data + k * obj_size, &col) && *w > col.depth) {
            *w = col.depth;
            ppm_write_at(
                lux->ppm, i, j,
                255.0 * col.color.x,
                255.0 * col.color.y,
                255.0 * col.color.z
            );
        }
    }
}

int render_job(lux_t *lux, void* data, collide *test, size_t obj_size, size_t obj_n)
{
    for (size_t j = 0; j < lux->ppm->height; j++) {
        for (size_t i = 0; i < lux->ppm->width; i++) {
            // project pixel (i, j) to R^3 screen coordinates
            // TODO we are assuming 1:1 aspect ratio
            /*vec3 proj = {
                2.0 * (double) i / (double) lux->ppm->width - 1.0,
                -2.0 * (double) j / (double) lux->ppm->height + 1.0,
                0.0
            };
            vec3 ray;
            vec3_sub(proj, lux->camera, &ray);
            vec3_normalize(ray, &ray);
            */

            vec3 ray = pixel_to_ray(
                &lux->camera,
                (double) i / (double) lux->ppm->width,
                (double) j / (double) lux->ppm->height, 
                ((double) lux->ppm->width) / lux->ppm->height
            );
            //printf("pixel (%zu, %zu) gives ray (%f, %f, %f)\n", i, j, VEC3_UNPACK(ray));
            vec3_normalize(ray, &ray);

            // for each object given, test ray
            render_objects(lux, &lux->depth[lux->ppm->width * j + i], i, j, ray, data, test, obj_size, obj_n);
        }
    }

    return 0;
}


int main(int argc, char **argv)
{
    const size_t WIDTH = 1000;
    const size_t HEIGHT = WIDTH;

    lux_t lux = {
        .ppm = ppm_create("test.ppm", WIDTH, HEIGHT),
        .depth = malloc(sizeof(float) * WIDTH * HEIGHT),
        .camera = camera_build(
            (vec3) { 0.0, 1.0, 0.0 },   // up
            (vec3) { -1.0, 0.0, 1.0 },   // watch
            (vec3) { 1.0, 0.0, -1.0 },  // position
            65.0                        // fov
        ),
    };

    for (size_t i = 0; i < HEIGHT * WIDTH; i++)
        lux.depth[i] = FLT_MAX;

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

    plane_t xz = {
        .p = { 0.0, -0.25, 0.0 },
        .u = { 1.0, 0.0, 0.0 },
        .v = { 0.0, 0.0, 1.0 },
        .color = { 1.0, 0.4, 0.7 }
    };

    render_job(&lux, &xz, &test_ray_plane, sizeof(plane_t), 1);
    render_job(&lux, spheres, &test_ray_sphere, sizeof(sphere_t), sizeof(spheres) / sizeof(sphere_t));

    free(lux.depth);
    ppm_close(lux.ppm);

    return 0;
}

