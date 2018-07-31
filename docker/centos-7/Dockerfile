FROM centos:7 as builder

RUN    yum update -y \
    && yum groupinstall -y "Development Tools" \
    && yum install -y curl git glib2-devel ksh bison flex flex-devel

RUN    mkdir -p ~/opt/src \
    && cd ~/opt/src \
    && git clone https://github.com/codeghar/Seagull.git seagull

RUN    mkdir -p ~/opt/src/seagull/seagull/trunk/src/external-lib-src \
    && curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/sctplib-1.0.15.tar.gz http://www.sctp.de/download/sctplib-1.0.15.tar.gz \
    && curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/socketapi-2.2.8.tar.gz http://www.sctp.de/download/socketapi-2.2.8.tar.gz \
    && curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/openssl-1.0.2e.tar.gz https://www.openssl.org/source/openssl-1.0.2e.tar.gz

RUN    cd ~/opt/src/seagull/seagull/trunk/src \
    && ksh build-ext-lib.ksh

RUN    cd ~/opt/src/seagull/seagull/trunk/src \
    && ksh build.ksh -target all

RUN    tar czf /root/bin.tgz ~/opt/src/seagull/seagull/trunk/src/bin/* \
    && tar czf /root/exe-env.tgz ~/opt/src/seagull/seagull/trunk/src/exe-env/* \
    && tar czf /root/pkg.tgz /root/exe-env.tgz /root/bin.tgz

FROM centos:7 as distro
RUN    yum update -y \
    && yum install -y ksh
# RUN    yum update \
#     && yum install -y ksh locales \
#     && yum upgrade -y \
#     && locale-gen en_US.UTF-8 \
#     && dpkg-reconfigure --frontend noninteractive locales \
#     && rm -rf /var/lib/apt/lists/*
COPY --from=builder /root/pkg.tgz /root/pkg.tgz
RUN    tar xzf /root/pkg.tgz -C /root --strip=1 \
    && tar xzf /root/bin.tgz -C /usr/local/bin --strip=8 \
    && mkdir -p /opt/seagull \
    && tar xzf /root/exe-env.tgz -C /opt/seagull --strip=8 \
    && mkdir -p /opt/seagull/diameter-env/logs \
    && mkdir -p /opt/seagull/h248-env/logs \
    && mkdir -p /opt/seagull/http-env/logs \
    && mkdir -p /opt/seagull/msrp-env/logs \
    && mkdir -p /opt/seagull/octcap-env/logs \
    && mkdir -p /opt/seagull/radius-env/logs \
    && mkdir -p /opt/seagull/sip-env/logs \
    && mkdir -p /opt/seagull/synchro-env/logs \
    && mkdir -p /opt/seagull/xcap-env/logs
RUN rm -f /root/*.tgz
