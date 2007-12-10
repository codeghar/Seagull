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
FILE_OUT=file.save
NTH=95
COMMAND_EXECUTED=$0


function usage 
{
    echo "Command line syntax of $1 - options"
    echo "-in <file name>             : input file (default ${FILE_IN})"
    echo "-out <file name>            : output file (default ${FILE_OUT})"
    echo "-nth <percentile>           : nth percentile calculus (default 95)"
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

          -nth)
	      shift
	      if test $# -ne 0
	      then
		  NTH=$1
		  if test `expr ${NTH} : [0-9][0-9]*` -eq 0
		  then
		      error "bad format for argument ${NTH} of -nth"
		  fi
	      else
		  error "nth percentile value needed (1-100)"
	      fi
	      ;;

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

if test -f ${FILE_OUT}
then
    echo "[delete existing file ${FILE_OUT}]"
  /bin/rm -f ${FILE_OUT}
fi

EXTRACT=KO

FILE_OCTAVE=${FILE_IN}.y
csvextract y ${FILE_IN} && EXTRACT=OK

if test ${EXTRACT} = OK
then
  startoctave_stat.ksh -in ${FILE_OCTAVE} -out ${FILE_OUT} -nth ${NTH} 
fi
