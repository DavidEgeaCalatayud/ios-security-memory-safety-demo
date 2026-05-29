#ifndef IOS_SECURITY_MEMORY_SAFETY_DEMO_H
#define IOS_SECURITY_MEMORY_SAFETY_DEMO_H

#define MAX_SPRAY 64

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

typedef struct {
    Estado bootrom;
    Estado iboot;
    Estado kernel;
    Estado secure_enclave;
    Estado activation_lock;
} Dispositivo;

void imprimirEstado(const Dispositivo *d);
void demoUseAfterFree(Dispositivo *disp);
void propagarCompromiso(Dispositivo *disp);
void capacidadesDelAtacante(const Dispositivo *disp);
int validarActivationLockModelo(
    const char *ecid_registrado,
    const char *apple_id_propietario,
    int activation_lock_activo,
    const char *ecid_solicitud,
    const char *apple_id_presentado,
    int ticket_firmado_por_apple
);
void demoActivationLockRemoto(const Dispositivo *disp);
void demoVulnerabilidadesActivationLock(const Dispositivo *disp);
void demoCorregida(void);
void demoAddressSanitizerUseAfterFree(void);

#endif
