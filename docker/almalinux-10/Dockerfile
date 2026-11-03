# Seagull (rama modernizacion) sobre AlmaLinux 10.2
# Base: quay.io/almalinuxorg/almalinux:10.2-20261027
ARG BASE=quay.io/almalinuxorg/almalinux@sha256:ef0a7395168d3c8750237f78d0029dfa4ea520f0da17ed9b51750a54139f1108

FROM ${BASE} AS build
RUN dnf install -y --setopt=install_weak_deps=False gcc-c++ make cmake ninja-build bison flex openssl-devel lksctp-tools-devel && dnf clean all
COPY seagull/trunk/src /src
WORKDIR /src
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSEAGULL_BUILD_CRYPTO=ON -DSEAGULL_BUILD_TLS=ON -DSEAGULL_BUILD_SCTP=ON && cmake --build build -j"$(nproc)"

FROM build AS test
RUN dnf install -y --setopt=install_weak_deps=False procps-ng iproute diffutils openssl ksh lksctp-tools && dnf clean all
ENV SEAGULL_TEST_DIR=/work

FROM ${BASE} AS runtime
ARG SEAGULL_COMMIT=unknown
LABEL org.opencontainers.image.title="Seagull" org.opencontainers.image.revision="${SEAGULL_COMMIT}" org.opencontainers.image.licenses="GPL-2.0-or-later" org.opencontainers.image.base.name="quay.io/almalinuxorg/almalinux:10.2-20261027"
RUN dnf install -y --setopt=install_weak_deps=False libstdc++ openssl-libs zlib-ng-compat ksh procps-ng && dnf clean all
COPY --from=build /src/build/bin/ /usr/local/bin/
COPY --from=build /src/exe-env/ /opt/seagull/
RUN for d in /opt/seagull/*-env; do mkdir -p "$d/logs"; done
WORKDIR /opt/seagull
CMD ["/bin/bash"]
