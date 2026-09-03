#include "zenoh-pico/link/config/quic.h" //this quic.h is not configured yet

#include <stdlib.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/link/manager.h"
#include "zenoh-pico/link/transport/quic.h" //this quic.h is not configured yet (different from the other at the top)

#if Z_FEATURE_LINK_QUIC == 1

z_result_t _z_endpoint_quic_valid(_z_endpoint_t *endpoint) {
    _z_string_t quic_str = _z_string_alias_str(QUIC_SCHEMA);
    if (!_z_string_equals(&endpoint->_locator._protocol, &quic_str)) {
        _Z_ERROR_LOG(_Z_ERR_CONFIG_LOCATOR_INVALID);
        return _Z_ERR_CONFIG_LOCATOR_INVALID;
    }

    z_result_t ret = _z_quic_address_valid(&endpoint->_locator._address);
    if (ret != _Z_RES_OK) {
        _Z_ERROR_LOG(_Z_ERR_CONFIG_LOCATOR_INVALID);
    }
    return ret;
}

z_result_t _z_f_link_open_quic(_z_link_t *zl) {
    uint32_t tout = Z_CONFIG_SOCKET_TIMEOUT;
    char *tout_as_str = _z_str_intmap_get(&zl->_endpoint._config, QUIC_CONFIG_TOUT_KEY);
    if (tout_as_str != NULL) {
        tout = (uint32_t)strtoul(tout_as_str, NULL, 10);
    }

    return _z_quic_open(&zl->_socket._quic._sock, zl->_socket._quic._rep, tout);
}

z_result_t _z_f_link_listen_quic(_z_link_t *zl) { return _z_quic_listen(&zl->_socket._quic._sock, zl->_socket._quic._rep); }

void _z_f_link_close_quic(_z_link_t *zl) { _z_quic_close(&zl->_socket._quic._sock); }

void _z_f_link_free_quic(_z_link_t *zl) { _z_quic_endpoint_clear(&zl->_socket._quic._rep); }

size_t _z_f_link_write_quic(const _z_link_t *zl, const uint8_t *ptr, size_t len, _z_sys_net_socket_t *socket) {
    if (socket != NULL) {
        return _z_quic_write(*socket, ptr, len);
    } else {
        return _z_quic_write(zl->_socket._quic._sock, ptr, len);
    }
}

size_t _z_f_link_write_all_quic(const _z_link_t *zl, const uint8_t *ptr, size_t len) {
    return _z_quic_write(zl->_socket._quic._sock, ptr, len);
}

size_t _z_f_link_read_quic(const _z_link_t *zl, uint8_t *ptr, size_t len, _z_slice_t *addr) {
    _ZP_UNUSED(addr);
    return _z_quic_read(zl->_socket._quic._sock, ptr, len);
}

size_t _z_f_link_read_exact_quic(const _z_link_t *zl, uint8_t *ptr, size_t len, _z_slice_t *addr,
                                _z_sys_net_socket_t *socket) {
    _ZP_UNUSED(addr);
    if (socket != NULL) {
        return _z_quic_read_exact(*socket, ptr, len);
    } else {
        return _z_quic_read_exact(zl->_socket._quic._sock, ptr, len);
    }
}

size_t _z_f_link_quic_read_socket(const _z_sys_net_socket_t socket, uint8_t *ptr, size_t len) {
    return _z_quic_read(socket, ptr, len);
}

uint16_t _z_get_link_mtu_quic(void) {
    // Maximum MTU for QUIC (??)
    return 65535;
}

z_result_t _z_new_peer_quic(_z_endpoint_t *endpoint, _z_sys_net_socket_t *socket) {
    _z_sys_net_endpoint_t sys_endpoint = {0};
    z_result_t ret = _z_quic_endpoint_init_from_address(&sys_endpoint, &endpoint->_locator._address);

    if (ret != _Z_RES_OK) {
        _z_quic_endpoint_clear(&sys_endpoint);
        return ret;
    }

    ret = _z_quic_open(socket, sys_endpoint, Z_CONFIG_SOCKET_TIMEOUT);
    _z_quic_endpoint_clear(&sys_endpoint);
    return ret;
}

z_result_t _z_new_link_quic(_z_link_t *zl, _z_endpoint_t *endpoint) {
    zl->_type = _Z_LINK_TYPE_QUIC;
    zl->_cap._transport = Z_LINK_CAP_TRANSPORT_UNICAST;
    zl->_cap._flow = Z_LINK_CAP_FLOW_STREAM;
    zl->_cap._is_reliable = true;

    zl->_mtu = _z_get_link_mtu_quic();

    zl->_endpoint = *endpoint;
    z_result_t ret = _z_quic_endpoint_init_from_address(&zl->_socket._quic._rep, &endpoint->_locator._address);

    zl->_open_f = _z_f_link_open_quic;
    zl->_listen_f = _z_f_link_listen_quic;
    zl->_close_f = _z_f_link_close_quic;
    zl->_free_f = _z_f_link_free_quic;

    zl->_write_f = _z_f_link_write_quic;
    zl->_write_all_f = _z_f_link_write_all_quic;
    zl->_read_f = _z_f_link_read_quic;
    zl->_read_exact_f = _z_f_link_read_exact_quic;
    zl->_read_socket_f = _z_f_link_quic_read_socket;

    return ret;
}
#else
z_result_t _z_endpoint_quic_valid(_z_endpoint_t *endpoint) {
    _ZP_UNUSED(endpoint);
    _Z_ERROR_RETURN(_Z_ERR_TRANSPORT_NOT_AVAILABLE);
}

z_result_t _z_new_peer_quic(_z_endpoint_t *endpoint, _z_sys_net_socket_t *socket) {
    _ZP_UNUSED(endpoint);
    _ZP_UNUSED(socket);
    _Z_ERROR_RETURN(_Z_ERR_TRANSPORT_NOT_AVAILABLE);
}

z_result_t _z_new_link_quic(_z_link_t *zl, _z_endpoint_t *endpoint) {
    _ZP_UNUSED(zl);
    _ZP_UNUSED(endpoint);
    _Z_ERROR_RETURN(_Z_ERR_TRANSPORT_NOT_AVAILABLE);
}
#endif