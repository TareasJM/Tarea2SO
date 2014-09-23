#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdlib.h>

//Tamaño y nombre de memoria compartida
#define SHMSZ 256 // 1 string de 256 chars
#define SHMK 1234 // nombre random
#define SEMSZ 96 //  3 semáforos
#define SEMK 1235 // nombre random + 1

void upTime(char* shm, int pid, sem_t *sem);
void ramInfo(char* shm, int pid, sem_t *sem);
void procsInfo(char* shm, int pid, sem_t *sem);
void finalizeProc(char* shm, int pid, sem_t *sem);
void readSHM(char* shm, sem_t *sem);
void writeSHM(char* shm, char* string, sem_t *sem);

int main (int argc, char const *argv[])
{ 

  // Verificación de argumentos
  if (argc != 3)
  {
    printf("Uso: ./Tarea2SO <#Repeticiones> <TiempoEspera>\n");
    return 1; //Si hay error, termina
  }
  int rep, wait;
  sscanf(argv[1], "%d", &rep);
  sscanf(argv[2], "%d", &wait);

	int shmMsg;
	int shmSem;
  char *shm;
  sem_t *sem;

  // pide la memoria compartida
  if ((shmSem = shmget(SEMK, SEMSZ, IPC_CREAT | 0666)) < 0)
  {
	  printf("Error: shmget\n");
	  return 1;
  }

  if ((sem = shmat(shmSem, NULL, 0)) == (sem_t *) -1)
  {
	  printf("Error: shmat\n");
	  return 1;
  }

  // sem_t *sem = (sem_t*)malloc(sizeof(sem_t)*3); // semaforos (cuando es 1 entra :L)
  sem_init(&sem[0],1,1); // semaforo de memoria (mutex)
  sem_init(&sem[1],1,1); // semaforo de mensaje (empty)
  sem_init(&sem[2],1,0); // semaforo de mensaje (full)

  // Limpia el txt
  FILE *p;
  p = fopen("log.txt","w");
  fclose(p);


  // pide la memoria compartida
  if ((shmMsg = shmget(SHMK, SHMSZ, IPC_CREAT | 0666)) < 0)
  {
	  printf("Error: shmget\n");
	  return 1;
  }

  if ((shm = shmat(shmMsg, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat\n");
	  return 1;
  }
  *shm = '\0';

 	// primer fork
	pid_t pid = fork();
	if(pid == -1)
	{
	  printf("Error al crear proceso hijo \n");
		printf("%s", strerror(errno));
		return 1;
	}
	if(pid == 0)
	{
		int r = rep;
		while(r-- > 0)
		{	
			// analiza
		  upTime(shm, getpid(), sem);
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
	  finalizeProc(shm, getpid(), sem);
		exit(0);
	}

	// segundo fork
	pid = fork();
	if(pid == -1)
	{
	  printf("Error al crear proceso hijo \n");
		printf("%s", strerror(errno));
		return 1;
	}
	if(pid == 0)
	{
		int r = rep;
		while(r-- > 0)
		{	
			// analiza
		  ramInfo(shm, getpid(), sem);
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
	  finalizeProc(shm, getpid(), sem);
		exit(0);
	}

	// tercer fork
	pid = fork();
	if(pid == -1)
	{
	  printf("Error al crear proceso hijo \n");
		printf("%s", strerror(errno));
		return 1;
	}
	if(pid == 0)
	{
		int r = rep;
		while(r-- > 0)
		{	
			// analiza
		  procsInfo(shm, getpid(), sem);
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
	  finalizeProc(shm, getpid(), sem);
		exit(0); 
	}     

  int i = ((3*rep) + 3);
  while(i-- > 0)
  {
  	readSHM(shm, sem);
  }

  p = fopen("log.txt","a+");
  fprintf(p, "%d: Programa finalizado\n", getpid());
  fclose(p);
  sem_destroy(&sem[0]);
  sem_destroy(&sem[1]);
  sem_destroy(&sem[2]);
  shmdt(shm);
  shmdt(sem);

  return 0;
}

// Escribe el tiempo del sistema
void upTime(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
	char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: El sistema lleva %lus encendido.\n", pid, info.uptime);
	writeSHM(shm, msg, sem);
	free(msg);
}

// Escribe la información de la RAM del sistema
void ramInfo(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Disponibles %luMB de %luMB.\n", pid, (info.freeram)/1048576, (info.totalram)/1048576);
	writeSHM(shm, msg, sem);
	free(msg);
}

// Escribe la información de Procesos del sistema
void procsInfo(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Hay %d procesos en ejecucion.\n", pid, info.procs);
	writeSHM(shm, msg, sem);
	free(msg);
}

// Escribe la finalización de un Subproceso
void finalizeProc(char* shm, int pid, sem_t *sem)
{
	char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: SubProceso finalizado.\n", pid);
	writeSHM(shm, msg, sem);
	free(msg);
}

// Espera a que haya un mensaje en SHM y lo escribe a log.txt (consumer)
void readSHM(char* shm, sem_t *sem)
{	
	// printf("readSHM: wait(full)\n");
	sem_wait(&sem[2]); // full
	// printf("readSHM: wait(mutex)\n");
	sem_wait(&sem[0]); // mutex

  FILE *p;
  p = fopen("log.txt","a+");
  fprintf(p, "%s", shm);
  fclose(p);

	// printf("readSHM: post(mutex)\n");
  sem_post(&sem[0]); // mutex
	// printf("readSHM: post(empty)\n");
  sem_post(&sem[1]); // empty
}

// Espera a que SHM esté vacia y escribe un mensaje (producer)
void writeSHM(char* shm, char* string, sem_t *sem)
{
	// printf("writeSHM: wait(empty)\n");
	sem_wait(&sem[1]); // empty
	// printf("writeSHM: wait(mutex)\n");
	sem_wait(&sem[0]); // mutex

	strcpy(shm, string);

	// printf("writeSHM: post(mutex)\n");
  sem_post(&sem[0]); // mutex
	// printf("writeSHM: post(full)\n");
  sem_post(&sem[2]); // full

}
//compilar gcc Tarea2SO.c -lpthread -Wall -lm