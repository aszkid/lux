all: lux

ppm.o: ppm.c
	gcc -c ppm.c -Wall -o ppm.o

vec3.o: vec3.c
	gcc vec3.c -c -Wall -o vec3.o

lux.o: lux.c
	gcc -c lux.c -Wall -o lux.o

lux: lux.o vec3.o ppm.o
	gcc $^ -lm -o lux

clean:
	rm -f lux lux.o vec3.o ppm.o

.PHONY: clean
