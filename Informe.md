# INFORME TAREA 2 CARLOS SERRA
## Documentación Tarea 2

### 1. Investigación preliminar
Se investigó la ubicación del ```scheduler``` dentro del sistema operativo y finalmente se encontró en la carpeta ```kernel```, específicamente en el archivo ```proc.c```. Dentro de este archivo, el ```scheduler``` está implementado en un bloque de código llamado ```scheduler(void)```.

### 2. Incorporar Prioridad y Boost
En una segunda etapa de investigación para implementar la funcionalidad de **Prioridad** y **Boost**, se identificó la necesidad de modificar la estructura ```proc``` en el archivo ```proc.h```. Esta modificación permitirá incorporar ambos campos a la estructura de cada proceso, facilitando su manejo dentro del sistema operativo. (107-108)
 
     
    struct proc {
    ...
    int priority; // Campo de prioridad
    int boost;    // Campo de boost
    };
    
Seguido a esto, en el archivo ```proc.c```, dentro de la función ```allocproc```, se establecen los valores iniciales de ambos campos para cada proceso recién creado. (127-130)
    
    static struct proc*
    allocproc(void)
    {
        ...
    found:
        ...
        // Inicializar prioridad y boost
        p->priority = 0; // Inicializar prioridad en 0 
        p->boost = 1;    // Inicializar boost en 1
        ...
    };

    

### 3. Implementar Lógica de Prioridad con Boost
Posteriormente en el archivo `proc.c` , se estableció la lógica de **Prioridad** y **Boost** en la función `scheduler(void)`, en donde inicialmente establecemos la variable  `high_p` para guardar el proceso con mayor prioridad. para luego implementar la lógica `Prioridad += Boost` y si la `Prioridad = 9` entonces `Boost = -1` y si `Prioridad = 0`, entonces `Boost = -1`. (451-499)
    
    void
        scheduler(void)
        {
        struct proc *p;
        struct cpu *c = mycpu();

        c->proc = 0;
        for(;;){
            // The most recent process to run may have had interrupts
            // turned off; enable them to avoid a deadlock if all
            // processes are waiting.
            intr_on();

            int found = 0;
            for(p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if(p->state == RUNNABLE) {
                // Actualizar la prioridad y el boost de los procesos RUNNABLE
                p->priority += p->boost;

                // Cambiar el boost según la prioridad
                if (p->priority >= 9) {
                p->boost = -1; // Disminuir la prioridad si alcanza 9
                }
                if (p->priority <= 0) {
                p->boost = 1;  // Aumentar la prioridad si alcanza 0
                }

                // Switch to chosen process.  It is the process's job
                // to release its lock and then reacquire it
                // before jumping back to us.
                p->state = RUNNING;
                c->proc = p;
                swtch(&c->context, &p->context);

                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
                found = 1;
            }
            release(&p->lock);
            }

            if(found == 0) {
            // nothing to run; stop running on this core until an interrupt.
            intr_on();
            asm volatile("wfi");
            }
        }
    }
    
   
### 4.  Programa de prueba
Para verificar la **prioridad** y el **boost** implementados, se añadieron dos funciones: `getpriority` y `getboost`. Estas funciones permiten visualizar tanto la **prioridad** como el valor del **boost** de cada proceso. A continuación, se detallan los cambios realizados para incluir estas funcionalidades.
   
* ### sysproc.c (38-58)
        
        uint64
        sys_getpriority(void)
        {
        int pid;
        argint(0, &pid);
        if(pid < 0)
            return -1;
        return getpriority(pid);
        }

        // sys_getboost
        uint64
        sys_getboost(void)
        {
        int pid;
        argint(0, &pid);
        if(pid < 0)
            return -1;
        return getboost(pid);
        }
        
* ### proc.c (538-556)
        
        int getpriority(int pid) {
            struct proc *p;
            for(p = proc; p < &proc[NPROC]; p++) {
                if(p->pid == pid) {
                return p->priority;
                }
            }
            return -1; // Si no se encuentra el proceso
        }

        int getboost(int pid) {
            struct proc *p;
            for(p = proc; p < &proc[NPROC]; p++) {
                if(p->pid == pid) {
                return p->boost;
                }
            }
            return -1; // Si no se encuentra el proceso
        }
        
* ### syscall. (23-24)
        
        ...
        #define SYS_getpriority 22
        #define SYS_getboost 23
        
* ### syscall.c (104-105, 132-133)
       
        ...
        extern uint64 sys_getpriority(void);
        extern uint64 sys_getboost(void);

        static uint64 (*syscalls[])(void) = {
        ...
        [SYS_getpriority] sys_getpriority,
        [SYS_getboost] sys_getboost,

        };
        

        
* ### usys.pl (39-40)
        
        ...
        entry("getpriority");
        entry("getboost");
        ```
* ### user.h (44-45)
        
        ...
        int getpriority(int pid);
        int getboost(int pid);
        
Luego en la carpeta `/user` se crea el archivo para crear el programa de pruebas `test_T2.c` con el codigo:
    
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

Ahora en el archivo `Makefile` (142) se agrega lo siguiente:

    UPROGS=\
    ...
    $U/_test_T2\
    
Finalmente corremos ```XV6```.
    
    make clean
    make qemu
    test_T2
    
Entregando los siguiente valores:

    xv6 kernel is booting

    hart 1 starting
    hart 2 starting
    init: starting sh
    $ test_T2
    Ejecutando proceso hijo 0 con PID 4, Prioridad 1, Boost 1
    Ejecutando proceso hijo 1 con PID 5, Prioridad 2, Boost 1
    Ejecutando proceso hijo 2 con PID 6, Prioridad 3, Boost 1
    Ejecutando proceso hijo 3 con PID 7, Prioridad 4, Boost 1
    Ejecutando proceso hijo 4 con PID 8, Prioridad 5, Boost 1
    Ejecutando proceso hijo 5 con PID 9, Prioridad 6, Boost 1
    Ejecutando proceso hijo 6 con PID 10, Prioridad 7, Boost 1
    Ejecutando proceso hijo 7 con PID 11, Prioridad 8, Boost 1
    Ejecutando proceso hijo 8 con PID 12, Prioridad 9, Boost -1
    Ejecutando proceso hijo 9 con PID 13, Prioridad 8, Boost -1
    Ejecutando proceso hijo 10 con PID 14, Prioridad 7, Boost -1
    Ejecutando proceso hijo 11 con PID 15, Prioridad 6, Boost -1
    Ejecutando proceso hijo 12 con PID 16, Prioridad 5, Boost -1
    Ejecutando proceso hijo 13 con PID 17, Prioridad 4, Boost -1
    Ejecutando proceso hijo 14 con PID 18, Prioridad 3, Boost -1
    Ejecutando proceso hijo 15 con PID 19, Prioridad 2, Boost -1
    Ejecutando proceso hijo 16 con PID 20, Prioridad 1, Boost -1
    Ejecutando proceso hijo 17 con PID 21, Prioridad 0, Boost 1
    Ejecutando proceso hijo 18 con PID 22, Prioridad 1, Boost 1
    Ejecutando proceso hijo 19 con PID 23, Prioridad 2, Boost 1
    
### 5. Dificultades encontradas y soluciones implementadas.
*   La principal dificultad radicó en comprender el funcionamiento interno del sistema operativo, específicamente en identificar los archivos y funciones que gestionan la organización de procesos. Esto implicó un análisis detallado de la documentación oficial del sistema, además de realizar búsquedas adicionales en diversas fuentes en internet. Fue necesario entender cómo interactúan los distintos componentes, como las estructuras de datos y las funciones encargadas de la planificación y manejo de los procesos, para poder implementar correctamente las modificaciones requeridas.

*   Otra dificultad fue desarrollar la lógica para implementar el manejo de **PrioridaFinalmente, la implementacion de funciones y de el programa de prueba no fue un proceso que genere dificultad ya que es un proceso que se hizo en la tarea anterior, por lo que fue relativamente sensilla la implementacion de estos.d** y **Boost** según lo propuesto en esta tarea. En muchas ocasiones, al ejecutar el programa de pruebas, el sistema no respondía adecuadamente o generaba una gran cantidad de caracteres incoherentes. Para abordar este problema, se realizaron diversas pruebas en las que se introdujeron pequeñas modificaciones al código original del **scheduler**, lo que redujo los errores. Además, en el programa de pruebas, se incorporó la instrucción ```sleep(i)```, que ayudó a disminuir aún más los fallos al ejecutar el código, mejorando la estabilidad del sistema durante las pruebas.

*   La última dificultad fue cómo visualizar o verificar los cambios en los valores de **Boost** y **Prioridad** para confirmar si la lógica implementada en el **scheduler** funcionaba correctamente. Para resolver esto, se crearon dos funciones: `getpriority` y `getboost`, que permiten obtener y mostrar en consola estos valores para cada proceso. Esto facilitó la verificación visual del correcto funcionamiento de la lógica solicitada en la tarea. ultima dificultad encontrada fue saber o de algun modo visualizar los cambio de **boost** y **prioridad** para de alguna forma saber si se hizo bien la logica implementada en en **scheduler**, y para eso se hicieron dos funciones `getprority` y `getboost` para obtener y mostrar en consola ambos datos para cada proceso y de esta forma verificar vizulamente el funcionamiento correcto de lo que se pidio en la tarea

*   Finalmente, la implementación de las  `getpriority`, `getboost` y del programa de prueba `test_T2` no representó una gran dificultad, ya que este proceso fue realizado en la tarea anterior. Por lo tanto, la implementación resultó ser relativamente sencilla en esta ocasión.
