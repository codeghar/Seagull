Seagull
========================================================

Introduction
--------------------------------------------------------

::

    Seagull is a free, Open Source (GPL) multi-protocol traffic generator
    test tool. Primarily aimed at IMS (3GPP, TISPAN, CableLabs) protocols
    (and thus being the perfect complement to SIPp for IMS testing),
    Seagull is a powerful traffic generator for functional, load,
    endurance, stress and performance/benchmark tests for almost
    any kind of protocol.

That was a quote from the `Seagull <http://gull.sourceforge.net/>`_ website.

This repo was cloned from `Seagull's svn repo on SourceForge <https://svn.code.sf.net/p/gull/code/>`_ on 2015-04-22.

Goal of this Repo
--------------------------------------------------------

I wanted to build and use Seagull on Ubuntu for Diameter protocol testing. I came across a lot of roadblocks that needed to be surmounted, primarily compiler errors when I tried to build it on anything modern, say CentOS 7 or Ubuntu 14.04.

The primary goal with this repo is to provide patches to the Subversion revision 422 of Seagull to make it build and work on Ubuntu 15.04 and CentOS 7.1 (1503).

Run ``svn info https://svn.code.sf.net/p/gull/code/`` and you'll see the following information that the code hasn't been updated in quite a while.

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

Build Instructions with Patches
--------------------------------------------------------

Since I have already done the work for you by adding patches to build on Ubuntu 15.04 and CentOS 7.1 (1503) ;) you can simply clone this repo and follow these instructions.

These instructions have been tested to work on:

* CentOS - 7.1 (1503) (64-bit version only)
* openSUSE - 13.2 (tested up to commit 79137704f54, before merging patches for issue #1)
* Ubuntu - 14.04.3, 15.04, 15.10 (64-bit versions only)

First Steps
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Install pre-requisites on Ubuntu.

::

    user@host:~$ sudo aptitude install build-essential curl git libglib2.0-dev ksh bison flex vim tmux

Install pre-requisites on CentOS.

::

    user@host:~$ sudo yum groupinstall "Development Tools"
    user@host:~$ sudo yum install curl git glib2-devel ksh bison flex flex-devel vim tmux

Install pre-requisites on openSUSE.

::

    user@host:~$ sudo zypper install -t pattern devel_basis
    user@host:~$ sudo zypper install curl git glib2-devel ksh bison flex

Clone this repo and create a branch to build from.

::

    user@host:~$ sudo mkdir -p ~/opt/src
    user@host:~$ cd ~/opt/src
    user@host:~/opt/src$ git clone https://github.com/hamzasheikh/Seagull.git seagull
    user@host:~/opt/src$ cd ~/opt/src/seagull
    user@host:~/opt/src/seagull$ git branch build master
    user@host:~/opt/src/seagull$ git checkout build

Add SCTP Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Download the latest sctplib and socketapi tarballs from http://www.sctp.de/ to the external-lib-src directory. They were are version 1.0.15 and 2.2.8 respectively at the time of writing.

::

    user@host:~$ cd ~/opt/src/seagull/seagull/trunk/src
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl --create-dirs -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/sctplib-1.0.15.tar.gz http://www.sctp.de/download/sctplib-1.0.15.tar.gz
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl --create-dirs -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/socketapi-2.2.8.tar.gz http://www.sctp.de/download/socketapi-2.2.8.tar.gz

Edit build-ext-lib.conf and update the versions of these two libraries ONLY if you downloaded and will use different versions.

::

    ...
    EXTBUILD_1_FILE=sctplib-1.0.15.tar.gz
    EXTBUILD_1_DIR=sctplib-1.0.15
    ...
    EXTBUILD_2_FILE=socketapi-2.2.8.tar.gz
    EXTBUILD_2_DIR=socketapi-2.2.8
    ...

Add TLS Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Download the `latest OpenSSL tarball <https://www.openssl.org/source/>`_ to the external-lib-src directory. It was 1.0.2e at the time of writing.

::

    user@host:~$ cd ~/opt/src/seagull/seagull/trunk/src
    user@host:~/opt/src/seagull/seagull/trunk/src$ curl --create-dirs -o ~/opt/src/seagull/seagull/trunk/src/external-lib-src/openssl-1.0.2e.tar.gz https://www.openssl.org/source/openssl-1.0.2e.tar.gz

Edit build-ext-lib.conf and update the version of this library ONLY if you downloaded a different version.

::

    ...
    EXTBUILD_3_FILE=openssl-1.0.2e.tar.gz
    EXTBUILD_3_DIR=openssl-1.0.2e
    ...

Build SCTP and TLS Support
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Build libraries for SCTP and TLS.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build-ext-lib.ksh

Build Seagull
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

First clean up previous build attempts and then build. Upon encountering errors (you shouldn't if you followed these instructions exactly but never say never) fix the error and run these two commands again.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build.ksh -target clean
    user@host:~/opt/src/seagull/seagull/trunk/src$ ksh build.ksh -target all

Install Seagull
--------------------------------------------------------------

I'm assuming Seagull built successfully. Install its binaries by copying files in ~/opt/src/seagull/seagull/trunk/src/bin to /usr/local/bin.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo cp ~/opt/src/seagull/seagull/trunk/src/bin/* /usr/local/bin

Following the lead of `rpm packages <http://sourceforge.net/projects/gull/files/>`_ provided by the official projects, install other required files in the /opt hierarchy.

::

    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo mkdir -p /opt/seagull
    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo cp -r ~/opt/src/seagull/seagull/trunk/src/exe-env/* /opt/seagull
    user@host:~/opt/src/seagull/seagull/trunk/src$ sudo chown -R myusername:myusername /opt/seagull
    user@host:~/opt/src/seagull/seagull/trunk/src$ mkdir -p /opt/seagull/{diameter-env,h248-env,http-env,msrp-env,octcap-env,radius-env,sip-env,synchro-env,xcap-env}/logs

To run a client or server, ``cd`` to /opt/seagull/\*/run dirctory and execute any start\*.ksh file. For example, to start a Diameter server

::

    user@host:~$ cd /opt/seagull/diameter-env/run
    user@host:/opt/seagull/diameter-env/run$ ./start_server.ksh


Investigation
--------------------------------------------------------------

If you're interested in how I investigated the build issues read the investigation.rst file.


Build Automation
--------------------------------------------------------------

Under the *build_automation* directory are Vagrantfiles for various OSes and
an Ansible playbook. ``cd`` into the directory with the Vagrantfile and run
``vagrant up``. At the end of provisioning you'll find built artifacts ready
to install.
