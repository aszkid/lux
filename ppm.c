#include "ppm.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

ppm_t *ppm_create(char *name, size_t width, size_t height)
{
    ppm_t *ppm = malloc(sizeof(ppm_t));
    ppm->f = fopen(name, "wb");
    ppm->width = width;
    ppm->height = height;

    char buf[64];
    sprintf(buf, "P3\n%zu %zu\n255\n", width, height);
    fwrite(buf, 1, strlen(buf), ppm->f);

    ppm->data = calloc(1, 3 * width * height);

    return ppm;
}

int ppm_close(ppm_t *ppm)
{
    char buf[32];
    uint8_t r, g, b;
    for (size_t i = 0; i < ppm->height; i++) {
        for (size_t j = 0; j < ppm->width; j++) {
            r = ppm->data[3 * (i * ppm->width + j)];
            g = ppm->data[3 * (i * ppm->width + j) + 1];
            b = ppm->data[3 * (i * ppm->width + j) + 2];
            sprintf(buf, "%d %d %d\n", r, g, b);
            fwrite(buf, 1, strlen(buf), ppm->f);
        }
    }

    fclose(ppm->f);
    free(ppm->data);
    free(ppm);

    return 0;
}

void ppm_write_at(ppm_t *ppm, size_t i, size_t j, uint8_t r, uint8_t g, uint8_t b)
{
    assert(i < ppm->width);
    assert(j < ppm->height);

    uint8_t *at = &ppm->data[3 * (j * ppm->width + i)];
    *(at++) = r;
    *(at++) = g;
    *(at++) = b;
}

