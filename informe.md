# INFORME TAREA 4 CARLOS SERRA
## Introducción
 En esta tares se busca extender las capacidades del sistema operativo xv6 (RISC-V) mediante la implementación de un sistema básico de permisos de archivos. Este sistema permitirá definir permisos de solo lectura, lectura/escritura y un permiso especial para marcar archivos como inmutables, contribuyendo a la robustez y control de acceso en la gestión de archivos.

 ## Documentación Tarea 4
 ## Parte 1
 * ### `file.h` (31) y `fs.h` (39)
    Establecemos el campo `permissions` a la estructura `inode` para definir los permisos de acceso al archivo.
     * `file.h`

        ```c
        struct inode {
            ...
            
            int permissions;    // Permisos del archivo
        };
    * `fs.h`
        ```c
        struct dinode {               
            ...
            int permissions;    // Permisos del archivo
            char padding[BSIZE - (sizeof(short) * 4 + sizeof(uint) * (NDIRECT + 2) + sizeof(int))]; // Relleno para alinear a BSIZE
        };

 * ### `fs.c` (211)
    Establecemos el valor inicial para `permissions`de un 
    ```c
    struct inode*
    ialloc(uint dev, short type)
    {
     int inum;
     struct buf *bp;
     struct dinode *dip;
 
     for(inum = 1; inum < sb.ninodes; inum++){
        ...
        dip->permissions = 3; /
        ...
     }   
     printf("ialloc: no inodes\n");
     return 0;
    }
 * ### `sysfile.c` (325-350, 386-408)

    Modificamos la función `sys_open` para que verifique los permisos del inode antes de permitir la apertura del archivo
    
    ```c
    uint64
    sys_open(void)
    {
      ...
      } else {
         if((ip = namei(path)) == 0){
            end_op();
            return -1;
         }
         ilock(ip);
         // Verificar si el archivo es un directorio y si el modo es incompatible
         if(ip->type == T_DIR && omode != O_RDONLY){
            iunlockput(ip);
            end_op();
            return -1;
         }
         if((omode & O_WRONLY) && !(ip->permissions & 2)) {
            // Se solicita escritura, pero el archivo no tiene permisos de escritura
            iunlockput(ip);
            end_op();
            return -1;
         }
         if((omode & O_RDONLY) && !(ip->permissions & 1)) {
            // Se solicita lectura, pero el archivo no tiene permisos de lectura
            iunlockput(ip);
            end_op();
            return -1;
         }
      }

      ...
    }
    ```
    Seguido a eso agregamos la llamada a sistema `chmod` para manejar los permisos de un archivo.
    ```c
   uint64
   sys_chmod(void)
   {
      char path[MAXPATH];
      int mode;
      struct inode *ip;
      argint(1, &mode);
      if(argstr(0, path, MAXPATH) < 0 || mode < 0)
         return -1;

      begin_op();
      if((ip = namei(path)) == 0){
         end_op();
         return -1;
      }
      ilock(ip);
      ip->permissions = mode;
      iupdate(ip);
      iunlockput(ip);
      end_op();
      return 0;
   }
    ```
 * ### `syscall.h` (23)
    Declaramos la llamada de sistem
    ```c
    #define SYS_chmod  22
    ```
 * ### `syscall.c` (104,130)
    Añadimos la llamada de sistema
    ```c
    extern int sys_chmod(void);
    ...
    static int (*syscalls[])(void) = {
    ...
    [SYS_chmod]    sys_chmod,
    ...
    };

 * ### `user.h` (25)
    Añadimos la llamada de sistema
    ```c
    int chmod(char *path, int mode);
    ```
 * ### `usys.pl` (39)
    Añadimos la llamada de sistema
    ```
    entry("chmod");
 * ### `t4.c` 
   Creamos el archivo de pruebas para verificar que la función `chmod` cumple con la correcta asignación de los permisos de lectura y escritura en los archivos.
   ```c
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

   printf("Prueba completada exitosamente\n");
   exit(0);
   }
   ```
   Con esto, el kernel entrega los siguientes resultados, confirmando que la función `chmod` fue implementada correctamente. Al cambiar los permisos a solo lectura, cualquier intento de escribir en el archivo falla, como se esperaba. Al restaurar los permisos de lectura/escritura, el archivo puede ser escrito nuevamente sin problemas. Esto verifica que la función `chmod` bloquea y desbloquea correctamente los permisos de escritura del archivo.

   ```
   xv6 kernel is booting

   hart 1 starting
   hart 2 starting
   init: starting sh
   $ t4
   Archivo creado con permisos de lectura/escritura
   Escritura inicial completada
   Contenido del archivo 'testfile':
   Hola, mundo

   Permisos cambiados a solo lectura
   Error: no se pudo escribir en el archivo
   Contenido del archivo 'testfile':
   Hola, mundo

   Permisos cambiados de vuelta a lectura/escritura
   Escritura final completada
   Contenido del archivo 'testfile':
   Adiós, mundo
   Prueba completada exitosamente

## Parte 2
Ahora hemos agregado la opción de marcar un archivo como inmutable, lo que significa que no se puede cambiar su estado y solo tiene permiso de lectura. Este permiso especial, representado por el número 5, asegura que cualquier intento de modificar el archivo o cambiar sus permisos con `chmod` fallará permitiendo solo su lectura.
 *  ### `sysfile.c` (337-355, 380, 409-414)
      Cambiamos la funcion `sys_open` para que verifique en primer lugar si el archivo es inmutable o no, ademas de solo permitir la escritura para archivos que no sean inmutables. (337-355, 380)
      ```c
      ...
      if(ip->permissions == 5) {
         if(omode != O_RDONLY) { // Inmutable solo permite lectura
            iunlockput(ip);
            end_op();
            return -1;
         }
         } else { // Verificar permisos estándar
         if((omode & O_WRONLY) && !(ip->permissions & 2)) {
            iunlockput(ip);
            end_op();
            return -1;
         }
         if((omode & O_RDONLY) && !(ip->permissions & 1)) {
            iunlockput(ip);
            end_op();
            return -1;
         }
         }
         ...
         f->writable = (ip->permissions != 5) && ((omode & O_WRONLY) || (omode & O_RDWR)); // No escribible si es inmutable

      ```
      De igual forma, en la función `sys_chmod`, se ha implementado una verificación para determinar si el archivo es inmutable antes de intentar cambiar sus permisos. Si el archivo es inmutable, la función `sys_chmod` devuelve un error y no permite la modificación de los permisos. (409-414)
      ```c
      ...
         // Verificar si el archivo es inmutable
      if(ip->permissions == 5){
         iunlockput(ip);
         end_op();
         return -1; // No se puede cambiar permisos de un archivo inmutable
      }
      ```
   * ### `t4.c`
      Ahora actualizamos el programa de prueba complementando el código anterior para incluir la inmutabilidad del archivo. Después de la escritura final, se cambia el permiso del archivo a inmutable usando `chmod(filename, 5)`. Luego, se intenta escribir en el archivo y cambiar sus permisos nuevamente para confirmar que la inmutabilidad se ha implementado correctamente. 

      ```c
      ...
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
      ```
      El programa verifica que no es posible escribir en un archivo inmutable ni cambiar sus permisos, asegurando que los archivos marcados como inmutables mantengan su estado de solo lectura y no puedan ser alterados. entregando lo siguiente:
      ```
      xv6 kernel is booting

      hart 1 starting
      hart 2 starting
      init: starting sh
      $ t4
      Archivo creado con permisos de lectura/escritura
      Escritura inicial completada
      Contenido del archivo 'testfile':
      Hola, mundo

      Permisos cambiados a solo lectura
      Error: no se pudo escribir en el archivo
      Contenido del archivo 'testfile':
      Hola, mundo

      Permisos cambiados de vuelta a lectura/escritura
      Escritura final completada
      Contenido del archivo 'testfile':
      Adiós, mundo
      Permisos cambiados a inmutable
      Error: no se pudo escribir en el archivo
      Prueba de escritura con archivo inmutable completada (no se pudo abrir en modo escritura)
      Contenido del archivo 'testfile':
      Adiós, mundo
      No se pudo cambiar los permisos de un archivo inmutable, como se esperaba
      Contenido del archivo 'testfile':
      Adiós, mundo
      Prueba completada exitosamente
 
## Dificultades encontradas y soluciones implementadas.

* La principal dificultad encontrada fue entender el fucionamiento del manejo de archivos en el sistema operativo ya que hay varias funciones que parecieran ser necesarias de editar, sin embargo no son realmente necesarias o directamente tiene otro uso, pero con su respectiva investigación y prueba/error, se logró obterner con exito lo pedido para esta tarea.

