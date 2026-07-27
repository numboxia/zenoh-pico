# zenoh-pico — NuttX / T3 Gemstone O1 (AM67A R5F) Examples

This directory contains NuttX builtin-app examples that run zenoh-pico on the **main-R5FSS0-0** Cortex-R5F core of the **T3 Gemstone O1** board (TI AM67A / J722S SoC).

Network connectivity is provided by **virtio-net** over the Linux remoteproc/virtio IPC channel — no physical Ethernet required on the R5F side. The Linux A53 core acts as the virtio-net backend and bridges traffic to the regular Linux network stack.

## Examples

| App | Description | Key expression |
|-----|-------------|----------------|
| `zenoh_pub` | Publisher — sends a counter every second | `t3/pub/data` |
| `zenoh_sub` | Subscriber — prints received samples | `t3/pub/**` |

Both use **peer mode + UDP multicast** (`udp/224.0.0.224:7446`) by default. No zenoh router is required.

## Requirements

| Tool | Version |
|------|---------|
| `arm-none-eabi-gcc` | ≥ 13 |
| CMake | ≥ 3.25 |
| Python | ≥ 3.10 |
| NuttX source | t3gemstone/nuttx |
| nuttx-apps source | t3gemstone/nuttx-apps |

---

## Build Steps

### 1. Clone the repositories

Clone all three repositories **into the same parent directory**. `zenoh-pico/` must be a sibling of `nuttx/` to match the default `CONFIG_EXAMPLES_ZENOH_PICO_DIR`.

```bash
mkdir t3-workspace && cd t3-workspace

git clone git@github.com:t3gemstone/nuttx.git --branch=zenoh nuttx
git clone git@github.com:t3gemstone/nuttx-apps.git nuttx-apps
git clone git@github.com:t3gemstone/zenoh-pico.git zenoh-pico
```

NuttX resolves the apps tree as `$(TOPDIR)/../apps`. Create a symlink so `apps/` points to `nuttx-apps/`:

```bash
ln -sf $(pwd)/nuttx-apps apps
```

Expected layout:

```
t3-workspace/
├── nuttx/          <- NuttX OS (includes t3-gem-o1 board support)
├── nuttx-apps/     <- nuttx-apps
└── zenoh-pico/     <- this repo
```

### 2. Add the zenoh examples to nuttx-apps

Symlink the zenoh_pub and zenoh_sub directories into the nuttx-apps examples tree:

```bash
ln -sf $(pwd)/zenoh-pico/examples/nuttx/zenoh_pub  nuttx-apps/examples/zenoh_pub
ln -sf $(pwd)/zenoh-pico/examples/nuttx/zenoh_sub  nuttx-apps/examples/zenoh_sub
```

Verify:

```bash
ls nuttx-apps/examples/ | grep zenoh
# zenoh_pub  zenoh_sub
```

### 3. Configure NuttX for t3-gem-o1

```bash
cd nuttx
./tools/configure.sh t3-gem-o1:nsh
```

This applies `boards/arm/am67/t3-gem-o1/configs/nsh/defconfig`, which already enables networking, virtio-net, and the zenoh examples:

```
CONFIG_DRIVERS_VIRTIO=y
CONFIG_DRIVERS_VIRTIO_NET=y
CONFIG_EXAMPLES_ZENOH_PUB=y
CONFIG_EXAMPLES_ZENOH_SUB=y
CONFIG_EXAMPLES_ZENOH_PICO_DIR="$(TOPDIR)/../zenoh-pico"
```

To adjust options:

```bash
make menuconfig
# Application Configuration → Examples:
#   [*] zenoh-pico publisher example
#   [*] zenoh-pico subscriber example
#   ($(TOPDIR)/../zenoh-pico) Path to zenoh-pico source tree
```

### 4. Build

```bash
# From inside nuttx/
make -j$(nproc)
```

Output:
- `nuttx` — ELF (loaded by Linux remoteproc)
- `nuttx.bin` — raw binary

During the build, zenoh-pico source files (`../zenoh-pico/src/*.c`) are compiled directly into the firmware by the example's CMakeLists.txt — there is no separate zenoh-pico build step.

---

## Loading the Firmware

Linux remoteproc loads the R5F ELF image:

```bash
# On Linux A53 (as root):
cp nuttx /lib/firmware/nuttx-r5f0.elf

echo nuttx-r5f0.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start          > /sys/class/remoteproc/remoteproc0/state
```

Confirm the resource table was parsed and virtio devices created:

```bash
dmesg | grep -E "remoteproc|virtio"
# Expected: "virtio0: Features ok"
```

---

## Linux A53 Network Setup

After remoteproc starts the R5F, a `virtio0` interface appears on Linux:

```bash
sudo ip addr add 192.168.10.1/24 dev virtio0
sudo ip link set virtio0 up

# Multicast route for zenoh peer mode:
sudo ip route add 224.0.0.0/4 dev virtio0

# Verify connectivity:
ping -c 3 192.168.10.2
```

**IP addressing** (static, set in defconfig):

| Side | Interface | Address |
|------|-----------|---------|
| R5F | `eth0` | 192.168.10.2/24 |
| Linux | `virtio0` | 192.168.10.1/24 |

---

## NSH Usage

Once NuttX boots and the rptun handshake completes, the NSH prompt appears on UART:

```
nsh> zenoh_pub
[zenoh_pub] mode=peer  keyexpr=t3/pub/data
[zenoh_pub] Publishing on 't3/pub/data' every 1000 ms...
[zenoh_pub] Put '[   0] Hello from NuttX R5F!'
```

```
nsh> zenoh_sub
[zenoh_sub] mode=peer  keyexpr=t3/pub/**
[zenoh_sub] Waiting for data on 't3/pub/**'...
[zenoh_sub] >> ('t3/pub/data': 'Hello from NuttX R5F!')
```

---

## Linux A53 — zenoh-pico CLI (optional)

Cross-compile zenoh-pico for Linux A53 from the development host:

```bash
cmake -B build_aarch64 \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_EXE_LINKER_FLAGS="-static" \
    -DZP_PLATFORM=linux

cmake --build build_aarch64 --target examples -j$(nproc)
```

On the target Linux A53:

```bash
# Subscribe to what the R5F publishes:
./z_sub -m peer -l "udp/224.0.0.224:7446#iface=virtio0" -k "t3/pub/**"

# Publish to the R5F subscriber:
./z_pub -m peer -l "udp/224.0.0.224:7446#iface=virtio0" \
        -k "t3/pub/data" -v "hello from linux"
```

---

## IPC / virtio-net Architecture

```
R5F (NuttX)                          Linux A53
───────────                          ─────────
zenoh-pico (peer mode)               zenoh router / peer
   │ UDP multicast                        │ UDP/IP
BSD sockets (NuttX net stack)        virtio0 (virtual Ethernet)
   │ Ethernet frames                      │
virtio-net driver                    virtio_net.ko
   │ TX/RX vrings (shared DDR)            │ virtqueue
   └──── VRING shared DRAM ───────────────┘
```

### Resource table

The R5F ELF places `.resource_table` at `0xA2100000`. Linux remoteproc reads it at boot:

| Entry | Type | Virtio ID | notifyid | Base address |
|-------|------|-----------|----------|--------------|
| 0 | `RSC_VDEV` | 7 (RPMsg) | 2 | `0xA2200000` |
| 1 | `RSC_VDEV` | 1 (net) | 5 | `0xA2210000` |

### Memory map

| Region | Address | Size | Usage |
|--------|---------|------|-------|
| Resource table | `0xA2100000` | 1 KB | ELF `.resource_table` section |
| RPMsg vring0 | `0xA2200000` | 32 KB | RPMsg TX |
| RPMsg vring1 | `0xA2208000` | 32 KB | RPMsg RX |
| net vring0 | `0xA2210000` | 32 KB | virtio-net TX |
| net vring1 | `0xA2218000` | 32 KB | virtio-net RX |
| Main DDR | `0xA2240000` | ~13.75 MB | NuttX code + data + heap + stack |

All IPC regions must be **non-cached** (MPU `Normal Non-Cacheable`) on the R5F side.

### How Linux and NuttX discover each other

The handshake is driven entirely by Linux remoteproc reading the `.resource_table` ELF section — no out-of-band negotiation happens.

```
Linux (A53)                               NuttX (R5F)
──────────                                ──────────
1. DTS reserved-memory carveout
   prevents kernel from using
   0xA2100000–0xA22FFFFF

2. echo start > remoteproc0/state
   → ELF loaded into DDR
   → .resource_table parsed at 0xA2100000
     ┌─ RSC_VDEV id=7 (RPMsg)  vrings @ 0xA2200000
     └─ RSC_VDEV id=1 (net)    vrings @ 0xA2210000

3. remoteproc creates a virtio_device
   for each RSC_VDEV entry
   → virtio_net.ko probes the net vdev
   → "virtio0" interface appears on Linux

4. R5F core started (jumps to NuttX entry)
                                          5. NuttX boot: am67_rptun_init()
                                             rptun_initialize() passes the same
                                             resource table to OpenAMP
                                             → virtio-net driver binds → "eth0"

Both sides map the same physical DDR
vrings. Linux via DMA coherent mapping,
NuttX via direct physical address access.

6. Mailbox kick (TODO): each side writes
   to a vring, then raises a NAVSS0
   mailbox interrupt to notify the other.
```

### Linux device tree requirements

Three DTS additions are required for this to work. Without them the kernel will either corrupt the shared memory or fail to start the R5F.

**1. `reserved-memory` carveout** — prevents the Linux kernel from allocating pages over the shared DDR regions:

```dts
reserved-memory {
    #address-cells = <2>;
    #size-cells = <2>;
    ranges;

    r5f0_dma_memory: r5f0-dma@a2100000 {
        compatible = "shared-dma-pool";
        reg = <0x00 0xa2100000 0x00 0x200000>; /* 2 MB: rsctable + all vrings */
        no-map;
    };
};
```

**2. remoteproc node** — points the R5F core at the carveout and the firmware image:

```dts
&main_r5fss0_core0 {
    memory-region = <&r5f0_dma_memory>;
    firmware-name = "nuttx-r5f0.elf";
    status = "okay";
};
```

**3. Mailbox bindings** — assigns mailbox0_cluster3 to the main R5F core for the IPC kick mechanism:

```dts
&mailbox0_cluster3 {
    status = "okay";
    mbox_main_r5_0: mbox-main-r5-0 {
        ti,mbox-rx = <0 0 0>;   /* Linux RX: FIFO 0 (R5F→Linux) */
        ti,mbox-tx = <1 0 0>;   /* Linux TX: FIFO 1 (Linux→R5F) */
    };
};

&main_r5fss0_core0 {
    mboxes = <&mailbox0_cluster3 &mbox_main_r5_0>;
    mbox-names = "tx";
};
```

This matches the J722S EVM DTS (`k3-j722s-evm.dts`) exactly. On the R5F side, `am67_rptun.c` uses mailbox0_cluster3 user-3 / VIM IRQ 116, which is what the NAVSS interrupt router wires to R5FSS0 core0 for this cluster.

### What works today vs. what requires the mailbox

| Step | Status | Notes |
|------|--------|-------|
| ELF loaded by remoteproc | ✅ Works | Requires correct DTS `reserved-memory` + remoteproc node |
| `virtio0` appears on Linux | ✅ Works | remoteproc parses RSC_VDEV entries, `virtio_net.ko` probes |
| `eth0` appears on NuttX | ✅ Works | NuttX virtio-net driver binds after `rptun_initialize()` |
| Vring data transfer | ⚠️ Needs test | Mailbox kick implemented (mailbox0_cluster3, VIM IRQ 116) — needs hardware validation |
| `ping 192.168.10.2` | ⚠️ Needs test | Depends on vring transfer working |
| zenoh pub/sub | ⚠️ Needs test | Depends on `ping` working |

---

## Platform Port Files (zenoh-pico)

| File | Description |
|------|-------------|
| `cmake/platforms/nuttx.cmake` | CMake platform profile; selects POSIX transports |
| `include/zenoh-pico/system/platform/nuttx.h` | pthread / socket / timespec type mappings |
| `src/system/nuttx/system.c` | `arc4random_buf`, pthreads, `clock_gettime`, `usleep` |
| `src/link/transport/tcp/tcp_posix.c` | TCP transport |
| `src/link/transport/udp/udp_posix.c` | UDP unicast transport |
| `src/link/transport/udp/udp_multicast_posix.c` | UDP multicast transport |

---

## Current Status

### Implemented

- NuttX platform port for zenoh-pico (POSIX-compatible; all transports working)
- `zenoh_pub` / `zenoh_sub` NuttX builtin apps
- Extended resource table with RPMsg + virtio-net vdevs (`am67_boot.c`)
- `am67_rptun.c` — rptun driver skeleton; registers resource table with NuttX OpenAMP
- defconfig: `CONFIG_DRIVERS_VIRTIO=y` + `CONFIG_DRIVERS_VIRTIO_NET=y`

### NAVSS mailbox notification

`notify()` and the IRQ handler in `arch/arm/src/am67/am67_rptun.c` are fully implemented:

| Direction | Mailbox | FIFO | User | Interrupt |
|-----------|---------|------|------|-----------|
| R5F → Linux (kick A53) | mailbox0_cluster3 (`0x29030000`) | 0 | Linux user-0 | GIC SPI 109 |
| Linux → R5F (kick R5F) | mailbox0_cluster3 (`0x29030000`) | 1 | R5F user-3 | VIM IRQ 116 |

Sources: `k3-j722s-evm.dts` (mbox_main_r5_0) and MCU+ SDK `cslr_intr_r5fss0_core0.h` (IRQ 116 = `CSLR_R5FSS0_CORE0_INTR_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_3`).

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `echo start` fails | DTS carveout mismatch | Verify `reserved-memory` covers `0xA2100000–0xA22FFFFF` |
| `virtio0` does not appear on Linux | Resource table not parsed | Check `dmesg` for remoteproc errors; confirm `.resource_table` section is in the ELF |
| `ping 192.168.10.2` fails | Mailbox kick not implemented | Implement `notify()` in `am67_rptun.c` |
| `zenoh_pub` → `z_open failed` | virtio-net not up on Linux | Run `ip addr add ... dev virtio0 && ip link set virtio0 up` |
| `z_open failed` in peer mode | Multicast route missing | `sudo ip route add 224.0.0.0/4 dev virtio0` |
| NSH prompt does not appear | Serial console misconfigured | UART0 at `0x2810000`, 115200 8N1 |
| Build error: `zenoh-pico not found` | Wrong `ZENOH_PICO_DIR` | `make menuconfig` → set zenoh-pico path |
| `zenoh_pub` not found in NSH | `CONFIG_EXAMPLES_ZENOH_PUB` not set | `make menuconfig` → enable the example; rebuild |
