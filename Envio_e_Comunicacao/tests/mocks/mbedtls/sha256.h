#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
} mbedtls_sha256_context;

inline void mbedtls_sha256_init(mbedtls_sha256_context *ctx) {}
inline void mbedtls_sha256_free(mbedtls_sha256_context *ctx) {}
inline int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int is224) { return 0; }
inline int mbedtls_sha256_update(mbedtls_sha256_context *ctx, const uint8_t *input, size_t ilen) { return 0; }
inline int mbedtls_sha256_finish(mbedtls_sha256_context *ctx, uint8_t output[32]) { return 0; }
