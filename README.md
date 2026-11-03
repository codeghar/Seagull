# Seagull

## Introduction

    Seagull is a free, Open Source (GPL) multi-protocol traffic generator
    test tool. Primarily aimed at IMS (3GPP, TISPAN, CableLabs) protocols
    (and thus being the perfect complement to SIPp for IMS testing),
    Seagull is a powerful traffic generator for functional, load,
    endurance, stress and performance/benchmark tests for almost
    any kind of protocol.

That was a quote from the [Seagull](http://gull.sourceforge.net/) website.

This repo was cloned from [Seagull's svn repo on SourceForge](https://svn.code.sf.net/p/gull/code/) on 2015-04-22.

## Goal of this Repo

C++23 Modernization for Seagull (Diameter Testing on AlmaLinux 10.2)

Driven by the need to perform Diameter protocol testing using Seagull on AlmaLinux 10.2, I started this project to update and modernize parts of the codebase using C++23.

While there is still work ahead and this is an ongoing effort, this repository represents a solid first step toward bringing the legacy code up to date with a more modern version of the language. Contributions and feedback are welcome!
Several legacy commands and configurations originally intended for other Linux distributions have been kept intact, with plans to update and optimize them over time. It is worth emphasizing that this project has been iterated and refined using AlmaLinux 10.2 as the primary target environment.

## Origin

The `doc/` and `seagull/` directories come from the original Seagull Subversion repository (trunk, later than the 1.8.2 release) and were imported into Git without changes. Everything else (CMake build, test harness, Diameter Gx support and fixes) was added afterwards as part of this modernization effort.

The original code is licensed under the GNU General Public License v2 or later, (c) 2006 Hewlett-Packard Development Company, LP. The last upstream revision is 422:

    URL: https://svn.code.sf.net/p/gull/code
    Revision: 422
    Last Changed Author: muralidharbm
    Last Changed Date: 2013-07-03 03:06:53 -0700 (Wed, 03 Jul 2013)

## Build Instructions (CMake)

The `modernizacion` branch replaces the legacy `build.ksh` / `build-ext-lib.ksh` scripts
(kept in `seagull/trunk/src/legacy/` for reference) with CMake. It builds with C++98 up to
C++23 (default C++23), OpenSSL 3 and native Linux kernel SCTP.

Tested on AlmaLinux 10.2 (GCC 14, OpenSSL 3.5.8).

### Prerequisites

AlmaLinux 10.2 (the CRB repository is needed for `flex-devel`):

    sudo dnf install -y dnf-plugins-core
    sudo dnf config-manager --set-enabled crb
    sudo dnf group install "Development Tools"
    sudo dnf install -y cmake make git
    sudo dnf install -y clang clang-tools-extra
    sudo dnf install -y openssl-devel libxml2-devel lksctp-tools-devel
    sudo dnf install -y flex flex-devel bison perl-core ksh

Optional, for debugging and for `SEAGULL_SANITIZE=ON` (ASan/UBSan):

    sudo dnf install -y gdb valgrind libasan libubsan

`perl-core` is only needed to build OpenSSL from source, and `ksh` to run the
`start*.ksh` scripts of the example environments.

### Build

    cd seagull/trunk/src
    cmake -S . -B build -DSEAGULL_BUILD_TLS=ON -DSEAGULL_BUILD_CRYPTO=ON -DSEAGULL_BUILD_SCTP=ON
    cmake --build build -j"$(nproc)"

The executable and the plugins (`.so`) are generated in `build/bin`.

| Option | Default | Description |
|---|---|---|
| `SEAGULL_CXX_STD` | `23` | C++ standard: 98, 11, 14, 17, 20 or 23 |
| `SEAGULL_BUILD_TLS` | `OFF` | TLS transport plugin (`libtrans_iptls.so`) |
| `SEAGULL_BUILD_CRYPTO` | `OFF` | Crypto plugin (`lib_crypto.so`: Digest, AKA, Radius) |
| `SEAGULL_BUILD_SCTP` | `OFF` | SCTP transport plugin (`libtrans_extsctp.so`) |
| `SEAGULL_WERROR` | `OFF` | Treat warnings as errors |
| `SEAGULL_SANITIZE` | `OFF` | AddressSanitizer and UndefinedBehaviorSanitizer |
| `CMAKE_BUILD_TYPE` | Baseline | Use `Release` for load tests |

CMake remembers `-D` options in an existing build directory: turn them off explicitly
(`-DSEAGULL_BUILD_TLS=OFF`) or use a new directory.

### OpenSSL

If `openssl-devel` is installed, CMake uses the system OpenSSL. Otherwise it builds OpenSSL
from `external-lib-src/openssl-<version>.tar.gz` (`SEAGULL_OPENSSL_VERSION`, default 3.5.8),
verified with SHA256. The tarball is not stored in this repository; download it from the
official release:

    cd seagull/trunk/src
    V=3.5.8
    curl -fL --create-dirs -o external-lib-src/openssl-$V.tar.gz \
      https://github.com/openssl/openssl/releases/download/openssl-$V/openssl-$V.tar.gz
    echo "a8f84a39918ec6415ce765d9b429d313ba97b8143169c172e734b9514464f5b2  external-lib-src/openssl-$V.tar.gz" | sha256sum -c

For servers without internet access, download it on another machine and copy it with `scp`
to `seagull/trunk/src/external-lib-src/`. Building OpenSSL from source requires `perl-core`;
on small VMs use `cmake --build build -j1` to avoid running out of memory. To use another
version, set `SEAGULL_OPENSSL_VERSION` and update `URL_HASH` in `CMakeLists.txt`.

### SCTP

The SCTP plugin uses the native Linux kernel SCTP (no `sctplib` / `socketapi`); only the
`<netinet/sctp.h>` header from `lksctp-tools-devel` is needed to build. At runtime the
`sctp` kernel module must be loaded (no reboot needed; it is blacklisted by default on RHEL):

    sudo modprobe sctp
    checksctp            # must print "SCTP supported"

Use the default socket buffer (65535) or larger. With small buffers (for example
`buffer=4096` in `open-args`), the kernel SCTP window (~4 KB), delayed SACK and
`rto_min` (1 s) limit throughput to about 25 messages/s, and queued messages time out.

### Write flow control

When the peer stops reading and the socket send buffer is full (`EAGAIN`), messages
are queued per socket (up to 4 MB) and sent in order once the socket becomes writable.
Previously they were silently dropped. If the limit is reached, the new message is
discarded and `write queue full` is printed on stderr. Queued data of a channel that
is being closed is discarded.

### Tests

A portable test harness (TCP, TLS 1.3, SCTP, load, sanitizers) is in
[`seagull/trunk/src/tests`](seagull/trunk/src/tests/README.md). Quick check:

    cd seagull/trunk/src
    tests/verify.sh "$PWD/build/bin"

## Install Seagull

Copy everything in `build/bin`. The executable and the plugins must stay in the same
directory (the executable finds them through its `$ORIGIN` RPATH).

    sudo cp seagull/trunk/src/build/bin/* /usr/local/bin

Following the lead of [rpm packages](http://sourceforge.net/projects/gull/files/) provided
by the official project, install the other required files in the /opt hierarchy.

    sudo mkdir -p /opt/seagull
    sudo cp -r seagull/trunk/src/exe-env/* /opt/seagull
    sudo chown -R "$USER":"$USER" /opt/seagull
    mkdir -p /opt/seagull/{diameter-env,h248-env,http-env,msrp-env,octcap-env,radius-env,sip-env,synchro-env,xcap-env}/logs

To run a client or server, `cd` to `/opt/seagull/*/run` and execute any `start*.ksh` file.
For example, to start a Diameter server:

    cd /opt/seagull/diameter-env/run
    ./start_server.ksh

## Build in Docker

`docker/almalinux-10` builds a container image (AlmaLinux 10.2, CMake, system OpenSSL,
native SCTP) from the current commit with Podman or Docker. See
[`docker/README.md`](docker/README.md). The original Dockerfiles are kept in
`docker/legacy` for reference.

## Investigation

`investigation.rst` is a historical record by Hamza Sheikh (@aikchar) of how Seagull
was first built from the SVN sources on Ubuntu 15.04 (before CMake). It does not apply
to the `modernizacion` branch; see *Build Instructions (CMake)* above.

## Diameter Gx

- [Diameter Gx](seagull/trunk/src/exe-env/diameter-env/README-gx.md): diccionario, escenarios, validacion y pruebas entre VMs.
