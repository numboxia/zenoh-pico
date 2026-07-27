# NuttX platform for zenoh-pico.
# Uses NuttX POSIX APIs (pthreads, BSD sockets) and NuttX's native TCP/IP stack.
# Network connectivity is provided by the rpmsgdrv virtual Ethernet over RPMsg.

set(ZP_PLATFORM_SYSTEM_LAYER nuttx)
set(ZP_PLATFORM_COMPILE_DEFINITIONS ZENOH_NUTTX)
set(ZP_PLATFORM_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/system/nuttx/system.c"
    "${PROJECT_SOURCE_DIR}/src/link/transport/tcp/tcp_posix.c"
    "${PROJECT_SOURCE_DIR}/src/link/transport/udp/udp_posix.c")
if(ZP_UDP_MULTICAST_ENABLED)
  list(APPEND ZP_PLATFORM_SOURCE_FILES
       "${PROJECT_SOURCE_DIR}/src/link/transport/udp/udp_multicast_posix.c")
endif()
set(CHECK_THREADS OFF)
