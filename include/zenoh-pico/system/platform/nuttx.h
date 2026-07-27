// Copyright (c) 2022 ZettaScale Technology
// Copyright (c) 2026 T3 Foundation (www.t3gemstone.org)
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Platform types for NuttX RTOS (POSIX-compatible, BSD sockets, pthreads).
// Network backend: rpmsgdrv virtual Ethernet over RPMsg to Linux A53.

#ifndef ZENOH_PICO_SYSTEM_NUTTX_TYPES_H
#define ZENOH_PICO_SYSTEM_NUTTX_TYPES_H

#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#if Z_FEATURE_MULTI_THREAD == 1
typedef pthread_t      _z_task_t;
typedef pthread_attr_t z_task_attr_t;
typedef pthread_mutex_t _z_mutex_t;
typedef pthread_mutex_t _z_mutex_rec_t;
typedef pthread_cond_t  _z_condvar_t;
typedef pthread_t       _z_task_id_t;
#endif  // Z_FEATURE_MULTI_THREAD == 1

typedef struct timespec z_clock_t;
typedef struct timespec z_time_t;

typedef struct {
    union {
#if defined(ZP_PLATFORM_SOCKET_LINKS_ENABLED)
        int _socket;
#endif
    };
} _z_sys_net_socket_t;

typedef struct {
    union {
#if defined(ZP_PLATFORM_SOCKET_LINKS_ENABLED)
        struct addrinfo *_iptcp;
#endif
    };
} _z_sys_net_endpoint_t;

#if Z_FEATURE_LINK_BLUETOOTH == 1
#error "Bluetooth not supported on NuttX port of Zenoh-Pico"
#endif

#if Z_FEATURE_LINK_SERIAL == 1
#error "Serial not supported on NuttX port of Zenoh-Pico"
#endif

#if Z_FEATURE_RAWETH_TRANSPORT == 1
#error "Raw Ethernet transport not supported on NuttX port of Zenoh-Pico"
#endif

#ifdef __cplusplus
}
#endif

#endif  // ZENOH_PICO_SYSTEM_NUTTX_TYPES_H
