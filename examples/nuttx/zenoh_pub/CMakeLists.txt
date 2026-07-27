# ##############################################################################
# examples/nuttx/zenoh_pub/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# zenoh-pico publisher example for NuttX (TI AM67A R5F, virtio-net networking).
#
# This file is meant to be placed (or symlinked) under nuttx-apps/examples/:
#   ln -sf /path/to/zenoh-pico/examples/nuttx/zenoh_pub \
#          /path/to/nuttx-apps/examples/zenoh_pub
#
# Kconfig knobs:
#   CONFIG_EXAMPLES_ZENOH_PUB        enable this app
#   CONFIG_EXAMPLES_ZENOH_PICO_DIR   path to the zenoh-pico source tree
#   CONFIG_EXAMPLES_ZENOH_PUB_PRIORITY / STACKSIZE
# ##############################################################################

if(CONFIG_EXAMPLES_ZENOH_PUB)

  set(ZENOH_PICO_DIR "${CONFIG_EXAMPLES_ZENOH_PICO_DIR}")

  file(GLOB_RECURSE ZENOH_PICO_SRCS "${ZENOH_PICO_DIR}/src/*.c")

  list(APPEND ZENOH_PICO_SRCS
       "${ZENOH_PICO_DIR}/src/system/nuttx/system.c"
       "${ZENOH_PICO_DIR}/src/link/transport/tcp/tcp_posix.c"
       "${ZENOH_PICO_DIR}/src/link/transport/udp/udp_posix.c"
       "${ZENOH_PICO_DIR}/src/link/transport/udp/udp_multicast_posix.c")

  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/system/unix/.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/system/ti_am67a/.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/system/ti_am64x/.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/transport/serial/.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/transport/bt/.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*raweth.*")
  list(FILTER ZENOH_PICO_SRCS EXCLUDE REGEX ".*/system/nuttx/system.c$")

  nuttx_add_application(
    NAME      zenoh_pub
    SRCS      "${CMAKE_CURRENT_SOURCE_DIR}/zenoh_pub_main.c"
              ${ZENOH_PICO_SRCS}
    INCLUDE_DIRECTORIES
              "${ZENOH_PICO_DIR}/include"
    DEFINITIONS
              ZENOH_NUTTX
              Z_FEATURE_LINK_TCP=1
              Z_FEATURE_LINK_UDP_UNICAST=1
              Z_FEATURE_LINK_UDP_MULTICAST=1
              Z_FEATURE_MULTI_THREAD=1
              Z_FEATURE_PUBLICATION=1
              Z_FEATURE_SUBSCRIPTION=0
  )

endif()
