Seagull
========================================================

Introduction
--------------------------------------------------------

::

    Seagull is a free, Open Source (GPL) multi-protocol traffic generator test tool. Primarily aimed at IMS (3GPP, TISPAN, CableLabs) protocols (and thus being the perfect complement to SIPp for IMS testing), Seagull is a powerful traffic generator for functional, load, endurance, stress and performance/benchmark tests for almost any kind of protocol.

That was a quote from the `Seagull <http://gull.sourceforge.net/>_ website.

This repo was cloned from `Seagull's svn repo on SourceForge <https://svn.code.sf.net/p/gull/code/>`_ on 2015-04-22.

Goal of this Repo
--------------------------------------------------------

I wanted to build and use Seagull on Ubuntu for Diameter protocol testing. I came across a lot of roadblocks that needed to be surmounted, primarily compiler errors when I tried to build it on anything modern, say CentOS 7 or Ubuntu 14.04.

The primary goal with this repo is to provide patches to the Subversion revision 422 of Seagull to make it build and work on Ubuntu. To get it to work on CentOS 7 is a "nice to have".

Run `svn info https://svn.code.sf.net/p/gull/code/` and you'll see the following information that the code hasn't been updated in quite a while.

::

    Path: code
    URL: https://svn.code.sf.net/p/gull/code
    Relative URL: ^/
    Repository Root: https://svn.code.sf.net/p/gull/code
    Repository UUID: 9920205e-2714-0410-b6de-9c83873d791f
    Revision: 422
    Node Kind: directory
    Last Changed Author: muralidharbm
    Last Changed Rev: 422
    Last Changed Date: 2013-07-03 03:06:53 -0700 (Wed, 03 Jul 2013)

Directory Structure
--------------------------------------------------------

Two directories that came directly from the SVN to Git clone are doc and seagull. Everything else was added by me.

Build Instructions During Investigation
--------------------------------------------------------

This section recounts how I built Seagull on Ubuntu 15.04. You can follow the steps and hopefully be able to build successfully as well.

First Steps
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Install pre-requisites.

::

    user@host:~$ sudo aptitude install build-essential curl git git-svn libglib2.0-dev ksh bison flex subversion

Find information about the latest revision.

::

    user@host:~$ svn info https://svn.code.sf.net/p/gull/code/

Clone the Seagull svn repo into a git repo. This is a personal preference as I like to work with git and it allowed me to share this repo on GitHub.

::

    user@host:~$ git svn clone svn://svn.code.sf.net/p/gull/code/ ~/opt/src/seagull
    user@host:~$ cd ~/opt/src/seagull
    user@host:~/opt/src/seagull$ git branch build master
    user@host:~/opt/src/seagull$ git checkout build

Add SCTP Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Download the latest sctplib and socketapi tarballs from http://www.sctp.de/ to the external-lib-src directory. They were are version 1.0.15 and 2.2.8 respectively at the time of writing.

::

    user@host:~$ cd ~/opt/src/seagull/seagull/trunk/src
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/sctplib-1.0.15.tar.gz http://www.sctp.de/download/sctplib-1.0.15.tar.gz
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/socketapi-2.2.8.tar.gz http://www.sctp.de/download/socketapi-2.2.8.tar.gz

Edit build-ext-lib.conf and update the versions of these two libraries. First (optional) run a diff to make sure things look OK. Then just run `sed` to make the changes. Adjust the versions if you downloaded different versions.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ diff build-ext-lib.conf <(sed -e 's/sctplib-1.0.5/sctplib-1.0.15/g' -e 's/socketapi-1.9.0/socketapi-2.2.8/g' build-ext-lib.conf)
    74,75c74,75
    < EXTBUILD_1_FILE=sctplib-1.0.5.tar.gz
    < EXTBUILD_1_DIR=sctplib-1.0.5
    ---
    > EXTBUILD_1_FILE=sctplib-1.0.15.tar.gz
    > EXTBUILD_1_DIR=sctplib-1.0.15
    82,83c82,83
    < EXTBUILD_2_FILE=socketapi-1.9.0.tar.gz
    < EXTBUILD_2_DIR=socketapi-1.9.0
    ---
    > EXTBUILD_2_FILE=socketapi-2.2.8.tar.gz
    > EXTBUILD_2_DIR=socketapi-2.2.8

    user@host:~/opt/src/seagull/seagull/trunk/src$ sed -i -e 's/sctplib-1.0.5/sctplib-1.0.15/g' -e 's/socketapi-1.9.0/socketapi-2.2.8/g' build-ext-lib.conf

Make sure in the *build.conf* file all lines starting with *BUILD_9* are uncommented to add SCTP support.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sed -i -e 's/^#BUILD_9/BUILD_9/g' build.conf

Add TLS Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Download the `latest OpenSSL tarball <https://www.openssl.org/source/>`_ to the external-lib-src directory. It was 1.0.2a at the time of writing.

::

    user@host:~$ cd ~/opt/src/seagull/seagull/trunk/src
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/openssl-1.0.2a.tar.gz https://www.openssl.org/source/openssl-1.0.2a.tar.gz
    
Edit build-ext-lib.conf and update the version of this library. Adjust the version if you downloaded a different version.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ diff build-ext-lib.conf <(sed -e 's/openssl-0.9.8c/openssl-1.0.2a/g' build-ext-lib.conf)
    90,91c90,91
    < EXTBUILD_3_FILE=openssl-0.9.8c.tar.gz
    < EXTBUILD_3_DIR=openssl-0.9.8c
    ---
    > EXTBUILD_3_FILE=openssl-1.0.2a.tar.gz
    > EXTBUILD_3_DIR=openssl-1.0.2a

    user@host:~/opt/src/seagull/seagull/trunk/src$ sed -i -e 's/openssl-0.9.8c/openssl-1.0.2a/g' build-ext-lib.conf

Make sure in the *build.conf* file all lines starting with *BUILD_10* are uncommented to add TLS support.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sed -i -e 's/^#BUILD_10/BUILD_10/g' build.conf

Build SCTP and TLS Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Build libraries for SCTP and TLS.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo aptitude install ksh bison flex
    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build-ext-lib.ksh


Disable HP OpenCall TCAP Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ diff build.conf <(sed -e 's/^BUILD_TCAP/#BUILD_TCAP/g' -e 's/^BUILD_8/#BUILD_8/g' build.conf)
    144,147c144,147
    < BUILD_TCAP_LIB_CC_LINUX=g++
    < BUILD_TCAP_LIB_CC_FLAGS_LINUX="-D_GNU_SOURCE -DTCAP_API100 -D_REENTRANT -shared -Wall -fPIC -Werror -I/opt/OC/include -I./octcap_include"
    < BUILD_TCAP_LIB_LD_LINUX=g++
    < BUILD_TCAP_LIB_LD_FLAGS_LINUX="-shared -fPIC"
    ---
    > #BUILD_TCAP_LIB_CC_LINUX=g++
    > #BUILD_TCAP_LIB_CC_FLAGS_LINUX="-D_GNU_SOURCE -DTCAP_API100 -D_REENTRANT -shared -Wall -fPIC -Werror -I/opt/OC/include -I./octcap_include"
    > #BUILD_TCAP_LIB_LD_LINUX=g++
    > #BUILD_TCAP_LIB_LD_FLAGS_LINUX="-shared -fPIC"
    149c149
    < BUILD_TCAP_LIB_LD_LIBS_LINUX="-lm -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    ---
    > #BUILD_TCAP_LIB_LD_LIBS_LINUX="-lm -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    202,207c202,207
    < BUILD_TCAP_LIB_CC_HP_UX=aCC
    < BUILD_TCAP_LIB_CC_FLAGS_HP_UX="-Aa +Z +DAportable +p -D_HPUX_SOURCE -DTCAP_API100 -D_XOPEN_SOURCE=500  -D_XOPEN_SOURCE_EXTENDED `check_dlopen_usage` -I/opt/OC/include -I./octcap_include"
    < BUILD_TCAP_LIB_CC_FLAGS_HP_UX_IA64="-AP -Aa +Z +p +DD64 -D_HPUX_SOURCE -DTCAP_API100 -D_XOPEN_SOURCE=500  -D_XOPEN_SOURCE_EXTENDED `check_dlopen_usage` -I/opt/OC/include -I./octcap_include"
    < BUILD_TCAP_LIB_LD_HP_UX=aCC
    < BUILD_TCAP_LIB_LD_FLAGS_HP_UX="-b"
    < BUILD_TCAP_LIB_LD_FLAGS_HP_UX_IA64="-b +DD64"
    ---
    > #BUILD_TCAP_LIB_CC_HP_UX=aCC
    > #BUILD_TCAP_LIB_CC_FLAGS_HP_UX="-Aa +Z +DAportable +p -D_HPUX_SOURCE -DTCAP_API100 -D_XOPEN_SOURCE=500  -D_XOPEN_SOURCE_EXTENDED `check_dlopen_usage` -I/opt/OC/include -I./octcap_include"
    > #BUILD_TCAP_LIB_CC_FLAGS_HP_UX_IA64="-AP -Aa +Z +p +DD64 -D_HPUX_SOURCE -DTCAP_API100 -D_XOPEN_SOURCE=500  -D_XOPEN_SOURCE_EXTENDED `check_dlopen_usage` -I/opt/OC/include -I./octcap_include"
    > #BUILD_TCAP_LIB_LD_HP_UX=aCC
    > #BUILD_TCAP_LIB_LD_FLAGS_HP_UX="-b"
    > #BUILD_TCAP_LIB_LD_FLAGS_HP_UX_IA64="-b +DD64"
    209c209
    < BUILD_TCAP_LIB_LD_LIBS_HP_UX="-lm -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    ---
    > #BUILD_TCAP_LIB_LD_LIBS_HP_UX="-lm -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    211c211
    < BUILD_TCAP_LIB_LD_LIBS_HP_UX_IA64="-lm -lxnet -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    ---
    > #BUILD_TCAP_LIB_LD_LIBS_HP_UX_IA64="-lm -lxnet -L/opt/OC/lib -lntl -lTTL -lTTLBase"
    323,325c323,325
    < BUILD_8_NAME="libtrans_octcap.so"
    < BUILD_8_TYPE=tcap_lib
    < BUILD_8_MODULES="common transport-frame protocol-frame library-trans-octcap32"
    ---
    > #BUILD_8_NAME="libtrans_octcap.so"
    > #BUILD_8_TYPE=tcap_lib
    > #BUILD_8_MODULES="common transport-frame protocol-frame library-trans-octcap32"

    user@host:~/opt/src/seagull/seagull/trunk/src$ sed -i -e 's/^BUILD_TCAP/#BUILD_TCAP/g' -e 's/^BUILD_8/#BUILD_8/g' build.conf

Build Seagull
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

First clean up previous build attempts and then build. Upon encountering errors, as you surely will without the patches, fix the error and run these two commands again.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build.ksh -target clean
    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build.ksh -target all

First error.

::

    [Compiling library-trans-extsctp/C_SocketSCTP.cpp]
    library-trans-extsctp/C_SocketSCTP.cpp: In member function ‘void C_SocketSCTPWithData::sctp_event_handler(C_TransportEvent*)’:
    library-trans-extsctp/C_SocketSCTP.cpp:429:29: error: variable ‘ssf’ set but not used [-Werror=unused-but-set-variable]
       struct sctp_send_failed  *ssf;
                                 ^
    library-trans-extsctp/C_SocketSCTP.cpp:431:29: error: variable ‘sre’ set but not used [-Werror=unused-but-set-variable]
       struct sctp_remote_error *sre;
                                 ^
    library-trans-extsctp/C_SocketSCTP.cpp:435:23: error: variable ‘sin’ set but not used [-Werror=unused-but-set-variable]
       struct sockaddr_in *sin;
                           ^
    library-trans-extsctp/C_SocketSCTP.cpp:436:24: error: variable ‘sin6’ set but not used [-Werror=unused-but-set-variable]
       struct sockaddr_in6 *sin6;
                            ^
    cc1plus: all warnings being treated as errors
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/dep-libtrans_extsctp.so.mk:82: recipe for target '/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/C_SocketSCTP.o' failed
    make[1]: *** [/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/C_SocketSCTP.o] Error 1
    make[1]: Leaving directory '/home/myusername/opt/src/seagull/seagull/trunk/src'
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/project.mk:66: recipe for target 'all_libtrans_extsctp.so' failed
    make: *** [all_libtrans_extsctp.so] Error 2

Fix for first error.

::

    diff --git a/seagull/trunk/src/library-trans-extsctp/C_SocketSCTP.cpp b/seagull/trunk/src/library-trans-extsctp/C_SocketSCTP.cpp
    index 777924e..25e0747 100755
    --- a/seagull/trunk/src/library-trans-extsctp/C_SocketSCTP.cpp
    +++ b/seagull/trunk/src/library-trans-extsctp/C_SocketSCTP.cpp
    @@ -426,14 +426,10 @@ void C_SocketSCTPWithData::set_properties() {
     void C_SocketSCTPWithData::sctp_event_handler (C_TransportEvent *P_event) {
     
       struct sctp_assoc_change *sac;
    -  struct sctp_send_failed  *ssf;
       struct sctp_paddr_change *spc;
    -  struct sctp_remote_error *sre;
       union  sctp_notification *snp;
       //  char   addrbuf[INET6_ADDRSTRLEN];
       //  const  char *ap;
    -  struct sockaddr_in *sin;
    -  struct sockaddr_in6 *sin6;
     
       snp = (union sctp_notification *)m_read_buf;
     
    @@ -511,14 +507,6 @@ void C_SocketSCTPWithData::sctp_event_handler (C_TransportEvent *P_event) {
     
           break;
     
    -  case SCTP_SEND_FAILED:
    -    ssf = &snp->sn_send_failed;
    -    SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " << 
    -        "EVENT sendfailed: len="
    -        << ssf->ssf_length
    -        << " err=" << ssf->ssf_error);
    -    break;
    -
       case SCTP_PEER_ADDR_CHANGE:
         spc = &snp->sn_paddr_change;
     
    @@ -553,22 +541,10 @@ void C_SocketSCTPWithData::sctp_event_handler (C_TransportEvent *P_event) {
           break ;
         } /* end switch */
         
    -    if (spc->spc_aaddr.ss_family == AF_INET) {
    -      sin = (struct sockaddr_in *)&spc->spc_aaddr;
    -      //    ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf, INET6_ADDRSTRLEN);
    -    } else {
    -      sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
    -      //      ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    -    }
         //    printf("EVENT intf_change: %s state=%d, error=%d\n", ap, spc->spc_state, spc->spc_error);
         //    printf("EVENT intf_change: state=%d, error=%d\n", spc->spc_state, spc->spc_error);
         break;
     
    -  case SCTP_REMOTE_ERROR:
    -    sre = &snp->sn_remote_error;
    -    //    printf("EVENT: remote_error: err=%hu len=%hu\n", ntohs(sre->sre_error), ntohs(sre->sre_length));
    -    break;
    -
       case SCTP_ADAPTATION_INDICATION:
         SOCKET_DEBUG(0, "C_SocketSCTPWithData::sctp_event_handler() " <<
             "EVENT: ADAPTATION INDICATION");

Second error after fixing the first error.

::

    [Compiling library-trans-tls/C_TransIPTLS.cpp]
    library-trans-tls/C_TransIPTLS.cpp:36:1: error: invalid conversion from ‘const SSL_METHOD* (*)() {aka const ssl_method_st* (*)()}’ to ‘C_TransIPTLS::T_SSLMethodType {aka ssl_method_st* (*)()}’ [-fpermissive]
     } ;
     ^
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/dep-libtrans_iptls.so.mk:80: recipe for target '/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/C_TransIPTLS.o' failed
    make[1]: *** [/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/C_TransIPTLS.o] Error 1
    make[1]: Leaving directory '/home/myusername/opt/src/seagull/seagull/trunk/src'
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/project.mk:74: recipe for target 'all_libtrans_iptls.so' failed
    make: *** [all_libtrans_iptls.so] Error 2

Fix for the second error.

::

    diff --git a/seagull/trunk/src/library-trans-tls/C_TransIPTLS.hpp b/seagull/trunk/src/library-trans-tls/C_TransIPTLS.hpp
    index 83884ab..8231550 100644
    --- a/seagull/trunk/src/library-trans-tls/C_TransIPTLS.hpp
    +++ b/seagull/trunk/src/library-trans-tls/C_TransIPTLS.hpp
    @@ -38,7 +38,7 @@ public:
     
       virtual int         config (T_pConfigValueList P_config_param_list) ;
     
    -  typedef SSL_METHOD* (*T_SSLMethodType)(void); 
    +  typedef const SSL_METHOD* (*T_SSLMethodType)(void);
       typedef struct _T_supported_methods {
         char *m_name ;
         T_SSLMethodType m_method ;


Third error after fixing the second error.

::

    [Compiling library-crypto/auth.c]
    library-crypto/auth.c: In function ‘int createAuthHeaderAKAv1MD5(char*, char*, char*, char*, char*, char*, char*, char*, char*, char*)’:
    library-crypto/auth.c:502:21: error: variable ‘resuf’ set but not used [-Werror=unused-but-set-variable]
       int has_auts = 0, resuf = 1;
                         ^
    cc1plus: all warnings being treated as errors
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/dep-lib_crypto.so.mk:54: recipe for target '/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/auth.o' failed
    make[1]: *** [/home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/auth.o] Error 1
    make[1]: Leaving directory '/home/myusername/opt/src/seagull/seagull/trunk/src'
    /home/myusername/opt/src/seagull/seagull/trunk/src/work-1.8.3/project.mk:82: recipe for target 'all_lib_crypto.so' failed
    make: *** [all_lib_crypto.so] Error 2

Fix for the third error.

::

    diff --git a/seagull/trunk/src/library-crypto/auth.c b/seagull/trunk/src/library-crypto/auth.c
    index 8f434b8..ca3eb39 100644
    --- a/seagull/trunk/src/library-crypto/auth.c
    +++ b/seagull/trunk/src/library-crypto/auth.c
    @@ -499,7 +499,7 @@ int createAuthHeaderAKAv1MD5(char * user, char * aka_OP,
                            
       char tmp[MAX_HEADER_LEN] ; 
       char *start, *end;
    -  int has_auts = 0, resuf = 1;
    +  int has_auts = 0;
       char *nonce64, *nonce;
       int noncelen;
       RESHEX resp_hex;
    @@ -572,7 +572,7 @@ int createAuthHeaderAKAv1MD5(char * user, char * aka_OP,
           resp_hex[2*i+1]=hexa[res[i]&0x0F];
         }
         resp_hex[RESLEN*2]=0;
    -    resuf = createAuthHeaderMD5(user, resp_hex, method, uri, msgbody, auth, algo, result);   
    +    createAuthHeaderMD5(user, resp_hex, method, uri, msgbody, auth, algo, result);
       } else {
         sqn_ms[5] = sqn_he[5] + 1;
         f5star(k, rnd, ak, op);
    @@ -582,7 +582,7 @@ int createAuthHeaderAKAv1MD5(char * user, char * aka_OP,
         has_auts = 1;
         /* When re-synchronisation occurs an empty password has to be used */
         /* to compute MD5 response (Cf. rfc 3310 section 3.2) */
    -    resuf=createAuthHeaderMD5(user, (char *)"",method,uri,msgbody,auth,algo,result);
    +    createAuthHeaderMD5(user, (char *)"",method,uri,msgbody,auth,algo,result);
       }
       if (has_auts) {
         /* Format data for output in the SIP message */

Install Seagull
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Install Seagull binaries by copying files in ~/opt/src/seagull/seagull/trunk/src/bin to /usr/local/bin.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo cp ~/opt/src/seagull/seagull/trunk/src/bin/* /usr/local/bin

Following the lead of `rpm packages <http://sourceforge.net/projects/gull/files/>`_ provided by the official projects, install other files in the /opt hierarchy.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo mkdir -p /opt/seagull
    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo cp -r ~/opt/src/seagull/seagull/trunk/src/exe-env/* /opt/seagull

To run a client or server, `cd` to /opt/seagull/\*/run dirctory and execute any start\*.ksh file. For example, to start a Diameter server

::

    user@host:~$ cd /opt/seagull/diameter-env/run
    user@host:/opt/seagull/diameter-env/run$ ./start_server.ksh
