all: mercator mercator_semaphore

mercator: mercator.c
	gcc -o mercator mercator.c -lm -pthread

mercator_semaphore: mercator_semaphore.c semaphoresarr.c semaphoresarr.h
	gcc -o mercator_semaphore mercator_semaphore.c semaphoresarr.c -lm -pthread
