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
COMMAND_EXECUTED=$0
VALUE_AVERAGE=0
VALUE_NTH=0


function usage 
{
    echo "Command line syntax of $1 - options"
    echo "-in <file name>             : input file (default ${FILE_IN})"
    echo "-out <file name>            : output file (default ${FILE_OUT})"
    echo "-average <value>            : average value line (default none)"
    echo "-nth <value>                : nth percentile value (default none)"
    echo "-n <value>                  : n (of nth percentile value)"
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

          -nth)
	      shift
	      if test $# -ne 0
	      then
		  VALUE_NTH=$1
	      else
		  error "value nth percentile needed"
	      fi
	      ;;

          -n)
              shift
              if test $# -ne 0
              then
                  VALUE_N=$1
              else
                  error "value n needed"
              fi
              ;;

	  
          -average)
	      shift
	      if test $# -ne 0
		  then
		  VALUE_AVERAGE=$1
	      else
		  error "value average needed"
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

get_arguments $*
echo "[command    : ${COMMAND_EXECUTED}]"

eval `head --lines 1 ${FILE_IN}|sed -e 's/^/X_LABEL\=/' | sed -e 's/\;/ \; Y_LABEL=/' | sed -e 's/\;$//'`
# X data si later divided by 1000 to get seconds - Temporary workaround before having a time proof solution
X_LABEL="time DD HH:MM:SS"

FILE_OCTAVE=${FILE_IN}
FILE_OCTAVE_P=${FILE_OCTAVE}.p
FILE_OCTAVE_P2=${FILE_OCTAVE}.p2
FILE_OCTAVE_P3=${FILE_OCTAVE}.p3

nth=100.0

PLOT_CMD="\"${FILE_OCTAVE_P3}\" using 1:3 title \"Response time\""
/bin/cp $FILE_OCTAVE_P $FILE_OCTAVE_P3

if test ${VALUE_NTH} != "0"
then
    cat ${FILE_OCTAVE_P} | sed -e "s/$/ ${VALUE_NTH}/" > ${FILE_OCTAVE_P2}
    PLOT_CMD="${PLOT_CMD},\"${FILE_OCTAVE_P3}\" using 1:4 title \"${VALUE_N}th percentile (${VALUE_NTH})\""
fi

if test ${VALUE_AVERAGE} != "0"
then
    cat ${FILE_OCTAVE_P2} | sed -e "s/$/ ${VALUE_AVERAGE}/" > ${FILE_OCTAVE_P3}
    PLOT_CMD="${PLOT_CMD},\"${FILE_OCTAVE_P3}\" using 1:5 title \"Average (${VALUE_AVERAGE})\""
fi

octave <<EOF

xlabel("${X_LABEL}");
ylabel("${Y_LABEL}");
grid on;
gset term png;
gset output "${FILE_OUT}";

gset xdata time;
gset format x "%d %H:%M:%S";
gset timefmt "%d %H:%M:%S";
gset xtics rotate;
gplot ${PLOT_CMD};

EOF

/bin/rm -f $FILE_OCTAVE_P $FILE_OCTAVE_P2 $FILE_OCTAVE_P3 
