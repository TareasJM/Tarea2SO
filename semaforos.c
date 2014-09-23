#include <stdio.h>
#include <semaphore.h>

int main ( )
{
	sem_t sem[2];
	printf("tamano: %lu\n", sizeof(sem));
	return 0;

} /* fin de la función main */