# Seagull in a container

`almalinux-10/` builds Seagull from the current commit (`git archive HEAD`) on
AlmaLinux 10.2 with CMake (Release, crypto, TLS and native SCTP) and the system
OpenSSL. Podman is the default engine; Docker works with `ENGINE=docker`.

    cd docker/almalinux-10
    make test     # build stage + tests/verify.sh inside the container
    make build    # runtime image localhost/seagull:almalinux10
    make run      # start a container in the background
    make exec     # open a shell in it
    make stop

Only committed code goes into the image. The runtime image has the binaries in
`/usr/local/bin` and the environments in `/opt/seagull` (as the original
Dockerfiles did), so the `start_*.ksh` scripts work unchanged. The label
`org.opencontainers.image.revision` holds the commit.

SCTP uses the host kernel: load the module on the host (`sudo modprobe sctp`).
It works in rootless Podman. TLS tests need certificates generated inside the
container with `tests/gen-certs.sh`; keys are never copied into the image.

Validated (rootless Podman 5.8.2, base image
`quay.io/almalinuxorg/almalinux:10.2-20261027`, pinned by digest): TCP, TLS and
SCTP equal to the gnu++98 reference, and Diameter Gx 20/20 in the runtime image.

The previous Dockerfiles are in [`legacy/`](legacy).
