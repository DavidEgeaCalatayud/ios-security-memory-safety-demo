#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "demo.h"

static void handlerLegitimo(const char *msg) {
    printf("  [LEGITIMO] Procesando comando: %s\n", msg);
}

static void handlerSecuestrado(const char *msg) {
    printf("  [SECUESTRADO] Flujo desviado. Mensaje: %s\n", msg);
}

static void inicializarPeticionLegitima(DFURequest *req) {
    strncpy(req->comando, "DFU_UPLOAD", sizeof(req->comando) - 1);
    req->comando[sizeof(req->comando) - 1] = '\0';
    req->longitud = 128;
    req->handler = handlerLegitimo;
}

static void inicializarPayloadControlado(DFURequest *req) {
    strncpy(req->comando, "datos_controlados", sizeof(req->comando) - 1);
    req->comando[sizeof(req->comando) - 1] = '\0';
    req->longitud = 0;
    req->handler = handlerSecuestrado;
}

void demoUseAfterFree(Dispositivo *disp) {
    printf("\n=== [1] DEMO: use-after-free con puntero a funcion ===\n");

    DFURequest *req = malloc(sizeof(DFURequest));
    if (!req) {
        perror("malloc");
        return;
    }

    inicializarPeticionLegitima(req);

    printf("Peticion legitima creada en direccion: %p\n", (void *)req);
    req->handler(req->comando);

    free(req);

    printf("Memoria liberada, pero el puntero antiguo sigue existiendo: %p\n",
           (void *)req);

    DFURequest *spray[MAX_SPRAY] = {0};
    int reutilizado = -1;

    for (int i = 0; i < MAX_SPRAY; i++) {
        spray[i] = malloc(sizeof(DFURequest));
        if (!spray[i]) {
            perror("malloc");
            break;
        }

        inicializarPayloadControlado(spray[i]);

        if (spray[i] == req && reutilizado == -1) {
            reutilizado = i;
        }
    }

    if (reutilizado >= 0) {
        printf("Heap grooming simulado: bloque reutilizado en spray[%d] (%p)\n",
               reutilizado,
               (void *)spray[reutilizado]);
        printf("El programa vuelve a usar el puntero antiguo:\n");

        req->handler(req->comando);
        if (disp != NULL) {
            disp->bootrom = COMPROMETIDO;
        }
    } else {
        printf("El allocator no reutilizo la direccion en esta ejecucion.\n");
        printf("La demo puede variar segun compilador, sistema y allocator.\n");
    }

    for (int i = 0; i < MAX_SPRAY; i++) {
        free(spray[i]);
    }
}

void demoCorregida(void) {
    printf("\n=== [7] VERSION CORREGIDA DEL USE-AFTER-FREE ===\n");

    DFURequest *req = malloc(sizeof(DFURequest));
    if (!req) {
        perror("malloc");
        return;
    }

    inicializarPeticionLegitima(req);

    printf("Peticion legitima creada en direccion: %p\n", (void *)req);
    req->handler(req->comando);

    free(req);
    req = NULL;

    if (req == NULL) {
        printf("El puntero ha sido invalidado correctamente.\n");
        printf("Se evita volver a usar memoria liberada.\n");
    }
}

void demoAddressSanitizerUseAfterFree(void) {
    printf("=== AddressSanitizer educational path ===\n");
    printf("Esta ruta provoca un acceso use-after-free minimo para que ASan lo detecte.\n");

    DFURequest *req = malloc(sizeof(DFURequest));
    if (!req) {
        perror("malloc");
        return;
    }

    inicializarPeticionLegitima(req);
    free(req);

    printf("Valor leido despues de free: %d\n", req->longitud);
}
