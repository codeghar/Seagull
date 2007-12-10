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
##################################################"

BUILD_PROJECT=$1
BUILD_CONF_FILE=$2
BUILD_COMPILER_FILE=$3

BUILD_TARGET=
BUILD_TYPES=

symbol_value()
{
    L_symbol=$1
    eval L_value='$'${L_symbol}
    echo ${L_value}
}

determine_generation()
{
    L_i=1
    L_end=0
    L_max=`symbol_value MAX_BUILD`
    if test -z "${L_max}"
    then
       L_max=0
    fi

    while test ${L_end} -eq 0
      do

      L_name=`symbol_value BUILD_${L_i}_NAME`
      if test -z "${L_name}"
	  then
       if test ${L_max} -eq 0
       then 
	       L_end=1 
       else
          if test ${L_i} -le ${L_max}
          then
             L_end=0
          else
             L_end=1
          fi
       fi


      else
	  BUILD_TARGET=${BUILD_TARGET:+${BUILD_TARGET}" "}"all_${L_name}"
	  L_modules=`symbol_value BUILD_${L_i}_MODULES`
	  L_type=`symbol_value BUILD_${L_i}_TYPE`
	  BUILD_TYPES=${BUILD_TYPES:+${BUILD_TYPES}" "}"${L_type}"

	  echo "\$(WORK_DIR)/dep-${L_name}.mk:"
	  echo "	@echo \"[Generating file dep-${L_name}.mk]\""
	  echo "	@./make-dep.ksh \$(WORK_DIR)/dep-${L_name}.mk ${L_name} ${L_type} ${L_modules}"
	  echo ""

	  echo "all_${L_name}: \$(WORK_DIR)/dep-${L_name}.mk"
	  echo "	@echo \"[Generating file ${L_name}]\""
	  echo "	@make -f \$(WORK_DIR)/dep-${L_name}.mk WORK_DIR=\$(WORK_DIR) BUILD_DIR=\$(BUILD_DIR) TOOL_NAME=\$(TOOL_NAME) TOOL_VERSION=\$(TOOL_VERSION) DEP_TARGET=${L_name}"
	  echo ""

      fi
	  L_i=`expr ${L_i} + 1`
      
    done
}

extract_compiler_def ()
{
    typeset -u L_type=$1
    typeset -u L_osname
    typeset -u L_osarchi
    typeset -u L_symbName
    typeset -u L_debugMode=${DEBUG_MODE}
    L_osname=`uname -s | tr '-' '_' | tr '.' '_' | tr '/' '_' `
    L_osarchi=`uname -m | tr '-' '_' | tr '.' '_' | tr '/' '_' `

    # Compiler definition
    # echo "# os: ${L_osname} arch: ${L_osarchi}"
    # echo "# BUILD_${L_type}_CC_${L_osname}_${L_osarchi}"
    L_symbName=`symbol_value "BUILD_${L_type}_CC_${L_osname}_${L_osarchi}"`
    # echo "# symb: [${L_symbName}]"
    if test -n "${L_symbName}"
    then
      echo "CC_${L_type}=`symbol_value "BUILD_${L_type}_CC_${L_osname}_${L_osarchi}"`"
    else
      L_symbName=`symbol_value "BUILD_${L_type}_CC_${L_osname}"`
      if test -n "${L_symbName}"
      then
        echo "CC_${L_type}=`symbol_value "BUILD_${L_type}_CC_${L_osname}"`"
      else
        error "Compiler variable for OS ${L_osname} not defined"
      fi
    fi

    # Compiler Flags definition
    # echo "# os: ${L_osname} arch: ${L_osarchi}"
    # echo "# BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}"
    L_symbName=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}"`"
        fi
      else
        echo "CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_CC_FLAGS_${L_osname}"`"
      fi
    fi

    # Linker definition
    L_symbName=`symbol_value "BUILD_${L_type}_LD_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      echo "LD_${L_type}=`symbol_value "BUILD_${L_type}_LD_${L_osname}_${L_osarchi}"`"
    else
      echo "LD_${L_type}=`symbol_value "BUILD_${L_type}_LD_${L_osname}"`"
    fi

    # Linker Flags definition
    L_symbName=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}"`"
        fi
      else
        echo "LD_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LD_FLAGS_${L_osname}"`"
      fi
    fi

    # Linker Libraries definition
    L_symbName=`symbol_value "BUILD_${L_type}_LD_LIBS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      echo "LD_LIBS_${L_type}=`symbol_value "BUILD_${L_type}_LD_LIBS_${L_osname}_${L_osarchi}"`"
    else
      echo "LD_LIBS_${L_type}=`symbol_value "BUILD_${L_type}_LD_LIBS_${L_osname}"`"
    fi


    # Lex definition
    L_symbName=`symbol_value "BUILD_${L_type}_LEX_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      echo "LEX_${L_type}=`symbol_value "BUILD_${L_type}_LEX_${L_osname}_${L_osarchi}"`"
    else
      echo "LEX_${L_type}=`symbol_value "BUILD_${L_type}_LEX_${L_osname}"`"
    fi

    # Lex Flags definition
    L_symbName=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}"`"
        fi
      else
        echo "LEX_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_FLAGS_${L_osname}"`"
      fi
    fi

    # Lex Compiler Flags definition
    L_symbName=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}"`"
        fi
      else
        echo "LEX_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_LEX_CC_FLAGS_${L_osname}"`"
      fi
    fi

    # Yacc definition
    L_symbName=`symbol_value "BUILD_${L_type}_YACC_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      echo "YACC_${L_type}=`symbol_value "BUILD_${L_type}_YACC_${L_osname}_${L_osarchi}"`"
    else
      echo "YACC_${L_type}=`symbol_value "BUILD_${L_type}_YACC_${L_osname}"`"
    fi

    # Yacc Flags definition
    L_symbName=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}"`"
        fi
      else
        echo "YACC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_FLAGS_${L_osname}"`"
      fi
    fi

    # Yacc Compiler Flags definition
    L_symbName=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_osarchi}"`
    if test -n "${L_symbName}"
    then
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_osarchi}_${L_debugMode}"`"
        else
          echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_osarchi}"`"
        fi
      else
        echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_osarchi}"`"
      fi
    else
      # Debug mode processing
      if test -n "${L_debugMode}"
      then
        L_symbName=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_debugMode}"`
        if test -n "${L_symbName}"
        then
          echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}_${L_debugMode}"`"
        else
          echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}"`"
        fi
      else
        echo "YACC_CC_FLAGS_${L_type}=`symbol_value "BUILD_${L_type}_YACC_CC_FLAGS_${L_osname}"`"
      fi
    fi


    echo ""
}

error ()
{
    echo $*
    exit 1
}


if test -f ${BUILD_CONF_FILE}
then
    . ./${BUILD_CONF_FILE}
else
    error "File ${BUILD_CONF_FILE} not found"
fi

# to add on build
if test -z "${PROJECT_NAME}"
then
    error "Shell variable PROJECT_NAME not defined"
fi

if test -z "${PROJECT_VERSION}"
then
    error "Shell variable PROJECT_VERSION not defined"
fi


echo "# File: ${BUILD_PROJECT}" >${BUILD_PROJECT}
if test -n "${DEBUG_MODE}"
then
  echo "Debug Mode: on" >>${BUILD_PROJECT}
else
  echo "Debug Mode: on" >>${BUILD_PROJECT}
fi

echo "" >>${BUILD_PROJECT}


echo "# dependance files generation" >>${BUILD_PROJECT}
determine_generation >>${BUILD_PROJECT}
echo "" >>${BUILD_PROJECT}

echo "# Compiler definitions" > ${BUILD_COMPILER_FILE}
echo "# OS Name        : `uname -s` " >> ${BUILD_COMPILER_FILE}
echo "# OS Architecture: `uname -s` arch: `uname -m`" >> ${BUILD_COMPILER_FILE}
if test -n "${DEBUG_MODE}"
then
  echo "# Debug Mode     : on" >>${BUILD_COMPILER_FILE}
else
  echo "# Debug Mode     : on" >>${BUILD_COMPILER_FILE}
fi
for L_gen_type in `echo ${BUILD_TYPES} | tr ' ' '\n' | sort -u`
do
  extract_compiler_def ${L_gen_type} >> ${BUILD_COMPILER_FILE}
done

echo $BUILD_TYPES

echo "# target to build" >>${BUILD_PROJECT}
echo "all: ${BUILD_TARGET}" >>${BUILD_PROJECT}
echo "" >>${BUILD_PROJECT}

echo "# bin distribution to build" >>${BUILD_PROJECT}
echo "dist: all" >>${BUILD_PROJECT}
echo "	@./make-dist.ksh \$(BUILD_DIR) \$(WORK_DIR)" >>${BUILD_PROJECT}


