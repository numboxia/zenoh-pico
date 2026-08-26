// Did not configure out how to change here

#ifndef ZENOH_PICO_LINK_CONFIG_QUIC_H
#define ZENOH_PICO_LINK_CONFIG_QUIC_H

#include "zenoh-pico/collections/intmap.h"
#include "zenoh-pico/collections/string.h"
#include "zenoh-pico/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if Z_FEATURE_LINK_QUIC == 1

#define QUIC_CONFIG_ARGC 1

#define QUIC_CONFIG_TOUT_KEY 0x01
#define QUIC_CONFIG_TOUT_STR "tout"

/* #define QUIC_CONFIG_MAPPING_BUILD              \
    _z_str_intmapping_t args[QUIC_CONFIG_ARGC]; \
    args[0]._key = TCP_CONFIG_TOUT_KEY;        \
    args[0]._str = (char *)TCP_CONFIG_TOUT_STR; */

size_t _z_quic_config_strlen(const _z_str_intmap_t *s);

void _z_quic_config_onto_str(char *dst, size_t dst_len, const _z_str_intmap_t *s);
char *_z_quic_config_to_str(const _z_str_intmap_t *s);

z_result_t _z_quic_config_from_str(_z_str_intmap_t *strint, const char *s);
z_result_t _z_quic_config_from_strn(_z_str_intmap_t *strint, const char *s, size_t n);
#endif

#ifdef __cplusplus
}
#endif

#endif  // ZENOH_PICO_LINK_CONFIG_QUIC_H
