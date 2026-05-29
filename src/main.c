#include <stdio.h>
#include <string.h>

#include "demo.h"

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--asan-trigger") == 0) {
        demoAddressSanitizerUseAfterFree();
        return 0;
    }

    Dispositivo disp = {
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO
    };

    printf("Estado inicial del dispositivo:\n");
    imprimirEstado(&disp);

    demoUseAfterFree(&disp);
    propagarCompromiso(&disp);

    printf("\nEstado final del dispositivo:\n");
    imprimirEstado(&disp);

    capacidadesDelAtacante(&disp);
    demoActivationLockRemoto(&disp);
    demoVulnerabilidadesActivationLock(&disp);
    demoCorregida();

    printf("\nConclusion general:\n");
    printf("  Una vulnerabilidad maxima en una capa local no implica necesariamente\n");
    printf("  compromiso total del dispositivo. checkm8 compromete la cadena de\n");
    printf("  arranque local porque afecta a la BootROM, que es la raiz de confianza\n");
    printf("  del procesador principal. Sin embargo, eso no equivale automaticamente\n");
    printf("  a romper Secure Enclave, descifrar datos protegidos ni retirar\n");
    printf("  Activation Lock de forma legitima.\n");

    printf("\n  El analisis de todas las vulnerabilidades debe distinguir entre:\n");
    printf("    - fallos locales de memoria;\n");
    printf("    - fallos de cadena de arranque;\n");
    printf("    - fallos criptograficos;\n");
    printf("    - fallos de servidor;\n");
    printf("    - robo de credenciales;\n");
    printf("    - ingenieria social.\n");

    return 0;
}
