#pragma once

#include <esp_err.h>

typedef struct {} esp_ota_handle_t;
typedef struct {} esp_partition_t;

inline esp_err_t esp_ota_mark_app_valid_cancel_rollback() { return 0; }
inline esp_err_t esp_ota_mark_app_invalid_rollback_and_reboot() { return 0; }
