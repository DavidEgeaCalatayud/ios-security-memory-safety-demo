#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "demo.h"

static void test_initial_device_state(void) {
    Dispositivo disp = {
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO
    };

    assert(disp.bootrom == INTACTO);
    assert(disp.iboot == INTACTO);
    assert(disp.kernel == INTACTO);
    assert(disp.secure_enclave == INTACTO);
    assert(disp.activation_lock == INTACTO);
}

static void test_boot_chain_propagation(void) {
    Dispositivo disp = {
        COMPROMETIDO,
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO
    };

    propagarCompromiso(&disp);

    assert(disp.bootrom == COMPROMETIDO);
    assert(disp.iboot == COMPROMETIDO);
    assert(disp.kernel == COMPROMETIDO);
    assert(disp.secure_enclave == INTACTO);
    assert(disp.activation_lock == INTACTO);
}

static void test_dfu_request_layout(void) {
    DFURequest req;

    memset(&req, 0, sizeof(req));
    strncpy(req.comando, "DFU_UPLOAD", sizeof(req.comando) - 1);
    req.longitud = 128;
    req.handler = NULL;

    assert(strcmp(req.comando, "DFU_UPLOAD") == 0);
    assert(req.longitud == 128);
    assert(req.handler == NULL);
}

static void test_activation_lock_rejects_attacker(void) {
    int allowed = validarActivationLockModelo(
        "ECID-A11-123456",
        "owner@icloud.com",
        1,
        "ECID-A11-123456",
        "attacker@icloud.com",
        0
    );

    assert(allowed == 0);
}

static void test_activation_lock_accepts_owner_with_ticket(void) {
    int allowed = validarActivationLockModelo(
        "ECID-A11-123456",
        "owner@icloud.com",
        1,
        "ECID-A11-123456",
        "owner@icloud.com",
        1
    );

    assert(allowed == 1);
}

static void test_activation_lock_rejects_wrong_ecid(void) {
    int allowed = validarActivationLockModelo(
        "ECID-A11-123456",
        "owner@icloud.com",
        1,
        "ECID-A10-999999",
        "owner@icloud.com",
        1
    );

    assert(allowed == 0);
}

static void test_activation_lock_requires_signed_ticket(void) {
    int allowed = validarActivationLockModelo(
        "ECID-A11-123456",
        "owner@icloud.com",
        1,
        "ECID-A11-123456",
        "owner@icloud.com",
        0
    );

    assert(allowed == 0);
}

static void test_activation_lock_inactive_allows_activation(void) {
    int allowed = validarActivationLockModelo(
        "ECID-A11-123456",
        "owner@icloud.com",
        0,
        "ECID-A11-123456",
        "someone@icloud.com",
        0
    );

    assert(allowed == 1);
}

int main(void) {
    test_initial_device_state();
    test_boot_chain_propagation();
    test_dfu_request_layout();
    test_activation_lock_rejects_attacker();
    test_activation_lock_accepts_owner_with_ticket();
    test_activation_lock_rejects_wrong_ecid();
    test_activation_lock_requires_signed_ticket();
    test_activation_lock_inactive_allows_activation();

    printf("All unit tests passed.\n");
    return 0;
}
