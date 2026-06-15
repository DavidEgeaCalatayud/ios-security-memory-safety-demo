#ifndef IOS_SECURITY_MEMORY_SAFETY_DEMO_H
#define IOS_SECURITY_MEMORY_SAFETY_DEMO_H

#define MAX_SPRAY 64
#define ACTIVATION_ECID_SIZE 32
#define ACTIVATION_APPLE_ID_SIZE 64

typedef void (*DFUHandler)(const char *msg);

typedef struct {
    char       comando[32];
    int        longitud;
    DFUHandler handler;
} DFURequest;

typedef enum {
    INTACTO,
    COMPROMETIDO
} Estado;

typedef enum {
    ACTIVATION_DENIED = 0,
    ACTIVATION_ALLOWED = 1
} ActivationResult;

typedef enum {
    IOS_TRUST_DENIED = 0,
    IOS_TRUST_ALLOWED = 1
} IosTrustDecision;

typedef struct {
    char ecid[ACTIVATION_ECID_SIZE];
    char apple_id_propietario[ACTIVATION_APPLE_ID_SIZE];
    int  activation_lock_activo;
} ActivationRecord;

typedef struct {
    char ecid[ACTIVATION_ECID_SIZE];
    char apple_id_presentado[ACTIVATION_APPLE_ID_SIZE];
    int  ticket_firmado_por_apple;
} ActivationRequest;

typedef struct {
    Estado bootrom;
    Estado iboot;
    Estado kernel;
    Estado secure_enclave;
    Estado activation_lock;
} Dispositivo;

typedef struct {
    int bootrom_immutable;
    int dfu_parser_exposed;
    int boot_policy_enforces_img4;
    int iboot_validates_kernelcache;
    int sep_separate_processor;
    int keybag_requires_user_secret;
    int activation_requires_remote_authorization;
} IosInternalsModel;

int estadoEsValido(Estado estado);
const char *estadoComoTexto(Estado estado);
void imprimirEstado(const Dispositivo *d);
void demoUseAfterFree(Dispositivo *disp);
void propagarCompromisoModelo(Dispositivo *disp);
void demoPropagacionCompromiso(Dispositivo *disp);
void capacidadesDelAtacante(const Dispositivo *disp);
ActivationResult validarActivationLockModelo(
    const ActivationRecord *record,
    const ActivationRequest *request
);
IosInternalsModel crearModeloIosInternalsAcademico(void);
IosTrustDecision validarCadenaArranqueIosModelo(const IosInternalsModel *model);
IosTrustDecision compromisoLocalPermiteExtraerClavesSepModelo(
    const IosInternalsModel *model,
    const Dispositivo *disp
);
IosTrustDecision compromisoLocalPermiteAutorizarActivationLockModelo(
    const IosInternalsModel *model,
    const Dispositivo *disp
);
void demoIosInternalsModelo(const Dispositivo *disp);
void demoActivationLockRemoto(const Dispositivo *disp);
void demoVulnerabilidadesActivationLock(const Dispositivo *disp);
void demoCorregida(void);
void demoAddressSanitizerUseAfterFree(void);

#endif
