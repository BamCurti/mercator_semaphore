all: mercator mercator_semaphore

mercator: mercator.c
	gcc -o mercator mercator.c -lm

mercator_semaphore: mercator_semaphore.c semaphoresarr.h
	gcc -o mercator_semaphore mercator_semaphore.c -lm -pthread
