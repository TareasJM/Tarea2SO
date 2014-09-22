#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>

//Tamaño y nombre de memoria compartida
#define SHMSZ 256 // 1 string de 256 chars
#define SHMK 1234 // nombre random

void upTime();
void RamInfo();
void ProcsInfo();
int cantWrite(int shmid);
int cantRead(int shmid);
void readSHM(int shmid);
void writeSHM(int shmid, char* string);

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

  int shmid;
  char *msg;

  // pide la memoria compartida
  if ((shmid = shmget(SHMK, SHMSZ, IPC_CREAT | 0666)) < 0)
  {
	  printf("Error: shmget\n");
	  return 1;
  }

  if ((msg = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat\n");
	  return 1;
  }

  *msg++ = '0';
  *msg-- = '\0';

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
		  upTime(shmid, getpid());		
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
		while(cantWrite(shmid));
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
		sleep(5);
		while(r-- > 0)
		{	
			// analiza
		  RamInfo(shmid, getpid());
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
		while(cantWrite(shmid));
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
		sleep(10);
		int r = rep;
		while(r-- > 0)
		{	
			// analiza
		  ProcsInfo(shmid, getpid());
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
		while(cantWrite(shmid));
	  printf("%d: SubProceso finalizado.\n", getpid());
		exit(0); 
	}     

  // fin del proceso padre
  int i = (3*rep) + 3;
  while(i-- > 0)
  {
  	while(cantRead(shmid))
  		sleep(1);
  	readSHM(shmid);
  }
  printf("%d: Programa finalizado\n", getpid());

  return 0;

}

//Hace fork y escribe el tiempo del sistema
void upTime(int shmid, int pid)
{
	struct sysinfo info;
	sysinfo(&info);
	while(cantWrite(shmid));
	char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: El sistema lleva %lus encendido.\n", pid, info.uptime);
	writeSHM(shmid, msg);
	free(msg);
  return;
}

//Hace fork y escribe la información de la RAM del sistema
void RamInfo(int shmid, int pid)
{
	struct sysinfo info;
	sysinfo(&info);
	while(cantWrite(shmid));
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Disponibles %luMB de %luMB.\n", pid, (info.freeram)/1048576, (info.totalram)/1048576);
	writeSHM(shmid, msg);
	free(msg);
  return;
}

//Hace fork y escribe la información de Procesos del sistema
void ProcsInfo(int shmid, int pid)
{
	struct sysinfo info;
	sysinfo(&info);
	while(cantWrite(shmid));
  char *msg = (char *)malloc(sizeof(char)*256);
  sprintf(msg, "%d: Hay %d procesos en ejecucion.\n", pid, info.procs);
	writeSHM(shmid, msg);
	free(msg);
  return;
}

// Revisa si los procesos hijos pueden escribir en la memoria compartida
int cantWrite(int shmid)
{
	char *msg;
	if ((msg = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat en write\n");
	  return 1;
  }

  if (*msg == '1')
  {
  	return 1;
  }
  // aqui se activa el semaforo
	return 0;
}

// Revisa si el padre tiene mensajes para leer en la memoria compartida
int cantRead(int shmid)
{
	char *msg;
	if ((msg = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat\n");
	  return 1;
  }

  if (*msg == '0')
  {
  	return 1;
  }
  // aqui se activa el semaforo
  return 0;

}

void readSHM(int shmid)
{
	char *msg;
	if ((msg = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat\n");
	  exit(1);
  }
  printf("shm: %s\n", msg);
  FILE *p;
  p = fopen("log.txt","a+");

  *msg++ = '0';

  fprintf(p, "%s\n", msg);

  fclose(p);
  
}

void writeSHM(int shmid, char* string)
{
	char *msg;
	if ((msg = shmat(shmid, NULL, 0)) == (char *) -1)
  {
	  printf("Error: shmat\n");
	  exit(1);
  }

  *msg++ = '1';

  while(*string != '\0')
  {
  	*msg++ = *string++;
  }
  *msg = '\0';
}