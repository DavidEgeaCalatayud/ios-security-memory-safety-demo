#include <stdio.h>
#include <string.h>

#include "demo.h"

typedef struct {
    char ecid[32];
    char apple_id_propietario[64];
    int  activation_lock_activo;
} RegistroServidorApple;

typedef struct {
    char ecid[32];
    char apple_id_presentado[64];
    int  ticket_firmado_por_apple;
} SolicitudActivacion;

typedef struct {
    const char *nombre;
    const char *capa_atacada;
    const char *objetivo;
    const char *resultado;
    int compromete_activation_lock;
} AnalisisVector;

static int servidorAppleValidaActivacionSilencioso(
    const RegistroServidorApple *registro,
    const SolicitudActivacion *solicitud
) {
    if (strcmp(registro->ecid, solicitud->ecid) != 0) {
        return 0;
    }

    if (!registro->activation_lock_activo) {
        return 1;
    }

    return strcmp(registro->apple_id_propietario, solicitud->apple_id_presentado) == 0
        && solicitud->ticket_firmado_por_apple;
}

int validarActivationLockModelo(
    const char *ecid_registrado,
    const char *apple_id_propietario,
    int activation_lock_activo,
    const char *ecid_solicitud,
    const char *apple_id_presentado,
    int ticket_firmado_por_apple
) {
    RegistroServidorApple registro;
    SolicitudActivacion solicitud;

    memset(&registro, 0, sizeof(registro));
    memset(&solicitud, 0, sizeof(solicitud));

    strncpy(registro.ecid, ecid_registrado, sizeof(registro.ecid) - 1);
    strncpy(registro.apple_id_propietario,
            apple_id_propietario,
            sizeof(registro.apple_id_propietario) - 1);
    registro.activation_lock_activo = activation_lock_activo;

    strncpy(solicitud.ecid, ecid_solicitud, sizeof(solicitud.ecid) - 1);
    strncpy(solicitud.apple_id_presentado,
            apple_id_presentado,
            sizeof(solicitud.apple_id_presentado) - 1);
    solicitud.ticket_firmado_por_apple = ticket_firmado_por_apple;

    return servidorAppleValidaActivacionSilencioso(&registro, &solicitud);
}

static int servidorAppleValidaActivacion(
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

static void imprimirVector(const AnalisisVector *v) {
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
