#ifndef PPN_H
#define PPN_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *f;
    uint8_t *data;
    size_t width, height;
} ppm_t;

ppm_t *ppm_create(char *name, size_t width, size_t height);
int ppm_close(ppm_t *ppm);
void ppm_write_at(ppm_t *ppm, size_t i, size_t j, uint8_t r, uint8_t g, uint8_t b);

#endif
