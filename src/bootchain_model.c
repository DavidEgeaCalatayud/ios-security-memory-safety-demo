#include <stdio.h>

#include "demo.h"

void imprimirEstado(const Dispositivo *d) {
    const char *etiqueta[] = { "INTACTO", "COMPROMETIDO" };

    printf("  BootROM .............. %s\n", etiqueta[d->bootrom]);
    printf("  iBoot ................ %s\n", etiqueta[d->iboot]);
    printf("  Kernel iOS ........... %s\n", etiqueta[d->kernel]);
    printf("  Secure Enclave (SEP) . %s\n", etiqueta[d->secure_enclave]);
    printf("  Activation Lock ...... %s\n", etiqueta[d->activation_lock]);
}

void propagarCompromiso(Dispositivo *disp) {
    printf("\n=== [2] PROPAGACION POR LA CADENA DE ARRANQUE ===\n");

    if (disp->bootrom == COMPROMETIDO) {
        printf("BootROM comprometida -> se puede cargar iBoot modificado.\n");
        disp->iboot = COMPROMETIDO;

        printf("iBoot comprometido   -> se puede cargar kernel modificado.\n");
        disp->kernel = COMPROMETIDO;
    }

    printf("Secure Enclave: dominio independiente -> permanece INTACTO.\n");
    printf("Activation Lock: verificacion remota -> permanece INTACTO.\n");
}

void capacidadesDelAtacante(const Dispositivo *disp) {
    printf("\n=== [3] ALCANCE REAL DEL COMPROMISO ===\n");

    printf("Con BootROM/iBoot/Kernel comprometidos, el atacante PUEDE:\n");
    printf("  - Ejecutar codigo arbitrario en el procesador principal.\n");
    printf("  - Cargar software no firmado en el entorno local.\n");
    printf("  - Alterar restricciones locales del sistema operativo.\n");
    printf("  - Instalar herramientas fuera del modelo normal de iOS.\n");

    printf("\nLo que el atacante NO puede hacer automaticamente:\n");

    if (disp->secure_enclave == INTACTO) {
        printf("  - Extraer claves protegidas por Secure Enclave.\n");
        printf("  - Obtener automaticamente datos cifrados protegidos por la clave del usuario.\n");
        printf("  - Romper directamente protecciones biometricas gestionadas por SEP.\n");
    }

    if (disp->activation_lock == INTACTO) {
        printf("  - Eliminar legitimamente Activation Lock.\n");
        printf("  - Generar una autorizacion valida del servidor de Apple.\n");
        printf("  - Convertir automaticamente un dispositivo bloqueado en reutilizable.\n");
    }
}
