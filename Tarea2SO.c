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

void upTime();
void RamInfo();
void ProcsInfo();
void readSHM(char* shm, sem_t *sem);
void writeSHM(char* shm, char* string, sem_t *sem);

int main (int argc, char const *argv[])
{ 

  // Verificación de argumentos
  if (argc != 3)
  {
    printf("Uso: ./Tarea2SO <#Repeticiones> <TiempoEspera>\n");
    return 0; //Si hay error, termina
  }
  int rep, wait;
  sscanf(argv[1], "%d", &rep);
  sscanf(argv[2], "%d", &wait);

	int shmMsg;
  char *shm;
  sem_t *sem = (sem_t*)malloc(sizeof(sem_t)*2); // semaforos
  sem_init(&sem[0],1,1); // semaforo de memoria
  sem_init(&sem[1],1,0); // semaforo de mensaje

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
		// cambiar por semaforo while(cantWrite(shm));
	  printf("%d: SubProceso finalizado.\n", getpid());
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
		  RamInfo(shm, getpid(), sem);
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
		// cambiar por semaforo while(cantWrite(shm));
	  printf("%d: SubProceso finalizado.\n", getpid());
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
		  ProcsInfo(shm, getpid(), sem);
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
		// cambiar por semaforo while(cantWrite(shm));
	  printf("%d: SubProceso finalizado.\n", getpid());
		exit(0); 
	}     

  int i = (3*rep);
  while(i-- > 0)
  {
  	readSHM(shm, sem);
  }
  printf("%d: Programa finalizado\n", getpid());

  return 0;

}

// Escribe el tiempo del sistema
void upTime(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
	// cambiar por semaforo while(cantWrite(shmMsg));
	char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: El sistema lleva %lus encendido.\n", pid, info.uptime);
	writeSHM(shm, msg, sem);
	free(msg);
  return;
}

// Escribe la información de la RAM del sistema
void RamInfo(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
	// cambiar por semaforo while(cantWrite(shmMsg));
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Disponibles %luMB de %luMB.\n", pid, (info.freeram)/1048576, (info.totalram)/1048576);
	writeSHM(shm, msg, sem);
	free(msg);
  return;
}

// Escribe la información de Procesos del sistema
void ProcsInfo(char* shm, int pid, sem_t *sem)
{
	struct sysinfo info;
	sysinfo(&info);
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Hay %d procesos en ejecucion.\n", pid, info.procs);
	writeSHM(shm, msg, sem);
	free(msg);
  return;
}

// Espera a que haya un mensaje en SHM y lo escribe a log.txt
void readSHM(char* shm, sem_t *sem)
{	
	sem_wait(&sem[1]); // espera a que haya mensaje
	sem_wait(&sem[0]);

  FILE *p;
  p = fopen("log.txt","a+");
  fprintf(p, "%s\n", shm);
  fclose(p);

  sem_post(&sem[1]);
  sem_post(&sem[0]);
}

// Espera a que SHM esté vacia y escribe un mensaje
void writeSHM(char* shm, char* string, sem_t *sem)
{
	sem_wait(&sem[1]);
	sem_wait(&sem[0]);

	strcpy(shm, string);

  sem_post(&sem[1]);
  sem_post(&sem[0]);

}
//compilar gcc Tarea2SO.c -lpthread -Wall -lm