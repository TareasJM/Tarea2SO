CC			= gcc
CFLAGS	= -Wall -lm


all: Tarea2SO

Tarea2SO: Tarea2SO.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f *o Tarea2SO