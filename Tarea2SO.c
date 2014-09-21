#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>

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
  
  // analiza el sistema rep veces
  while(rep-- > 0)
  {
  	// primer fork
		pid_t pid = fork();
		if(pid == -1)
		{
		  printf("Error al crear proceso hijo \n");
			printf("%s", strerror(errno));
			return 0;
		}
		if(pid == 0)
		{
		  upTime();
			return 0;
		}

  	// segundo fork
		pid = fork();
		if(pid == -1)
		{
		  printf("Error al crear proceso hijo \n");
			printf("%s", strerror(errno));
			return 0;
		}
		if(pid == 0)
		{
		  RamInfo();
			return 0;
		}

  	// tercer fork
		pid = fork();
		if(pid == -1)
		{
		  printf("Error al crear proceso hijo \n");
			printf("%s", strerror(errno));
			return 0;
		}
		if(pid == 0)
		{
		  ProcsInfo();
			return 0;
		}     

		// espera wait segundos
		sleep(wait);
		// salto de linea para mejor lectura
		printf("\n");
  }
  
  // fin del proceso padre
  printf("%d:Programa finalizado\n", getpid());

}

//Hace fork y escribe el tiempo del sistema
void upTime()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: El sistema lleva %lus encendido.\n", getpid(), info.uptime);
  printf("%d: SubProceso finalizado.\n", getpid());
  return;
}

//Hace fork y escribe la información de la RAM del sistema
void RamInfo()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: Disponibles %luMB de %luMB.\n", getpid(), (info.freeram)/1048576, (info.totalram)/1048576);
  printf("%d: SubProceso finalizado.\n", getpid());
  return;
}

//Hace fork y escribe la información de Procesos del sistema
void ProcsInfo()
{
	struct sysinfo info;
	sysinfo(&info);
  printf("%d: Hay %d procesos en ejecucion.\n", getpid(), info.procs);
  printf("%d: SubProceso finalizado.\n", getpid());
  return;
}