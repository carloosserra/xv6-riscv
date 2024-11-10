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
    printf("Escribir en la dirección %p\n", addr);  // Verificar el valor
    char *ptr = addr;
    printf("dirección antes del intento: %p\n", ptr);
    printf("Valor antes del intento: %s\n", ptr);
    
    *ptr = 'A';  // Esto debería fallar si la protección es exitosa
    printf("Dirección despues: %p\n", ptr);  // Verificar el valor
    printf("Valor despues: %s\n", ptr);

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