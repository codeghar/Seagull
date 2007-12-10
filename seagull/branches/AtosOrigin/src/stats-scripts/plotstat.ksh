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

FILE_IN=file.csv
FILE_OUT=file.png
FILE_STAT=
VALUE_NTH=0
VALUE_N=0
VALUE_AVERAGE=0
COMMAND_EXECUTED=$0


function usage 
{
    echo "Command line syntax of $1 - options"
    echo "-in <file name>             : input file (default ${FILE_IN})"
    echo "-out <file name>            : output file (default ${FILE_OUT})"
    echo "-stat <file name>           : input stat file name (default no file)"
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

          -in)
	      shift
	      if test $# -ne 0
		  then
		  FILE_IN=$1
	      else
		  error "input file name needed"
	      fi
	      ;;

          -out)
	      shift
	      if test $# -ne 0
		  then
		  FILE_OUT=$1
	      else
		  error "output file name needed"
	      fi
	      ;;

          -stat)
	      shift
	      if test $# -ne 0
		  then
		  FILE_STAT=$1
	      else
		  error "stat file name needed"
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

if test $# -eq 0
then
   usage ${COMMAND_EXECUTED}
   exit 0
fi

get_arguments $*
echo "[command    : $0]"
echo "[input file : ${FILE_IN}]"
echo "[output file: ${FILE_OUT}]"
if test -n "${FILE_STAT}"
then
echo "[stat file  : ${FILE_STAT}]"
fi

if test ! -f "${FILE_IN}"
then
    error "[file ${FILE_IN} not found]"
fi


if test -f ${FILE_OUT}
then
    echo "[delete existing file ${FILE_OUT}]"
  /bin/rm -f ${FILE_OUT}
fi

if test -f "${FILE_STAT}"
then
    VALUE_NTH=`cat ${FILE_STAT} | grep 'th percentile' | \
        sed -n -e 's/^[^:]*\:[ 	]*\([^\]]*\)/\1/p' | tr -d ']'`
    VALUE_N=`cat ${FILE_STAT} | grep 'th percentile' | \
        sed -n -e 's/^\[\(.*\)th.*/\1/p' | tr -d ']'`
    VALUE_AVERAGE=`cat ${FILE_STAT} | grep 'average value' | \
        sed -n -e 's/^[^:]*\:[ 	]*\([^\]]*\)/\1/p' | tr -d ']'`
    echo "[using nth percentile : ${VALUE_NTH}]"
    echo "[using n              : ${VALUE_N}]"
    echo "[using average        : ${VALUE_AVERAGE}]"
fi

EXTRACT=KO

FILE_OCTAVE=${FILE_IN}.y
#csvextract a ${FILE_IN} && EXTRACT=OK
#csvextract x ${FILE_IN} && EXTRACT=OK
csvextract p ${FILE_IN} && EXTRACT=OK


if test ${EXTRACT} = OK
then
  startoctave_plot.ksh -in ${FILE_IN} -out ${FILE_OUT} -nth ${VALUE_NTH} -n ${VALUE_N} -average ${VALUE_AVERAGE}
fi
