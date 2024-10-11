#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROCESSES 20


void create_processes() {
  for (int i = 0; i < NUM_PROCESSES; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("Error: fork failed\n");
      exit(1);
    } else if (pid == 0) {
      // Código del proceso hijo
      sleep(i); // Esperar un momento antes de imprimir
      int priority = getpriority(getpid());
      int boost = getboost(getpid());

      printf("Ejecutando proceso hijo %d con PID %d, Prioridad %d, Boost %d\n", i, getpid(), priority, boost);
      // Detener el proceso unos segundos
      sleep(10);
      exit(0);
      
    }
  }

  // Esperar a que todos los procesos hijos terminen
  for (int i = 0; i < NUM_PROCESSES; i++) {
    wait(0);
  }
}

int main(int argc, char *argv[]) {
  create_processes();
  
  exit(0);
}