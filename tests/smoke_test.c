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

int main(void) {
    test_initial_device_state();
    test_boot_chain_propagation();
    test_dfu_request_layout();

    printf("All tests passed.\n");
    return 0;
}
