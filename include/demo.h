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

void imprimirEstado(const Dispositivo *d);
void demoUseAfterFree(Dispositivo *disp);
void propagarCompromisoModelo(Dispositivo *disp);
void demoPropagacionCompromiso(Dispositivo *disp);
void capacidadesDelAtacante(const Dispositivo *disp);
ActivationResult validarActivationLockModelo(
    const ActivationRecord *record,
    const ActivationRequest *request
);
void demoActivationLockRemoto(const Dispositivo *disp);
void demoVulnerabilidadesActivationLock(const Dispositivo *disp);
void demoCorregida(void);
void demoAddressSanitizerUseAfterFree(void);

#endif
