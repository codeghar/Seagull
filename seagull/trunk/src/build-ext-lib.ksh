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

BUILD_UNCOMPRESS=""
BUILD_VERSION_FILE=build.conf
CURRENT_DIR=`pwd`
EXT_SRC_DIR="external-lib-src"
EXTBUILD_CONF_FILE=build-ext-lib.conf

EXT_ROOT_DIR=${CURRENT_DIR}

function kerror
{
    echo "ERROR: $1"
    exit 1
}

function kwarning
{
    echo "WARNING: $1"
}

function symbol_value
{
    L_symbol=$1
    eval L_value='$'${L_symbol}
    echo ${L_value}
}

echo "[Building external library]"

if test -f ${BUILD_VERSION_FILE}
then
	. ./${BUILD_VERSION_FILE}
        BUILD_VERSION=${PROJECT_VERSION}
else
	kerror "no version file ${BUILD_VERSION_FILE} found"
fi

EXT_VERSION=${BUILD_VERSION}

if test -f ${EXTBUILD_CONF_FILE}
then
	. ./${EXTBUILD_CONF_FILE}
else
	kerror "no version file ${EXTBUILD_CONF_FILE} found"
fi

# determine number of library to be generated
NB_BUILD=`cat ${EXTBUILD_CONF_FILE} | sed -n -e 's/^EXTBUILD_\([^_]*\)_FILE=.*$/\1/p' | sort -u`

echo -n "[External build "
for build in ${NB_BUILD}
do
  echo -n "[${build}]"
done
echo "]"

if test ! -d ${CURRENT_DIR}/${EXT_SRC_DIR}
then
   kerror "unable to find directory [${CURRENT_DIR}/${EXT_SRC_DIR}]"
fi


EXT_WORK_DIR="${CURRENT_DIR}/work-${BUILD_VERSION}"
EXT_BUILD_DIR="${CURRENT_DIR}/build-${BUILD_VERSION}"

if test -x /usr/bin/gzip
then 
  BUILD_UNCOMPRESS="/usr/bin/gzip -dv"
fi

if test -z "${BUILD_UNCOMPRESS}"
then
  if test -x /bin/gzip
  then
    BUILD_UNCOMPRESS="/bin/gzip -dv"
  fi
fi

if test -z "${BUILD_UNCOMPRESS}"
then
  if test -x /usr/contrib/bin/gzip
  then
    BUILD_UNCOMPRESS="/usr/contrib/bin/gzip -dv"
  fi
fi

if test -z "${BUILD_UNCOMPRESS}"
then
  if test -x /usr/local/bin/gzip
  then
    BUILD_UNCOMPRESS="/usr/local/bin/gzip -dv"
  fi
fi

if test -z "${BUILD_UNCOMPRESS}"
then
  if test -x /usr/bin/uncompress
  then
    BUILD_UNCOMPRESS="/usr/bin/uncompress"
  fi
fi

if test -z "${BUILD_UNCOMPRESS}"
then
    kerror "Unable to find [gzip or uncompress]"
fi

echo "[Using ${BUILD_UNCOMPRESS}]"

if test ! -d ${EXT_WORK_DIR}
then
    echo "[Creating directory ${EXT_WORK_DIR}]"
    mkdir ${EXT_WORK_DIR}
fi 

if test ! -d ${EXT_BUILD_DIR}
then
    echo "[Creating directory ${EXT_BUILD_DIR}]"
    mkdir ${EXT_BUILD_DIR}
fi 


for build in ${NB_BUILD}
do

  EXT_FILE=`symbol_value EXTBUILD_${build}_FILE`
  echo "[File: ${EXT_FILE}]"
  EXT_DIR=`symbol_value EXTBUILD_${build}_DIR`
  echo "[Directory: ${EXT_DIR}]"
  EXT_BEGINSCRIPT=`symbol_value EXTBUILD_${build}_BEGINSCRIPT`
  echo "[Begin script: ${EXT_BEGINSCRIPT}]"

  EXT_GENSCRIPT=`symbol_value EXTBUILD_${build}_GENSCRIPT`
  echo "[Gen script: ${EXT_GENSCRIPT}]"
  EXT_ENDSCRIPT=`symbol_value EXTBUILD_${build}_ENDSCRIPT`
  echo "[End script: ${EXT_ENDSCRIPT}]"

  echo "[go to directory ${EXT_WORK_DIR}]"
  cd ${EXT_WORK_DIR}

  if test -f ${CURRENT_DIR}/${EXT_SRC_DIR}/${EXT_FILE}
  then
      if test -d ${EXT_DIR}
      then
         echo "[Directory ${EXT_DIR} already exists]"
      else
         if test ! -f ${EXT_FILE}
         then
           cp ${CURRENT_DIR}/${EXT_SRC_DIR}/${EXT_FILE} .
         fi
	 echo "[Executing ${BUILD_UNCOMPRESS} ${EXT_FILE}]"
         ${BUILD_UNCOMPRESS} ${EXT_FILE}
         tar -xvf `echo ${EXT_FILE} | sed -e 's/\.tgz$/\.tar/' | sed -e 's/\.tar.gz$/\.tar/'`
	 if test ! -d ${EXT_DIR}
         then
	     kerror "[Directory ${EXT_DIR} not created]"
         fi
      fi
  else 
      kerror "unable to find directory [${CURRENT_DIR}/${EXT_SRC_DIR}/${EXT_FILE}]"     
  fi

  cd ${EXT_DIR}

  if test -n ${EXT_GENSCRIPT}
  then
      echo "[Executing ${EXT_GENSCRIPT}]"
      ${EXT_GENSCRIPT}
  fi

  if test -n ${EXT_ENDSCRIPT}
  then
      echo "[Executing ${EXT_ENDSCRIPT}]"
      ${EXT_ENDSCRIPT}
  fi

  cd ${CURRENT_DIR}  

  echo "[go to directory ${CURRENT_DIR}]"
done




for file in ${EXT_SRC_LIST}
do
   local_file=`basename ${file}`
   local_dir=`echo ${local_file} | sed -e 's/\.tgz$//' | sed -e 's/\.tar.gz$//'`
   echo "[`basename ${local_file}`]"
   if test -d ${local_dir}
   then
     echo "[directory ${local_dir} already exists]"
   else
     echo cp ../${EXT_SRC_DIR}/${local_file} .
     echo ${BUILD_UNCOMPRESS} ${local_file}
     echo tar -xvf `echo ${local_file} | sed -e 's/\.tgz$/\.tar/' | sed -e 's/\.tar.gz$/\.tar/'`
     echo rm `echo ${local_file} | sed -e 's/\.tgz$/\.tar/' | sed -e 's/\.tar.gz$/\.tar/`
   fi
   echo cd ${local_dir}
   echo ./configure
   echo make
   echo cd ..
done


