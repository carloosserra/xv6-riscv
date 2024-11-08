# INFORME TAREA 2 CARLOS SERRA
## Documentación Tarea 3    

* ### syscall.h (23-24)
    Definimos la nuevas llamadas de sistema.```SYS_mprotect``` y ```SYS_munprotect```.
    ```
    #define SYS_mprotect 23
    #define SYS_munprotect 24
    ```
* ### syscall.c (104-105,131-132)
    Declaramos las funciones ```SYS_mprotect``` y ```SYS_munprotect```.
    ```
    extern uint64 sys_mprotect(void);
    extern uint64 sys_munprotect(void);

    static uint64 (*syscalls[])(void) = {
    ...
    [SYS_mprotect] sys_mprotect,
    [SYS_munprotect] sys_munprotect,
    };
    ```
* ### sysproc.c (94-116)
    Establecemos la lógica de las funciones.

    ```
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

* ### usys.pl (39-40)
    Agregamos las llamadas de sistemas para poder llamar a las funciones desde el espacio de usuario.
    ```
    entry("mprotect");
    entry("munprotect");
    ```
* ### user.h (25-26)
    Declaramos las funciones de usuario.

    ```
    int mprotect(void *addr, int len);
    int munprotect(void *addr, int len);
    ```
* ### vm.c
    