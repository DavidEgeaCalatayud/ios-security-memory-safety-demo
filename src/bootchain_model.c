#include <stdio.h>

#include "demo.h"

int estadoEsValido(Estado estado) {
    return estado == INTACTO || estado == COMPROMETIDO;
}

const char *estadoComoTexto(Estado estado) {
    switch (estado) {
        case INTACTO:
            return "INTACTO";
        case COMPROMETIDO:
            return "COMPROMETIDO";
        default:
            return "DESCONOCIDO";
    }
}

void imprimirEstado(const Dispositivo *d) {
    if (d == NULL) {
        printf("  Dispositivo .......... NO DISPONIBLE\n");
        return;
    }

    printf("  BootROM .............. %s\n", estadoComoTexto(d->bootrom));
    printf("  iBoot ................ %s\n", estadoComoTexto(d->iboot));
    printf("  Kernel iOS ........... %s\n", estadoComoTexto(d->kernel));
    printf("  Secure Enclave (SEP) . %s\n", estadoComoTexto(d->secure_enclave));
    printf("  Activation Lock ...... %s\n", estadoComoTexto(d->activation_lock));
}

void propagarCompromisoModelo(Dispositivo *disp) {
    if (disp == NULL) {
        return;
    }

    if (!estadoEsValido(disp->bootrom) ||
        !estadoEsValido(disp->iboot) ||
        !estadoEsValido(disp->kernel) ||
        !estadoEsValido(disp->secure_enclave) ||
        !estadoEsValido(disp->activation_lock)) {
        return;
    }

    if (disp->bootrom == COMPROMETIDO) {
        disp->iboot = COMPROMETIDO;
        disp->kernel = COMPROMETIDO;
    }
}

void demoPropagacionCompromiso(Dispositivo *disp) {
    printf("\n=== [2] PROPAGACION POR LA CADENA DE ARRANQUE ===\n");

    if (disp == NULL) {
        printf("Dispositivo no valido. No se puede propagar el compromiso.\n");
        return;
    }

    if (!estadoEsValido(disp->bootrom)) {
        printf("Estado de BootROM desconocido. No se propaga el compromiso.\n");
        return;
    }

    if (disp->bootrom == COMPROMETIDO) {
        printf("BootROM comprometida -> se puede cargar iBoot modificado.\n");
        printf("iBoot comprometido   -> se puede cargar kernel modificado.\n");
    }

    propagarCompromisoModelo(disp);

    printf("Secure Enclave: dominio independiente -> permanece %s.\n",
           estadoComoTexto(disp->secure_enclave));
    printf("Activation Lock: verificacion remota -> permanece %s.\n",
           estadoComoTexto(disp->activation_lock));
}

void capacidadesDelAtacante(const Dispositivo *disp) {
    printf("\n=== [3] ALCANCE REAL DEL COMPROMISO ===\n");

    if (disp == NULL) {
        printf("Dispositivo no valido. No se puede evaluar el alcance.\n");
        return;
    }

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
