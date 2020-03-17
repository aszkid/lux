#include "camera.h"
#include <math.h>

void set_vertical_up(camera_t *cam)
{
    cam->u = (vec3) { 0.0, 1.0, 0.0 };
    if (cam->v.y != 0) {
        vec3_mul(cam->u, - vec3_dot(cam->v, cam->v) / cam->v.y, &cam->u);
        vec3_add(cam->u, cam->v, &cam->u);
        vec3_normalize(cam->u, &cam->u);
    }
}

camera_t camera_build(vec3 watch, vec3 pos, double fov)
{
    camera_t cam;
    vec3_normalize(watch, &cam.v);
    set_vertical_up(&cam);
    cam.p = pos;
    cam.fov = fov;
    
    return cam;
}

void camera_look_at(vec3 pos, camera_t *cam)
{
    cam->v = pos;
    vec3_sub(cam->v, cam->p, &cam->v);
    vec3_normalize(cam->v, &cam->v);
    set_vertical_up(cam);
}

vec3 camera_pixel_to_ray(camera_t *camera, double i, double j, double aspect_ratio)
{
    double h = 2 * tan(camera->fov * (M_PI / 180.0));
    double w = aspect_ratio * h;

    vec3 center, l, ray, au, bl;
    vec3_add(camera->p, camera->v, &center);
    vec3_cross(camera->v, camera->u, &l);

    vec3 world = { 0.0, 0.0, 0.0 };
    vec3_add(world, center, &world);
    vec3_mul(camera->u, (h / 2) * (1 - 2 * j), &au);
    vec3_mul(l, (w / 2) * (1 - 2 * i), &bl);
    vec3_add(world, au, &world);
    vec3_add(world, bl, &world);
    
    vec3_sub(world, camera->p, &ray);
    vec3_normalize(ray, &ray);

    return ray;
}


