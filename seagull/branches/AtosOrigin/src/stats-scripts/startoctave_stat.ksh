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

get_arguments $*
echo "[command    : ${COMMAND_EXECUTED}]"

FILE_OCTAVE=${FILE_IN}

octave <<EOF

#open file for output
file=fopen("${FILE_OUT}","w")

# load data
fprintf(file, "[Using file        : ${FILE_OCTAVE}]\n");
fprintf(file, "[number values     : ");
fflush(file);
load -ascii ${FILE_OCTAVE} data_y
number_measures=rows(data_y)
fprintf(file, "%d]\n", number_measures);

# min value
fprintf(file, "[minimum value     : ");
fflush(file);
value_min = min(data_y)
fprintf(file, "%f]\n", value_min);

# max value
fprintf(file, "[maximum value     : ");
fflush(file);
value_max = max(data_y)
fprintf(file, "%f]\n", value_max);

# average
fprintf(file, "[average value     : ");
fflush(file);
value_average = mean(data_y)
fprintf(file, "%f]\n", value_average);

# variance
fprintf(file, "[variance          : ");
fflush(file);
value_variance = var(data_y)
fprintf(file, "%f]\n", value_variance);

# standard deviation
fprintf(file, "[standard deviation: ");
fflush(file);
value_standard_deviation = std(data_y)
fprintf(file, "%f]\n", value_standard_deviation);

# nth percentile calculus
nth_percentile=${NTH}
fprintf(file, "[%dth percentile   : ", nth_percentile);
fflush(file);
g=sort(data_y);
index=floor((100-nth_percentile)*number_measures/100)+1
value_nth_percentile=g(number_measures-index-1)
fprintf(file, "%f]\n", value_nth_percentile);
fflush(file);

fclose(file);

EOF

