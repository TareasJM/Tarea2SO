#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

//Tamaño y nombre de memoria compartida
#define SHMSZ 368 // 2 ints + 6 strings de 60 chars c/u
#define SHMK 1234 // nombre random

void upTime();
void RamInfo();
void ProcsInfo();
 
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

  // pide la memoria compartida
  if ((shmid = shmget(SHMK, SHMSZ, IPC_CREAT | 0666)) < 0)
  {
	  printf("Error: shmget\n");
	  return 1;
  }
  
  // analiza el sistema rep veces
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
		  upTime();		
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
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
		  RamInfo();
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
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
		  ProcsInfo();
		  // espera wait segundos
			if(r > 0)
				sleep(wait);
		}
	  printf("%d: SubProceso finalizado.\n", getpid());
		exit(0); 
	}     

  // fin del proceso padre
  sleep(wait*rep);
  printf("%d: Programa finalizado\n", getpid());

  return 0;

}

//Hace fork y escribe el tiempo del sistema
void upTime()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: El sistema lleva %lus encendido.\n", getpid(), info.uptime);
  return;
}

//Hace fork y escribe la información de la RAM del sistema
void RamInfo()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: Disponibles %luMB de %luMB.\n", getpid(), (info.freeram)/1048576, (info.totalram)/1048576);
  return;
}

//Hace fork y escribe la información de Procesos del sistema
void ProcsInfo()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: Hay %d procesos en ejecucion.\n", getpid(), info.procs);
  return;
}

void writeSHM(char* string)
{

}