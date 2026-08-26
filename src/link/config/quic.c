#include "zenoh-pico/link/config/quic.h"

#include <string.h>

#include "zenoh-pico/config.h"

#if Z_FEATURE_LINK_QUIC == 1

size_t _z_quic_config_strlen(const _z_str_intmap_t *s) {
    QUIC_CONFIG_MAPPING_BUILD

    return _z_str_intmap_strlen(s, QUIC_CONFIG_ARGC, args);
}

void _z_quic_config_onto_str(char *dst, size_t dst_len, const _z_str_intmap_t *s) {
    QUIC_CONFIG_MAPPING_BUILD

    _z_str_intmap_onto_str(dst, dst_len, s, QUIC_CONFIG_ARGC, args);
}

char *_z_quic_config_to_str(const _z_str_intmap_t *s) {
    QUIC_CONFIG_MAPPING_BUILD

    return _z_str_intmap_to_str(s, QUIC_CONFIG_ARGC, args);
}

z_result_t _z_quic_config_from_strn(_z_str_intmap_t *strint, const char *s, size_t n) {
    QUIC_CONFIG_MAPPING_BUILD

    return _z_str_intmap_from_strn(strint, s, QUIC_CONFIG_ARGC, args, n);
}

z_result_t _z_quic_config_from_str(_z_str_intmap_t *strint, const char *s) {
    return _z_quic_config_from_strn(strint, s, strlen(s));
}
#endif
