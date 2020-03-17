#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h" 

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

camera_t camera_build(vec3 watch, vec3 pos, double fov);
void camera_look_at(vec3 pos, camera_t *cam);
vec3 camera_pixel_to_ray(camera_t *camera, double i, double j, double aspect_ratio);

#endif
