#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>
 /* crear subprocesos mediante fork
  * cada subproceso entregar informacion sobre estado del sistema y enviar resultados al padre mediante
  * memoria compartida
  * 
  * Registrar los resultados, PID del proceso que envia el mensaje y el contenido en el archivo log.txt
  * Cuando hijo termine escribir PID subproveso finalizado
  * CUando padre termine escribir PID Programa finalizado
  * 
  * Cada hijo analiza el sistema n veces, el usuario determina n.
  * Cada hijo toma el tiempo de espera entre cada analisis realizado
  * 
  * Padre creare 3 subprocesos que funcionen en paralelo
  *el primero de ellos entrega tiempo en segundos que lleva encendido el sistema
  * el segundo entrega cantidad de ram disponible sobre la total
  * tercero indica la cantidad de procesos en ejecucion
  * */
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

  struct sysinfo info;
  sysinfo(&info);

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
	else if(pid >0)
	{
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
		else if(pid >0)
		{
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
		} 
	}     
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
  printf("%d: Disponibles %lu de %lu.\n", getpid(), info.totalram, info.freeram);
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