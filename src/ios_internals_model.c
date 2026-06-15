#include <stdio.h>

#include "demo.h"

IosInternalsModel crearModeloIosInternalsAcademico(void) {
    IosInternalsModel model = {
        1,
        1,
        1,
        1,
        1,
        1,
        1
    };

    return model;
}

IosTrustDecision validarCadenaArranqueIosModelo(const IosInternalsModel *model) {
    if (model == NULL) {
        return IOS_TRUST_DENIED;
    }

    if (!model->bootrom_immutable) {
        return IOS_TRUST_DENIED;
    }

    if (!model->boot_policy_enforces_img4) {
        return IOS_TRUST_DENIED;
    }

    if (!model->iboot_validates_kernelcache) {
        return IOS_TRUST_DENIED;
    }

    return IOS_TRUST_ALLOWED;
}

IosTrustDecision compromisoLocalPermiteExtraerClavesSepModelo(
    const IosInternalsModel *model,
    const Dispositivo *disp
) {
    if (model == NULL || disp == NULL) {
        return IOS_TRUST_DENIED;
    }

    if (!model->sep_separate_processor) {
        return IOS_TRUST_ALLOWED;
    }

    if (!model->keybag_requires_user_secret) {
        return IOS_TRUST_ALLOWED;
    }

    if (disp->secure_enclave == COMPROMETIDO) {
        return IOS_TRUST_ALLOWED;
    }

    return IOS_TRUST_DENIED;
}

IosTrustDecision compromisoLocalPermiteAutorizarActivationLockModelo(
    const IosInternalsModel *model,
    const Dispositivo *disp
) {
    if (model == NULL || disp == NULL) {
        return IOS_TRUST_DENIED;
    }

    if (!model->activation_requires_remote_authorization) {
        return IOS_TRUST_ALLOWED;
    }

    if (disp->activation_lock == COMPROMETIDO) {
        return IOS_TRUST_ALLOWED;
    }

    return IOS_TRUST_DENIED;
}

void demoIosInternalsModelo(const Dispositivo *disp) {
    IosInternalsModel model = crearModeloIosInternalsAcademico();

    printf("\n=== [6] MODELO CONCEPTUAL DE iOS INTERNALS ===\n");
    printf("Este modulo no reproduce checkm8 ni implementa internals reales de Apple.\n");
    printf("Modela dominios de seguridad para explicar limites de confianza.\n");

    printf("\nBoot chain conceptual:\n");
    printf("  - BootROM/SecureROM: primera raiz de confianza del procesador principal.\n");
    printf("  - DFU: superficie temprana de recuperacion antes del sistema operativo.\n");
    printf("  - iBoot: etapa que valida y prepara la carga del kernel.\n");
    printf("  - Kernelcache: nucleo del sistema iOS.\n");
    printf("  - Userland: procesos y servicios de usuario.\n");

    printf("\nDominios separados:\n");
    printf("  - Secure Enclave: dominio separado para claves y operaciones sensibles.\n");
    printf("  - Keybag: modelo conceptual de proteccion de claves ligadas al usuario.\n");
    printf("  - Activation Lock: autorizacion remota ligada a identidad y servidor.\n");

    printf("\nPolitica de arranque segura: %s\n",
           validarCadenaArranqueIosModelo(&model) == IOS_TRUST_ALLOWED ? "COHERENTE" : "ROTA");

    if (disp == NULL) {
        printf("No hay dispositivo local para evaluar alcance de compromiso.\n");
        return;
    }

    printf("\nEvaluacion de alcance con el estado actual del dispositivo:\n");
    printf("  Estado BootROM: %s\n", estadoComoTexto(disp->bootrom));
    printf("  Estado Kernel:  %s\n", estadoComoTexto(disp->kernel));
    printf("  Estado SEP:     %s\n", estadoComoTexto(disp->secure_enclave));
    printf("  Activation Lock:%s\n", estadoComoTexto(disp->activation_lock));

    printf("\nResultado conceptual:\n");
    printf("  - Compromiso local del procesador principal != control automatico del SEP: %s\n",
           compromisoLocalPermiteExtraerClavesSepModelo(&model, disp) == IOS_TRUST_ALLOWED ?
           "NO APLICABLE EN ESTE MODELO" : "SE MANTIENE SEPARADO");
    printf("  - Compromiso local != autorizacion remota de Activation Lock: %s\n",
           compromisoLocalPermiteAutorizarActivationLockModelo(&model, disp) == IOS_TRUST_ALLOWED ?
           "NO APLICABLE EN ESTE MODELO" : "REQUIERE SERVIDOR");

    printf("\nNota academica:\n");
    printf("  La demo UAF ilustra una clase de error de memoria. No describe offsets,\n");
    printf("  paquetes USB, payloads, cadenas ROP, blobs firmados ni procedimientos\n");
    printf("  reales de explotacion sobre dispositivos Apple.\n");
}
