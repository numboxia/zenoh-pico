#ifndef ZENOH_PICO_LINK_TRANSPORT_QUIC_H
#define ZENOH_PICO_LINK_TRANSPORT_QUIC_H

#include <stddef.h>
#include <stdint.h>

#include "zenoh-pico/collections/string.h"
#include "zenoh-pico/system/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    _z_sys_net_socket_t _sock;
    _z_sys_net_endpoint_t _rep;
} _z_quic_socket_t;

char *_z_quic_address_parse_host(const _z_string_t *address);
z_result_t _z_quic_address_valid(const _z_string_t *address);
z_result_t _z_quic_endpoint_init(_z_sys_net_endpoint_t *ep, const char *address, const char *port);
void _z_quic_endpoint_clear(_z_sys_net_endpoint_t *ep);
z_result_t _z_quic_endpoint_init_from_address(_z_sys_net_endpoint_t *ep, const _z_string_t *address);

// flawfinder: ignore
z_result_t _z_quic_open(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t endpoint, uint32_t tout);
z_result_t _z_quic_listen(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t endpoint);
z_result_t _z_quic_accept(const _z_sys_net_socket_t *sock_in, _z_sys_net_socket_t *sock_out);
void _z_quic_close(_z_sys_net_socket_t *sock);

// flawfinder: ignore
size_t _z_quic_read(_z_sys_net_socket_t sock, uint8_t *ptr, size_t len);
size_t _z_quic_read_exact(_z_sys_net_socket_t sock, uint8_t *ptr, size_t len);
size_t _z_quic_write(_z_sys_net_socket_t sock, const uint8_t *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ZENOH_PICO_LINK_TRANSPORT_QUIC_H */
