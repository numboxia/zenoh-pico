/****************************************************************************
 * examples/nuttx/zenoh_sub/zenoh_sub_main.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * zenoh-pico subscriber NuttX builtin app.
 * Subscribes to "t3/pub/**" and prints each received sample.
 * Network is provided by virtio-net (Linux A53 remoteproc backend).
 *
 * NSH usage:
 *   nsh> zenoh_sub
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <zenoh-pico.h>

/* ---- Session mode --------------------------------------------------------- */
/* 0 = client (router at ZENOH_LOCATOR), 1 = peer (multicast, default) */
#ifndef ZENOH_SUB_CLIENT_OR_PEER
#  define ZENOH_SUB_CLIENT_OR_PEER 1
#endif

#if ZENOH_SUB_CLIENT_OR_PEER == 0
#  define ZENOH_MODE    "client"
#  define ZENOH_LOCATOR "tcp/192.168.1.100:7447"
#elif ZENOH_SUB_CLIENT_OR_PEER == 1
#  define ZENOH_MODE    "peer"
#  define ZENOH_LOCATOR "udp/224.0.0.224:7446"
#endif

#define SUB_KEYEXPR "t3/pub/**"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void sample_handler(z_loaned_sample_t *sample, void *ctx)
{
    (void)ctx;

    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);

    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    printf("[zenoh_sub] >> ('%.*s': '%.*s')\n",
           (int)z_string_len(z_loan(keystr)),
           z_string_data(z_loan(keystr)),
           (int)z_string_len(z_loan(value)),
           z_string_data(z_loan(value)));

    z_drop(z_move(value));
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int zenoh_sub_main(int argc, FAR char *argv[])
{
    (void)argc;
    (void)argv;

    printf("[zenoh_sub] mode=%s  keyexpr=%s\n", ZENOH_MODE, SUB_KEYEXPR);

    z_owned_config_t cfg;
    z_config_default(&cfg);
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_MODE_KEY, ZENOH_MODE);
#if ZENOH_SUB_CLIENT_OR_PEER == 0
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_CONNECT_KEY, ZENOH_LOCATOR);
#else
    zp_config_insert(z_loan_mut(cfg), Z_CONFIG_LISTEN_KEY, ZENOH_LOCATOR);
#endif

    z_owned_session_t s;
    if (z_open(&s, z_move(cfg), NULL) < 0)
    {
        printf("[zenoh_sub] z_open failed\n");
        return -1;
    }

    if (zp_start_read_task(z_loan_mut(s), NULL) < 0 ||
        zp_start_lease_task(z_loan_mut(s), NULL) < 0)
    {
        printf("[zenoh_sub] Failed to start background tasks\n");
        z_drop(z_move(s));
        return -1;
    }

    z_owned_subscriber_t sub;
    z_owned_closure_sample_t cb;
    z_closure(&cb, sample_handler, NULL, NULL);

    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str(&ke, SUB_KEYEXPR);
    if (z_declare_subscriber(z_loan(s), &sub, z_loan(ke), z_move(cb), NULL) < 0)
    {
        printf("[zenoh_sub] z_declare_subscriber failed\n");
        z_drop(z_move(s));
        return -1;
    }

    printf("[zenoh_sub] Waiting for data on '%s'...\n", SUB_KEYEXPR);

    /* Spin — zenoh read task delivers callbacks asynchronously */
    while (1)
    {
        usleep(1000U * 1000U);
    }

    /* Not reached in normal operation */
    z_drop(z_move(sub));
    zp_stop_read_task(z_loan_mut(s));
    zp_stop_lease_task(z_loan_mut(s));
    z_drop(z_move(s));
    return 0;
}
