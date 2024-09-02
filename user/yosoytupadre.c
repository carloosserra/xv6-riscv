#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/types.h"

int main(void) {
  int ppid = getppid();
  fprintf(2, "ID del proceso padre: %d\n", ppid);
  
  exit(0);
}