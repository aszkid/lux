CFLAGS = -Wall
LFLAGS = -lm

all: lux

ppm.o: ppm.c
	gcc -c $(CFLAGS) $< -o $@

vec3.o: vec3.c
	gcc -c $(CFLAGS) $< -o $@

camera.o: camera.c
	gcc -c $(CFLAGS) $< -o $@

lux.o: lux.c
	gcc -c $(CFLAGS) $< -o $@

lux: lux.o vec3.o ppm.o camera.o
	gcc $^ $(LFLAGS) -o $@

clean:
	rm -f lux lux.o vec3.o ppm.o camera.o

.PHONY: clean
