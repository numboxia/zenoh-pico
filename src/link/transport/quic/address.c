#include <stdlib.h>

#include "zenoh-pico/link/endpoint.h"
#include "zenoh-pico/link/transport/quic.h"

z_result_t _z_quic_address_valid(const _z_string_t *address) {
    char *host = _z_endpoint_parse_host(address);
    char *port = _z_endpoint_parse_port(address);
    z_result_t ret = ((host != NULL) && (port != NULL)) ? _Z_RES_OK : _Z_ERR_CONFIG_LOCATOR_INVALID;

    z_free(host);
    z_free(port);
    return ret;
}

z_result_t _z_quic_endpoint_init_from_address(_z_sys_net_endpoint_t *ep, const _z_string_t *address) {
    z_result_t ret = _Z_RES_OK;
    char *host = _z_endpoint_parse_host(address);
    char *port = _z_endpoint_parse_port(address);

    if ((host == NULL) || (port == NULL)) {
        ret = _Z_ERR_CONFIG_LOCATOR_INVALID;
    } else {
        ret = _z_quic_endpoint_init(ep, host, port);
    }

    z_free(host);
    z_free(port);
    return ret;
}
