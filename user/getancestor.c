#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(2, "Usage: getancestor <n>\n");
    exit(1);
  }

  int n = atoi(argv[1]); // Convertir el argumento de la línea de comandos a un entero
  int ancestor_pid = getancestor(n);

  if (ancestor_pid == -1) {
    printf("No hay tantos ancestros\n");
  } else {
    printf("El ancestro %d tiene PID %d\n", n, ancestor_pid);
  }

  exit(0);
}