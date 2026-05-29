#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SPRAY 64

/*
 * ============================================================
 *  DEMOSTRACION ACADEMICA
 *  Use-after-free + modelo de capas de seguridad estilo iOS
 * ============================================================
 *
 *  Este programa NO reproduce checkm8.
 *  No interactua con USB, DFU, ni ningun dispositivo real.
 *  No realiza, ni simula realizar, ningun bypass real de Activation Lock.
 *
 *  Objetivo:
 *   1. Mostrar el patron use-after-free con puntero a funcion.
 *   2. Modelar conceptualmente la cadena de arranque:
 *      BootROM -> iBoot -> Kernel.
 *   3. Explicar por que comprometer la BootROM rompe la raiz
 *      de confianza local.
 *   4. Mostrar por que Secure Enclave y Activation Lock no caen
 *      automaticamente por un jailbreak/checkm8.
 *   5. Analizar distintas vulnerabilidades relacionadas con
 *      Activation Lock sin implementar ningun bypass real.
 * ============================================================
 */

typedef void (*DFUHandler)(const char *msg);

/* ---------- Estructura vulnerable estilo DFU ---------- */

typedef struct {
    char       comando[32];
    int        longitud;
    DFUHandler handler;
} DFURequest;

/* ---------- Handlers para la demo de UAF ---------- */

void handlerLegitimo(const char *msg) {
    printf("  [LEGITIMO] Procesando comando: %s\n", msg);
}

void handlerSecuestrado(const char *msg) {
    printf("  [SECUESTRADO] Flujo desviado. Mensaje: %s\n", msg);
}

/* ---------- Modelo de capas del dispositivo ---------- */

typedef enum {
    INTACTO,
    COMPROMETIDO
} Estado;

typedef struct {
    Estado bootrom;            /* ROM inmutable del SoC          */
    Estado iboot;              /* Segunda etapa de arranque      */
    Estado kernel;             /* Kernel de iOS                  */
    Estado secure_enclave;     /* Coprocesador SEP independiente */
    Estado activation_lock;    /* Verificado contra servidores   */
} Dispositivo;

void imprimirEstado(const Dispositivo *d) {
    const char *etiqueta[] = { "INTACTO", "COMPROMETIDO" };

    printf("  BootROM .............. %s\n", etiqueta[d->bootrom]);
    printf("  iBoot ................ %s\n", etiqueta[d->iboot]);
    printf("  Kernel iOS ........... %s\n", etiqueta[d->kernel]);
    printf("  Secure Enclave (SEP) . %s\n", etiqueta[d->secure_enclave]);
    printf("  Activation Lock ...... %s\n", etiqueta[d->activation_lock]);
}

/* ---------- Funciones auxiliares para la demo UAF ---------- */

void inicializarPeticionLegitima(DFURequest *req) {
    strncpy(req->comando, "DFU_UPLOAD", sizeof(req->comando) - 1);
    req->comando[sizeof(req->comando) - 1] = '\0';

    req->longitud = 128;
    req->handler = handlerLegitimo;
}

void inicializarPayloadAtacante(DFURequest *req) {
    strncpy(req->comando, "datos_controlados", sizeof(req->comando) - 1);
    req->comando[sizeof(req->comando) - 1] = '\0';

    req->longitud = 0;
    req->handler = handlerSecuestrado;
}

/* ---------- Demo 1: use-after-free ---------- */

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

    /*
     * BUG:
     * Se libera la memoria, pero req sigue guardando la direccion antigua.
     * El puntero queda colgando, es decir, apunta a memoria que ya no
     * deberia usarse.
     */
    free(req);

    printf("Memoria liberada, pero el puntero antiguo sigue existiendo: %p\n",
           (void *)req);

    /*
     * Simulamos heap grooming.
     *
     * En una explotacion real, el atacante intenta influir en el estado
     * del heap para que una nueva reserva reutilice la misma zona liberada.
     * Aqui se reserva varias veces una estructura del mismo tamaño.
     */
    DFURequest *spray[MAX_SPRAY] = {0};
    int reutilizado = -1;

    for (int i = 0; i < MAX_SPRAY; i++) {
        spray[i] = malloc(sizeof(DFURequest));

        if (!spray[i]) {
            perror("malloc");
            break;
        }

        inicializarPayloadAtacante(spray[i]);

        if (spray[i] == req && reutilizado == -1) {
            reutilizado = i;
        }
    }

    if (reutilizado >= 0) {
        printf("Heap grooming simulado: bloque reutilizado en spray[%d] (%p)\n",
               reutilizado,
               (void *)spray[reutilizado]);

        printf("El programa vuelve a usar el puntero antiguo:\n");

        /*
         * USE-AFTER-FREE:
         * El programa cree que req sigue apuntando a una peticion legitima.
         * En realidad, esa memoria ha sido reutilizada y contiene datos
         * controlados por el atacante.
         */
        req->handler(req->comando);

        /*
         * En el modelo conceptual, marcamos BootROM como comprometida,
         * porque estamos usando esta demo para representar el caso de
         * checkm8: una vulnerabilidad en una fase inicial del arranque.
         */
        disp->bootrom = COMPROMETIDO;
    } else {
        printf("El allocator no reutilizo la direccion en esta ejecucion.\n");
        printf("La demo puede variar segun compilador, sistema y allocator.\n");
    }

    for (int i = 0; i < MAX_SPRAY; i++) {
        free(spray[i]);
    }
}

/* ---------- Demo 2: propagacion por la cadena de arranque ---------- */

void propagarCompromiso(Dispositivo *disp) {
    printf("\n=== [2] PROPAGACION POR LA CADENA DE ARRANQUE ===\n");

    if (disp->bootrom == COMPROMETIDO) {
        printf("BootROM comprometida -> se puede cargar iBoot modificado.\n");
        disp->iboot = COMPROMETIDO;

        printf("iBoot comprometido   -> se puede cargar kernel modificado.\n");
        disp->kernel = COMPROMETIDO;
    }

    /*
     * El Secure Enclave se mantiene como dominio separado.
     * En la realidad, SEP tiene su propio sistema, memoria y cadena de
     * arranque. Comprometer el procesador principal no implica obtener
     * automaticamente las claves protegidas por SEP.
     */
    printf("Secure Enclave: dominio independiente -> permanece INTACTO.\n");

    /*
     * Activation Lock depende de validaciones remotas.
     * Aunque se controle el software local, no se obtiene automaticamente
     * una autorizacion valida del servidor.
     */
    printf("Activation Lock: verificacion remota -> permanece INTACTO.\n");
}

/* ---------- Demo 3: alcance real del compromiso ---------- */

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

/* ---------- Modelo simplificado de servidor de activacion ---------- */

typedef struct {
    char ecid[32];                 /* Identificador unico del dispositivo */
    char apple_id_propietario[64]; /* Apple ID asociado al dispositivo    */
    int  activation_lock_activo;   /* Estado registrado en servidor       */
} RegistroServidorApple;

typedef struct {
    char ecid[32];
    char apple_id_presentado[64];
    int  ticket_firmado_por_apple;
} SolicitudActivacion;

/*
 * Simula la validacion del servidor.
 *
 * En un sistema real habria TLS, certificados, firmas digitales,
 * tokens, nonces, validacion de identidad, asociacion con el ECID
 * y comprobaciones internas del servidor.
 *
 * Aqui solo se representa el principio:
 * el servidor no permite la activacion si el Apple ID no coincide
 * o si no existe una autorizacion valida.
 */
int servidorAppleValidaActivacion(
    const RegistroServidorApple *registro,
    const SolicitudActivacion *solicitud
) {
    if (strcmp(registro->ecid, solicitud->ecid) != 0) {
        printf("  [SERVIDOR] ECID desconocido. Activacion rechazada.\n");
        return 0;
    }

    if (!registro->activation_lock_activo) {
        printf("  [SERVIDOR] Activation Lock no esta activo. Activacion permitida.\n");
        return 1;
    }

    if (strcmp(registro->apple_id_propietario,
               solicitud->apple_id_presentado) == 0
        && solicitud->ticket_firmado_por_apple) {

        printf("  [SERVIDOR] Apple ID correcto y ticket valido.\n");
        printf("  [SERVIDOR] Activation Lock puede retirarse legitimamente.\n");
        return 1;
    }

    printf("  [SERVIDOR] Apple ID incorrecto o ticket no valido.\n");
    printf("  [SERVIDOR] Activation Lock permanece activo.\n");

    return 0;
}

/* ---------- Demo 4: Activation Lock como limite del jailbreak ---------- */

void demoActivationLockRemoto(const Dispositivo *disp) {
    printf("\n=== [4] ACTIVATION LOCK: VALIDACION REMOTA ===\n");

    RegistroServidorApple registro = {
        "ECID-A11-123456",
        "propietario@icloud.com",
        1
    };

    SolicitudActivacion intentoAtacante = {
        "ECID-A11-123456",
        "atacante@icloud.com",
        0
    };

    /*
     * Simulamos un estado local visible.
     * Un atacante con control del kernel podria ocultar una pantalla,
     * modificar un servicio local o parchear una comprobacion local.
     */
    int activation_lock_local_visible = 1;

    if (disp->kernel == COMPROMETIDO) {
        printf("El atacante controla el kernel local.\n");
        printf("Intenta modificar el estado local de Activation Lock...\n");

        activation_lock_local_visible = 0;

        printf("  [LOCAL] Pantalla/bloqueo local parcheado temporalmente.\n");
        printf("  [LOCAL] activation_lock_local_visible = %d\n",
               activation_lock_local_visible);
    }

    printf("\nEl dispositivo intenta activarse contra el servidor remoto...\n");

    int activacionPermitida = servidorAppleValidaActivacion(
        &registro,
        &intentoAtacante
    );

    if (!activacionPermitida) {
        printf("\nResultado del intento del atacante:\n");
        printf("  - El parche local no basta para activar legitimamente el dispositivo.\n");
        printf("  - El servidor sigue asociando el ECID al Apple ID propietario.\n");
        printf("  - Activation Lock sigue siendo efectivo frente a este atacante.\n");
    }

    printf("\nCaso legitimo: propietario original autenticado.\n");

    SolicitudActivacion solicitudPropietario = {
        "ECID-A11-123456",
        "propietario@icloud.com",
        1
    };

    activacionPermitida = servidorAppleValidaActivacion(
        &registro,
        &solicitudPropietario
    );

    if (activacionPermitida) {
        printf("\nResultado legitimo:\n");
        printf("  - El servidor reconoce al propietario.\n");
        printf("  - Se permite retirar Activation Lock de forma autorizada.\n");
    }
}

/* ---------- Demo 5: catalogo de vulnerabilidades de Activation Lock ---------- */

typedef struct {
    const char *nombre;
    const char *capa_atacada;
    const char *objetivo;
    const char *resultado;
    int compromete_activation_lock;
} AnalisisVector;

void imprimirVector(const AnalisisVector *v) {
    printf("\nVector: %s\n", v->nombre);
    printf("  Capa atacada: %s\n", v->capa_atacada);
    printf("  Objetivo: %s\n", v->objetivo);
    printf("  Resultado: %s\n", v->resultado);
    printf("  Compromete Activation Lock de forma real?: %s\n",
           v->compromete_activation_lock ? "SI" : "NO");
}

void demoVulnerabilidadesActivationLock(const Dispositivo *disp) {
    printf("\n=== [5] CATALOGO DE VULNERABILIDADES: ACTIVATION LOCK ===\n");

    AnalisisVector vectores[] = {
        {
            "Parche local de interfaz",
            "Sistema local",
            "Ocultar o modificar la pantalla local de bloqueo",
            "Puede cambiar lo que ve el usuario, pero no elimina el vinculo remoto con el Apple ID.",
            0
        },
        {
            "Modificacion de servicios locales tras jailbreak",
            "Kernel / servicios locales",
            "Alterar procesos locales relacionados con la activacion",
            "Puede modificar el comportamiento del sistema comprometido, pero no genera una activacion valida.",
            0
        },
        {
            "Suplantacion del servidor de activacion",
            "Comunicacion cliente-servidor",
            "Hacer creer al dispositivo que habla con un servidor legitimo",
            "Debe fallar si se validan correctamente certificados, TLS y respuestas firmadas.",
            0
        },
        {
            "Replay de ticket de activacion",
            "Protocolo de activacion",
            "Reutilizar una respuesta antigua de activacion",
            "Debe fallar si el ticket esta ligado al ECID, a la sesion y a valores no reutilizables.",
            0
        },
        {
            "Robo de credenciales del propietario",
            "Cuenta del usuario",
            "Obtener el Apple ID y la contrasena del propietario real",
            "Podria permitir retirar el bloqueo, pero no es consecuencia directa de checkm8.",
            1
        },
        {
            "Fallo logico en servidor",
            "Infraestructura remota",
            "Explotar un error en la logica del servidor de activacion",
            "Seria critico, pero ya no seria una vulnerabilidad local del dispositivo.",
            1
        },
        {
            "Ingenieria social o phishing",
            "Usuario propietario",
            "Enganar al propietario para que retire el dispositivo de su cuenta",
            "Podria desactivar el bloqueo, pero pertenece al plano humano, no al exploit de BootROM.",
            1
        }
    };

    int total = sizeof(vectores) / sizeof(vectores[0]);

    printf("Estado local del dispositivo:\n");
    printf("  BootROM: %s\n", disp->bootrom == COMPROMETIDO ? "COMPROMETIDO" : "INTACTO");
    printf("  iBoot:   %s\n", disp->iboot == COMPROMETIDO ? "COMPROMETIDO" : "INTACTO");
    printf("  Kernel:  %s\n", disp->kernel == COMPROMETIDO ? "COMPROMETIDO" : "INTACTO");

    printf("\nAnalisis de vectores posibles:\n");

    for (int i = 0; i < total; i++) {
        imprimirVector(&vectores[i]);
    }

    printf("\nResumen academico:\n");
    printf("  - Las vulnerabilidades locales pueden alterar el dispositivo.\n");
    printf("  - Activation Lock depende de identidad, servidor y autorizacion remota.\n");
    printf("  - Por eso, jailbreak y retirada legitima de Activation Lock no son equivalentes.\n");
}

/* ---------- Demo 6: version corregida del use-after-free ---------- */

void demoCorregida(void) {
    printf("\n=== [6] VERSION CORREGIDA DEL USE-AFTER-FREE ===\n");

    DFURequest *req = malloc(sizeof(DFURequest));

    if (!req) {
        perror("malloc");
        return;
    }

    inicializarPeticionLegitima(req);

    printf("Peticion legitima creada en direccion: %p\n", (void *)req);
    req->handler(req->comando);

    /*
     * Se libera la memoria.
     */
    free(req);

    /*
     * Medida defensiva basica:
     * se invalida el puntero para que no pueda usarse accidentalmente.
     */
    req = NULL;

    if (req == NULL) {
        printf("El puntero ha sido invalidado correctamente.\n");
        printf("Se evita volver a usar memoria liberada.\n");
    } else {
        req->handler(req->comando);
    }
}

/* ---------- Main ---------- */

int main(void) {
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
