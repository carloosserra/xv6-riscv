#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void print_file_content(char *filename) {
  int fd;
  char buf[512];
  int n;

  fd = open(filename, O_RDONLY);
  if(fd < 0){
    printf("Error: no se pudo abrir el archivo para lectura\n");
    return;
  }

  printf("Contenido del archivo '%s':\n", filename);
  while((n = read(fd, buf, sizeof(buf))) > 0) {
    write(1, buf, n);
  }
  close(fd);
  printf("\n");
}

int main(void) {
  int fd;
  char *filename = "testfile";

  // Creación del Archivo
  fd = open(filename, O_CREATE | O_RDWR);
  if(fd < 0){
    printf("Error: no se pudo crear el archivo\n");
    exit(1);
  }
  printf("Archivo creado con permisos de lectura/escritura\n");

  // Escritura Inicial
  if(write(fd, "Hola, mundo\n", 12) != 12){
    printf("Error: no se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  printf("Escritura inicial completada\n");
  close(fd);
  print_file_content(filename);

  // Cambio de Permisos a Solo Lectura
  if(chmod(filename, 1) < 0){
    printf("Error: no se pudo cambiar los permisos a solo lectura\n");
    exit(1);
  }
  printf("Permisos cambiados a solo lectura\n");

  // Prueba de Escritura con Solo Lectura
  fd = open(filename, O_WRONLY);

  if(write(fd, "Intento de escritura\n", 21) != 21){
    printf("Error: no se pudo escribir en el archivo\n");
  } else {
    printf("Error: se pudo escribir en el archivo cuando debería ser solo lectura\n");
  }
    close(fd);

  print_file_content(filename);

  // Cambio de Permisos de Vuelta a Lectura/Escritura
  if(chmod(filename, 3) < 0){
    printf("Error: no se pudo cambiar los permisos a lectura/escritura\n");
    exit(1);
  }
  printf("Permisos cambiados de vuelta a lectura/escritura\n");


  // Escritura Final
  fd = open(filename, O_RDWR);
  if(fd < 0){
    printf("Error: no se pudo abrir el archivo en modo lectura/escritura\n");
    exit(1);
  }
  if(write(fd, "Adiós, mundo\n", 13) != 13){
    printf("Error: no se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  printf("Escritura final completada\n");
  close(fd);
  print_file_content(filename);

 // Cambio de Permisos a Inmutable
  if(chmod(filename, 5) < 0){
    printf("Error: no se pudo cambiar los permisos a inmutable\n");
    exit(1);
  }
  printf("Permisos cambiados a inmutable\n");
  

  // Prueba de Escritura con Inmutable
  fd = open(filename, O_WRONLY);

  if(write(fd, "Intento de escritura\n", 21) != 21){
    printf("Error: no se pudo escribir en el archivo\n");
  } else {
    printf("Error: se pudo escribir en el archivo cuando debería ser inmutable\n");
  }
  close(fd);
 
 
  printf("Prueba de escritura con archivo inmutable completada (no se pudo abrir en modo escritura)\n");
  print_file_content(filename);

  // Intento de Cambio de Permisos de Vuelta a Lectura/Escritura
  if(chmod(filename, 3) < 0){
    printf("No se pudo cambiar los permisos de un archivo inmutable, como se esperaba\n");
  } else {
    printf("Error: se pudieron cambiar los permisos de un archivo inmutable\n");
    exit(1);
  }
  print_file_content(filename);

  printf("Prueba completada exitosamente\n");
  exit(0);
}