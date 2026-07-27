/****************************************************************************
 * examples/nuttx/zenoh_pub/zenoh_pub_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * zenoh-pico publisher NuttX builtin app.
 * Publishes a counter on "t3/pub/data" every second via UDP peer mode.
 * Network is provided by virtio-net (Linux A53 remoteproc backend).
 *
 * NSH usage:
 *   nsh> zenoh_pub
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <zenoh-pico.h>

/* ---- Session mode --------------------------------------------------------- */
/* 0 = client (router at ZENOH_LOCATOR), 1 = peer (multicast, default) */
#ifndef ZENOH_PUB_CLIENT_OR_PEER
#  define ZENOH_PUB_CLIENT_OR_PEER 1
#endif

#if ZENOH_PUB_CLIENT_OR_PEER == 0
#  define ZENOH_MODE    "client"
#  define ZENOH_LOCATOR "tcp/192.168.1.100:7447"
#elif ZENOH_PUB_CLIENT_OR_PEER == 1
#  define ZENOH_MODE    "peer"
#  define ZENOH_LOCATOR "udp/224.0.0.224:7446"
#endif

#define PUB_KEYEXPR   "t3/pub/data"
#define PUB_PERIOD_MS 1000U

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int zenoh_pub_main(int argc, FAR char *argv[])
{
    (void)argc;
    (void)argv;

    printf("[zenoh_pub] mode=%s  keyexpr=%s\n", ZENOH_MODE, PUB_KEYEXPR);

    z_owned_config_t cfg;
    z_config_default(&cfg);
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_MODE_KEY, ZENOH_MODE);
#if ZENOH_PUB_CLIENT_OR_PEER == 0
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_CONNECT_KEY, ZENOH_LOCATOR);
#else
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_LISTEN_KEY, ZENOH_LOCATOR);
#endif

    z_owned_session_t s;
    if (z_open(&s, z_move(cfg), NULL) < 0)
    {
        printf("[zenoh_pub] z_open failed\n");
        return -1;
    }

    if (zp_start_read_task(z_loan_mut(s), NULL) < 0 ||
        zp_start_lease_task(z_loan_mut(s), NULL) < 0)
    {
        printf("[zenoh_pub] Failed to start background tasks\n");
        z_drop(z_move(s));
        return -1;
    }

    z_owned_publisher_t pub;
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str(&ke, PUB_KEYEXPR);
    if (z_declare_publisher(z_loan(s), &pub, z_loan(ke), NULL) < 0)
    {
        printf("[zenoh_pub] z_declare_publisher failed\n");
        z_drop(z_move(s));
        return -1;
    }

    printf("[zenoh_pub] Publishing on '%s' every %u ms...\n",
           PUB_KEYEXPR, (unsigned)PUB_PERIOD_MS);

    char buf[64];
    for (uint32_t idx = 0; ; idx++)
    {
        snprintf(buf, sizeof(buf), "[%4u] Hello from NuttX R5F!", idx);

        z_owned_bytes_t payload;
        z_bytes_copy_from_str(&payload, buf);
        z_publisher_put(z_loan(pub), z_move(payload), NULL);

        printf("[zenoh_pub] Put '%s'\n", buf);
        usleep(PUB_PERIOD_MS * 1000U);
    }

    /* Not reached in normal operation */
    z_drop(z_move(pub));
    zp_stop_read_task(z_loan_mut(s));
    zp_stop_lease_task(z_loan_mut(s));
    z_drop(z_move(s));
    return 0;
}
