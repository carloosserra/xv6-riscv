# INFORME TAREA 2 CARLOS SERRA
## Introducción
La protección de memoria es un mecanismo esencial en los sistemas operativos modernos que evita accesos indebidos a áreas críticas de memoria, asegurando así la estabilidad y seguridad del sistema. Mediante el uso de tablas de páginas, el sistema operativo puede definir permisos como solo lectura, evitando que los procesos alteren datos no autorizados o comprometan la seguridad.
## Contexto Tarea
En este trabajo, se modificarán las funcionalidades de xv6 para implementar un sistema de protección de memoria que permita marcar ciertas regiones de memoria como de solo lectura. Se crearán las funciones `mprotect` y `munprotect` para gestionar estos permisos en la tabla de páginas del proceso, aplicando y retirando la restricción de escritura según sea necesario.
## Documentación Tarea 3    

* ### `syscall.h` (23-24)
    Definimos la nuevas llamadas de sistema.`SYS_mprotect` y `SYS_munprotect`.

    ```c
    #define SYS_mprotect 23
    #define SYS_munprotect 24
    
* ### `syscall.c` (104-105,131-132)
    Declaramos las funciones ```SYS_mprotect``` y ```SYS_munprotect```.

    ```c
    extern uint64 sys_mprotect(void);
    extern uint64 sys_munprotect(void);

    static uint64 (*syscalls[])(void) = {
    ...
    [SYS_mprotect] sys_mprotect,
    [SYS_munprotect] sys_munprotect,
    };
    ```
* ### `sysproc.c` (94-116)
    Establecemos la lógica de las funciones.

    ```c
    uint64
    sys_mprotect(void)
    {
        uint64 addr;
        int len;
        argaddr(0, &addr);
        argint(1, &len);
        if(addr < 0 || len < 0)
            return -1;
        return mprotect(addr, len);
    }

    uint64
    sys_munprotect(void)
    {
        uint64 addr;
        int len;
        argaddr(0, &addr);
        argint(1, &len);
        if(addr < 0 || len < 0)
            return -1;
        return munprotect(addr, len);
    }
    ```

* ### `usys.pl` (39-40)
    Agregamos las llamadas de sistemas para poder llamar a las funciones desde el espacio de usuario.
    ```c
    entry("mprotect");
    entry("munprotect");
    ```
* ### `user.h` (25-26)
    Declaramos las funciones de usuario.

    ```c
    int mprotect(void *addr, int len);
    int munprotect(void *addr, int len);
    ```
* ### `vm.c` (8-81)
    Creamos las funciones que modifican los permisos  de escritura de las paginas de memoria de un proceso siendo `mprotect` para bloquear memoria y `munprotect` para desbloquear memoria.

    ### Funcionamiento 
    - ### Validación de Parámetros:

        Ambas funciones verifican que la dirección (`addr`) esté alineada a una página y que la longitud (`len`) sea positiva.
        Comprueban que la dirección y la longitud especificadas estén dentro del espacio de direcciones del proceso.

    
    - ### Modificación de la Tabla de Páginas:

        Recorren cada página en la región especificada.
        Utilizan la función `walk` para obtener la entrada de la tabla de páginas (PTE) correspondiente a cada dirección.
        Verifican que la página pertenezca al espacio de usuario.
        `mprotect` deshabilita el bit de escritura (W) en la PTE para marcar la página como solo lectura.
        `munprotect` habilita el bit de escritura (W) en la PTE para permitir la escritura en la página.

    - ### Actualización de la TLB:

        Ambas funciones llaman a `sfence_vma` para asegurar que los cambios en los permisos de la memoria se reflejen correctamente en la TLB (Translation Lookaside Buffer)

    ```c
    ...

    #include "spinlock.h" 
    #include "proc.h"

    int
    mprotect(uint64 addr, int len)
    {
        struct proc *p = myproc();
        pte_t *pte;
        uint64 a;

        // Verificar que la dirección esté alineada a una página y que la longitud sea positiva
        if (addr % PGSIZE != 0 || len <= 0) {
            printf("mprotect: invalid address or length\n");
            return -1;
        }

        // Verificar que la dirección esté dentro del espacio de direcciones del proceso
        if (addr + len * PGSIZE > p->sz) {
            printf("mprotect: address out of bounds\n");
            return -1;
        }

        for(a = PGROUNDDOWN(addr); a < addr + len * PGSIZE; a += PGSIZE){
            if((pte = walk(p->pagetable, a, 0)) == 0) {
                printf("mprotect: walk failed for address 0x%lx\n", a);
                return -1;
            }
            if(!(*pte & PTE_U)) { // Verificar que la página pertenece al usuario
                printf("mprotect: page not user-accessible for address 0x%lx\n", a);
                return -1;
            }
            *pte &= ~PTE_W; // Deshabilitar el bit de escritura
            printf("mprotect: write bit disabled for address 0x%lx\n", a);
        }
        
        sfence_vma();
        return 0;
    }

    int
    munprotect(uint64 addr, int len)
    {
        struct proc *p = myproc();
        pte_t *pte;
        uint64 a;

        // Verificar que la dirección esté alineada a una página y que la longitud sea positiva
        if (addr % PGSIZE != 0 || len <= 0) {
            printf("munprotect: invalid address or length\n");
            return -1;
        }

        // Verificar que la dirección esté dentro del espacio de direcciones del proceso
        if (addr + len * PGSIZE > p->sz) {
            printf("munprotect: address out of bounds\n");
            return -1;
        }

        for(a = PGROUNDDOWN(addr); a < addr + len * PGSIZE; a += PGSIZE){
            if((pte = walk(p->pagetable, a, 0)) == 0) {
                printf("munprotect: walk failed for address 0x%lx\n", a);
                return -1;
            }
            if(!(*pte & PTE_U)) { // Verificar que la página pertenece al usuario
                printf("munprotect: page not user-accessible for address 0x%lx\n", a);
                return -1;
            }
            *pte |= PTE_W; // Habilitar el bit de escritura
            printf("munprotect: write bit enabled for address 0x%lx\n", a);
        }
        
        sfence_vma();
        return 0;
    }
    ...
* ### `trap.c` (70- 76)
   Como xv6 no está preparado para el bloqueo de espacios de memoria, aunque lo anterior esté creado correctamente, el sistema soltará el siguiente error cuando se intente actualizar un espacio de memoria bloqueado.
    ```
    hart 2 starting
    hart 1 starting
    init: starting sh
    $ t3
    usertrap(): unexpected scause 0xf pid=3
                sepc=0x2a stval=0x4000
    ```
    Para solucionar esto y que se siga ejecutando el programa es necesario editar el archivo `trap.c`.
    ```c
    ...
    } else if(r_scause() == 0xf) {
     // Handle store/AMO page fault
    
    printf("usertrap: Store/AMO page fault at address 0x%lx\n", r_stval());

    // No terminar el proceso, solo registrar el fallo y continuar
    p->trapframe->epc += 4;
   } else {
    ...
    ```
    Con el código anterior, se notificará el error en la consola, pero a diferencia de antes, la ejecución del programa continuará sin interrupciones.

* ### `t3.c`
    Ya que las funciones `mprotect` y `munprotect` para el bloqueo y desbloqueo de memoria han sido creadas e implementadas correctamente, se creará el archivo `t3.c` en la carpeta `user` para generar un programa de pruebas que verificará si las funciones se implementarioncorrectamente.
    
    Primero en el archivo `Makefile` agregamos (142):

    ```c
    UPROGS=\
    ...
    $U/_t3\
    ```
    Para luego en el archivo `t3.c` crear lo siguiente:
    ```c
    #include "kernel/types.h"
    #include "kernel/stat.h"
    #include "user/user.h"

    int main(void) {
        char *addr = sbrk(0);  // Obtener la dirección actual del heap
        printf("Dirección actual del heap: %p\n", addr);
        sbrk(4096);  // Reservar una página
        // Intentar proteger la nueva página
        if (mprotect(addr, 1) == -1) {
            printf("mprotect falló\n");
            exit(1);
        }
        
        // Intentar escribir en la página protegida
        printf("Escribir en la dirección %p\n", addr); // Verificar el valor
        char *ptr = addr;
        printf("dirección antes del intento: %p\n", ptr); // Verificar la direccion
        printf("Valor antes del intento: %s\n", ptr); // Verificar el valor
        
        *ptr = 'A';  // Esto debería fallar si la protección es exitosa
        printf("Dirección despues: %p\n", ptr);  // Verificar la direccion
        printf("Valor despues: %s\n", ptr); // Verificar el valor

        // Intentar desproteger la página
        if (munprotect(addr, 1) == -1) {
            printf("munprotect falló\n");
            exit(1);
        }
        
        // Intentar escribir en la página desprotegida
        printf("Escribiendo en la dirección después de desproteger\n");
        *ptr = 'B';  // Esto debería tener éxito si la desprotección es exitosa
        printf("Dirección despues: %p\n", ptr);  // Verificar la direccion
        printf("Valor despues: %s\n", ptr); // Verificar el valor
        
        exit(0);
    }
    ```
    Para este programa, se hizo uso de varios `printf` para monitorear constantemente la dirección y el valor de `ptr` antes y después de bloquear el espacio de memoria, así como después de desbloquearlo nuevamente. Esto permitió confirmar que las funciones `mprotect` y `munprotect` funcionan correctamente. Los `printf` se utilizaron para verificar que la protección de solo lectura se aplicara adecuadamente y que, al intentar escribir en una página protegida, se generara un fallo de página. Posteriormente, se verificó que al desproteger la página, la escritura se realizara con éxito. Entregando lo siguiente
    
    ```
    xv6 kernel is booting

    hart 2 starting
    hart 1 starting
    init: starting sh
    $ t3
    Dirección actual del heap: 0x0000000000004000 
    mprotect: write bit disabled for address 0x4000 // respuesta de la funcion mprotect
    Escribir en la dirección 0x0000000000004000
    dirección antes del intento: 0x0000000000004000 
    Valor antes del intento: 
    usertrap: Store/AMO page fault at address 0x4000 // respuesta del archivo trap.c
    Dirección despues: 0x0000000000004000
    Valor despues: // valor sin cambios
    munprotect: write bit enabled for address 0x4000 // respuesta de la función munprotect
    Escribiendo en la dirección después de desproteger
    Dirección despues: 0x0000000000004000
    Valor despues: B // valor cambiado correctamente
    ```
   Con lo entregado por la consola se puede confirmar que el valor inicial de `ptr` era vacío o nulo. Después de intentar actualizar el valor de `ptr` tras aplicar `mprotect` a la dirección de memoria de `ptr`, se observa que el valor no cambia y permanece vacío o nulo. Esto indica que la protección de solo lectura se aplicó correctamente, impidiendo la escritura en la memoria protegida.

    En contraste, al utilizar la función `munprotect` para desproteger la misma dirección de memoria y luego intentar cambiar el valor de `ptr`, se puede ver que el valor se actualiza correctamente a `B`. Esto confirma que la función `munprotect` revierte adecuadamente la protección de solo lectura, permitiendo la escritura en la memoria previamente protegida.

    Por lo tanto, el uso de múltiples `printf` permitió monitorear constantemente la dirección y el valor de `ptr` antes y después de bloquear el espacio de memoria, así como después de desbloquearlo nuevamente. Esto confirmó que las funciones `mprotect` y `munprotect` funcionan correctamente, ya que la protección de solo lectura se aplicó adecuadamente y, al intentar escribir en una página protegida, se generó un fallo de página. Posteriormente, al desproteger la página, la escritura se realizó con éxito. 

## Dificultades encontradas y soluciones implementadas
- La principal dificultad encontrada fue el manejo de errores y su origen. Inicialmente, no se tenía conocimiento de la causa de los errores ni de su significado. Sin embargo, tras una investigación exhaustiva, se identificó que el archivo `trap.c` era el responsable de no permitir la ejecución del programa de pruebas al intentar modificar una dirección de memoria bloqueada. Posteriormente, se realizaron las modificaciones necesarias en este archivo para permitir que el programa continuara ejecutándose a pesar de los intentos de escritura en memoria protegida.

- Otra dificultad fue identificar qué archivos debían ser editados. Por primera vez, se tuvo que modificar el archivo `vm.c`. Nuevamente, mediante una investigación detallada en la documentación, se llegó a la conclusión de que este archivo debía ser modificado. Se realizaron las modificaciones pertinentes en `vm.c` para implementar correctamente las funciones de protección y desprotección de memoria (`mprotect` y `munprotect`), permitiendo así llevar a cabo la tarea de manera efectiva.

- Por último, establecer la lógica de las funciones `mprotect` y `munprotect` fue un desafío, ya que no se había realizado algo similar en entregas anteriores. Sin embargo, a través de un proceso de prueba y error, junto con una investigación exhaustiva, se logró concretar efectivamente la lógica y el funcionamiento correcto de ambas funciones. Este proceso implicó comprender cómo modificar los permisos de las páginas de memoria y asegurarse de que los cambios se reflejaran correctamente en la TLB. Finalmente, se verificó que las funciones funcionaran como se esperaba mediante un programa de pruebas detallado.


