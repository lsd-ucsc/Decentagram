#ifndef MBEDTLS_THREADING_ALT_H
#define MBEDTLS_THREADING_ALT_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MBEDTLS_THREADING_ALT)

typedef void* mbedtls_threading_mutex_t;

#endif

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_THREADING_ALT_H */
