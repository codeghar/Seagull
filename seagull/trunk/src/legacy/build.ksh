#!/bin/ksh
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# (c)Copyright 2006 Hewlett-Packard Development Company, LP.
#
#
# 
# Build script 
# Unix
# Creation: 2004/01/28
# Version 1.0
#

COMMAND_EXECUTED=$0

# Default values
BUILD_DEFAULT_TARGET=all
BUILD_DEFAULT_EXEC=RELEASE

# Init of variables
BUILD_TARGET=${BUILD_DEFAULT_TARGET}
BUILD_EXEC=${BUILD_DEFAULT_EXEC}
typeset -u BUILD_OS=`uname -s | tr '-' '_' | tr '.' '_' | tr '/' '_' `
BUILD_CODE_DIRECTORY=code
BUILD_DIRECTORY=`pwd`
typeset -u BUILD_ARCH=`uname -m | tr '-' '_' | tr '.' '_' | tr '/' '_' `
BUILD_VERSION_FILE=build.conf
BUILD_DIST_MODE=0 
BUILD_FORCE_MODE=0

function usage 
{
    echo "Command line syntax of $1 - options"
    echo "-exec <RELEASE|DEBUG>       : mode used for compilation (default ${BUILD_DEFAULT_EXEC})"
    echo "-target <all|clean|force|dist>    : target used for compilation (default ${BUILD_DEFAULT_TARGET})"
    echo "-help                       : display the command line syntax"
}

function error
{
    echo "ERROR: $1"
    exit 1
}

function warning
{
    echo "WARNING: $1"
}

function get_arguments
{
    while [ $# -ne 0 ]
    do
      case $1 in

          -exec)
	      shift
	      if [ $# -ne 0 ]
		  then
		  case $1 in
		      RELEASE|DEBUG)
			  ;;
		      *)
			  error "Unrecognized -exec option argument"
			  ;;
		  esac
		  BUILD_EXEC=$1
	      else
		  error "executable mode needed (RELEASE or DEBUG)"
	      fi
	      ;;
	  
          -target)
	      shift
	      if [ $# -ne 0 ]
		  then
		  case $1 in
                      dist)
                          BUILD_DIST_MODE=1
			  ;;
		      all|clean)
			  ;;
                      force)
                          BUILD_FORCE_MODE=1
			  ;;
		      *)
			  error "Unrecognized -target option argument"
			  ;;
		  esac
		  BUILD_TARGET=$1
	      else
		  error "target needed (all, clean or sample)"
	      fi
	      ;;
	  
          -help)
	      usage ${COMMAND_EXECUTED}
	      exit 0
	      ;;

	  *)
	      error "unrecognized option $1"
	      ;;
      esac
      shift
    done
}

function check_dlfcn {
    if test -f /usr/lib/libdld.sl
    then
      nm /usr/lib/libdld.sl | grep dlopen >/dev/null
      if test $? -eq 0
      then
        echo "-DUSE_DLOPEN"
      else
        echo ""
      fi
    fi
}

# Arguments analysis
get_arguments $*

if test -f ${BUILD_VERSION_FILE}
then
	. ./${BUILD_VERSION_FILE}

	#BUILD_VERSION & BUILD_NAME
        BUILD_VERSION=${PROJECT_VERSION}
	BUILD_NAME=${PROJECT_NAME}
else
	error "no version file ${BUILD_VERSION_FILE} found"
fi


# Print build environment

echo "[System  : ${BUILD_OS}]"
echo "[Archi   : ${BUILD_ARCH}]"
echo "[Mode    : ${BUILD_EXEC}]"
echo "[Name    : ${BUILD_NAME}]"
echo "[Version : ${BUILD_VERSION}]"

BUILD_WORK="${BUILD_DIRECTORY}/work-${BUILD_VERSION}"
BUILD_BUILD="${BUILD_DIRECTORY}/build-${BUILD_VERSION}"
EXT_DIR="${BUILD_DIRECTORY}/ext-${BUILD_VERSION}"

#  In case of Distribution build just build all
if test ${BUILD_DIST_MODE} -eq 1
then
  BUILD_TARGET=all
fi

case ${BUILD_EXEC} in
    RELEASE)
	# Nothing more to do
	DEBUG_MODE=""
	;;
    DEBUG)
	# Change build option for debug mode
	DEBUG_MODE="DBG"
	;;
esac

#  In case of Force mode do a clean first
if test ${BUILD_FORCE_MODE} -eq 1
then
  echo " "
  echo "[Begin Clean Forced phase]"
  (make -f Makefile-generic.mk clean "WORK_DIR="${BUILD_WORK} "BUILD_DIR="${BUILD_BUILD} "BUILD_NAME="${BUILD_NAME} "EXT_BUILD_DIR="${EXT_DIR} "CURRENT_DIR="${BUILD_DIRECTORY} "DEBUG_MODE="${DEBUG_MODE} && echo "[End Makefile generation phase OK]")

  # set the target to all now
  BUILD_TARGET=all
fi

# Build the required target
echo " "
echo "[Begin Makefile generation phase]"
(make -f Makefile-generic.mk ${BUILD_TARGET} "WORK_DIR="${BUILD_WORK} "BUILD_DIR="${BUILD_BUILD} "BUILD_NAME="${BUILD_NAME} "EXT_BUILD_DIR="${EXT_DIR} "CURRENT_DIR="${BUILD_DIRECTORY} "DEBUG_MODE="${DEBUG_MODE} && echo "[End Makefile generation phase OK]")

# Now determine the real target
# in case of distributation  change the target
if test ${BUILD_DIST_MODE} -eq 1
then
  BUILD_TARGET=dist
fi

if test -f ${BUILD_WORK}/project.mk
then
    echo " "
    echo "[Begin compilation phase]"
    (make -f ${BUILD_WORK}/project.mk ${BUILD_TARGET} "WORK_DIR="${BUILD_WORK} "BUILD_DIR="${BUILD_BUILD} "BUILD_NAME="${BUILD_NAME} && echo "[End compilation phase OK]" )
fi

echo " "
