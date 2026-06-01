#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "demo.h"

static ActivationRecord make_record(const char *ecid, const char *apple_id, int lock_active) {
    ActivationRecord record;

    memset(&record, 0, sizeof(record));
    strncpy(record.ecid, ecid, sizeof(record.ecid) - 1);
    strncpy(record.apple_id_propietario, apple_id, sizeof(record.apple_id_propietario) - 1);
    record.activation_lock_activo = lock_active;

    return record;
}

static ActivationRequest make_request(const char *ecid, const char *apple_id, int signed_ticket) {
    ActivationRequest request;

    memset(&request, 0, sizeof(request));
    strncpy(request.ecid, ecid, sizeof(request.ecid) - 1);
    strncpy(request.apple_id_presentado, apple_id, sizeof(request.apple_id_presentado) - 1);
    request.ticket_firmado_por_apple = signed_ticket;

    return request;
}

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

static void test_boot_chain_model_propagation(void) {
    Dispositivo disp = {
        COMPROMETIDO,
        INTACTO,
        INTACTO,
        INTACTO,
        INTACTO
    };

    propagarCompromisoModelo(&disp);

    assert(disp.bootrom == COMPROMETIDO);
    assert(disp.iboot == COMPROMETIDO);
    assert(disp.kernel == COMPROMETIDO);
    assert(disp.secure_enclave == INTACTO);
    assert(disp.activation_lock == INTACTO);
}

static void test_boot_chain_model_ignores_null_device(void) {
    propagarCompromisoModelo(NULL);
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

static void test_activation_lock_rejects_null_inputs(void) {
    ActivationRecord record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    ActivationRequest request = make_request("ECID-A11-123456", "owner@icloud.com", 1);

    assert(validarActivationLockModelo(NULL, &request) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&record, NULL) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(NULL, NULL) == ACTIVATION_DENIED);
}

static void test_activation_lock_rejects_invalid_empty_fields(void) {
    ActivationRecord valid_record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    ActivationRequest valid_request = make_request("ECID-A11-123456", "owner@icloud.com", 1);

    ActivationRecord empty_ecid_record = make_record("", "owner@icloud.com", 1);
    ActivationRecord empty_owner_record = make_record("ECID-A11-123456", "", 1);
    ActivationRequest empty_ecid_request = make_request("", "owner@icloud.com", 1);
    ActivationRequest empty_owner_request = make_request("ECID-A11-123456", "", 1);

    assert(validarActivationLockModelo(&empty_ecid_record, &valid_request) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&empty_owner_record, &valid_request) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&valid_record, &empty_ecid_request) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&valid_record, &empty_owner_request) == ACTIVATION_DENIED);
}

static void test_activation_lock_rejects_non_terminated_fields(void) {
    ActivationRecord record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    ActivationRequest request = make_request("ECID-A11-123456", "owner@icloud.com", 1);

    memset(record.ecid, 'A', sizeof(record.ecid));
    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_DENIED);

    record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    memset(record.apple_id_propietario, 'B', sizeof(record.apple_id_propietario));
    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_DENIED);

    record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    memset(request.ecid, 'C', sizeof(request.ecid));
    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_DENIED);

    request = make_request("ECID-A11-123456", "owner@icloud.com", 1);
    memset(request.apple_id_presentado, 'D', sizeof(request.apple_id_presentado));
    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_DENIED);
}

static void test_activation_lock_accepts_owner_with_ticket(void) {
    ActivationRecord record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    ActivationRequest request = make_request("ECID-A11-123456", "owner@icloud.com", 1);

    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_ALLOWED);
}

static void test_activation_lock_rejects_wrong_identity_or_ticket(void) {
    ActivationRecord record = make_record("ECID-A11-123456", "owner@icloud.com", 1);
    ActivationRequest wrong_ecid = make_request("ECID-A10-999999", "owner@icloud.com", 1);
    ActivationRequest wrong_owner = make_request("ECID-A11-123456", "other@icloud.com", 1);
    ActivationRequest missing_ticket = make_request("ECID-A11-123456", "owner@icloud.com", 0);

    assert(validarActivationLockModelo(&record, &wrong_ecid) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&record, &wrong_owner) == ACTIVATION_DENIED);
    assert(validarActivationLockModelo(&record, &missing_ticket) == ACTIVATION_DENIED);
}

static void test_activation_lock_inactive_allows_activation(void) {
    ActivationRecord record = make_record("ECID-A11-123456", "owner@icloud.com", 0);
    ActivationRequest request = make_request("ECID-A11-123456", "someone@icloud.com", 0);

    assert(validarActivationLockModelo(&record, &request) == ACTIVATION_ALLOWED);
}

int main(void) {
    test_initial_device_state();
    test_boot_chain_model_propagation();
    test_boot_chain_model_ignores_null_device();
    test_dfu_request_layout();
    test_activation_lock_rejects_null_inputs();
    test_activation_lock_rejects_invalid_empty_fields();
    test_activation_lock_rejects_non_terminated_fields();
    test_activation_lock_accepts_owner_with_ticket();
    test_activation_lock_rejects_wrong_identity_or_ticket();
    test_activation_lock_inactive_allows_activation();

    printf("All unit tests passed.\n");
    return 0;
}
