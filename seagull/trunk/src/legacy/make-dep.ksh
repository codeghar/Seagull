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


# Internal functions definition
find_dep () 
{
    file_name=$1
    if test -f ${file_name}
    then
	cat ${file_name} | \
	    sed -e 's/\/\/.*$//' | \
	    sed -n -e 's/[ 	]*\#[ 	]*include[ 	]*\"\([^\"][^\"]*\)\"[ 	]*/\1/p'
	return 0
    else
	return 1
    fi
}

# Internal functions definition
find_dep_with_path () 
{
    for file in `find_dep $1`
    do
      ls */$file 2>/dev/null
    done
}

# Internal
find_dep_not_verified () 
{
    file_name=$1
    cat ${file_name} | \
	sed -e 's/\/\/.*$//' | \
	sed -n -e 's/[ 	]*\#[ 	]*include[ 	]*\"\([^\"][^\"]*\)\"[ 	]*/\1/p'
}


# Command line
if test $# -lt 2
then
   echo "SYNTAX error: $0 <file-name> <build-name> <modules>"
   exit 1
fi

GENERATED_FILE=$1
shift
BUILD_FILE=$1
shift
typeset -u BUILD_TYPE=$1
shift
MODULES_USED=$*

echo "[build: ${BUILD_FILE}, modules: ${MODULES_USED}]"


# General variables definition
# Extention for the files treated 
FILE_EXTENTION="C cpp c l y"
# List of the files to be compiled

ALL_CODE_FILES=`for EXTENSION in ${FILE_EXTENTION}
do
  find . -follow -type f -name \*.${EXTENSION} 2>/dev/null | sed -e 's/^\.\///' 
done`

CODE_FILES=`for module in ${MODULES_USED}
do
  echo ${ALL_CODE_FILES} | tr ' ' '\n' | egrep '^'${module}'/'
done`

INCLUDE_PATHS_MODULES=`for module in ${MODULES_USED}
do
  echo "-I./${module}"
done | tr '\n' ' '`

# No file
if test -z "${CODE_FILES}"
then
    echo "[no file]"
    exit 1
fi

#echo "[Code Files = ${CODE_FILES}]"
NB_FILES=`echo "${CODE_FILES}" | wc -l`
echo "[Code files: ${NB_FILES}]"
L_I=1

# Search for file list
ALL_FILES=`for code_file in ${CODE_FILES}
do
  find_dep_with_path ${code_file}
done | sort -u`

echo "[Header files: `echo ${ALL_FILES} | tr ' ' '\n' | wc -l`]"

echo "# File: ${GENERATED_FILE}" >${GENERATED_FILE}
echo "" >>${GENERATED_FILE}

echo "include Makefile-common.mk" >>${GENERATED_FILE}
echo "include \$(WORK_DIR)/compiler.mk" >>${GENERATED_FILE}

echo "# Target objects" >>${GENERATED_FILE}
echo "TARGET_OBJ = \\" >>${GENERATED_FILE}
for file in ${CODE_FILES}
do
  file_obj=`echo ${file} | sed -e 's/\.[^\.]*$/\.o/'`
  file_obj=`basename ${file_obj}`
  file_ext=`basename ${file} | sed -e 's/[^\.]*\.//'`
  case ${file_ext} in
  l)
     file_obj="lex.yy.o"
     ;;
  y)
     file_base_name=`basename ${file}`
     file_without_ext=`echo ${file_base_name} | sed -e 's/\.y//'`
     file_obj="y.tab.o"
     ;;
  *)
     ;;
  esac
  if test ${L_I} -eq ${NB_FILES}
  then
    echo "    \$(WORK_DIR)/${file_obj}"
  else
    echo "    \$(WORK_DIR)/${file_obj} \\"
  fi
  L_I=`expr ${L_I} + 1`
done >>${GENERATED_FILE}
echo "" >>${GENERATED_FILE}
echo "" >>${GENERATED_FILE}

echo "# Include specific paths" >>${GENERATED_FILE}
echo "MODULE_PATH = ${INCLUDE_PATHS_MODULES}" >>${GENERATED_FILE}
echo "" >>${GENERATED_FILE}


echo "# General entry" >>${GENERATED_FILE}
echo "FINAL_TARGET=\$(BUILD_DIR)/${BUILD_FILE}" >>${GENERATED_FILE}
echo "\$(FINAL_TARGET): \$(TARGET_OBJ)" >>${GENERATED_FILE}
echo "	@echo \"[Linking \$(FINAL_TARGET)]\"" >>${GENERATED_FILE}
echo "	@\$(LD_${BUILD_TYPE}) \$(LD_FLAGS_${BUILD_TYPE}) \$(TARGET_OBJ) \$(LD_PATH_${BUILD_TYPE}) \$(LD_LIBS_${BUILD_TYPE}) -o \$(FINAL_TARGET)" >>${GENERATED_FILE}
echo "	" >>${GENERATED_FILE}
echo "" >>${GENERATED_FILE}

echo "# Header files rules" >>${GENERATED_FILE}
for file in ${ALL_FILES}
do
  echo ${file}": " `find_dep_with_path ${file}`
  echo ""
done >>${GENERATED_FILE}

echo "# Object file rules" >>${GENERATED_FILE}
for file in ${CODE_FILES}
do
  file_obj=`echo ${file} | sed -e 's/\.[^\.]*$/\.o/'`
  file_obj=`basename ${file_obj}`
  file_ext=`basename ${file} | sed -e 's/[^\.]*\.//'`

  case ${file_ext} in 
      l)
	  lex_file=$file
          lex_basename_file=`basename ${file}`
          lex_withtout_ext=`echo ${lex_basename_file} | sed -e 's/\.\l//'`
	  lex_code=lex.yy.c
	  lex_flags="LEX_"
	  echo "\$(WORK_DIR)/${lex_code}: ${lex_file} "`find_dep_with_path ${lex_file}`" \${WORK_DIR}/y.tab.h"
	  echo "	@echo \"[Generating ${lex_code}]\""
	  echo "	@\$(LEX_${BUILD_TYPE}) \$(LEX_FLAGS_${BUILD_TYPE}) ${lex_file}"
	  echo "	@\$(MV) ${lex_code} \$(WORK_DIR)/${lex_code}"
	  echo ""
	  file_obj=lex.yy.o
          file="\$(WORK_DIR)/lex.yy.c"

	  ;;
      y) 
	  yacc_basename_file=`basename ${file}`
	  yacc_file=${file}
	  yacc_code=y.tab.c
	  yacc_decl=y.tab.h
	  yacc_withtout_ext=y
	  yacc_code_path=`echo ${file} | sed -e 's/\.y//'`".tab.c"
	  yacc_decl_path=`echo ${file} | sed -e 's/\.y//'`".tab.h"
	  lex_flags="YACC_"
	  echo "\$(WORK_DIR)/${yacc_code} \$(WORK_DIR)/${yacc_decl}: ${yacc_file} "`find_dep_with_path ${yacc_file}`
	  echo "	@echo \"[Generating ${yacc_code}]\""
	  echo "	@\$(YACC_${BUILD_TYPE}) \$(YACC_FLAGS_${BUILD_TYPE}) ${yacc_file}"
	  echo "	@\$(MV) ${yacc_code} \$(WORK_DIR)/${yacc_code}"
	  echo "	@\$(MV) ${yacc_decl} \$(WORK_DIR)/${yacc_decl}"
	  echo ""
	  

	  file_obj=y.tab.o
	  file="\$(WORK_DIR)/${yacc_withtout_ext}.tab.c"
	  ;;
      *)
	  lex_flags=
	  ;;
  esac
  
  echo "\$(WORK_DIR)/${file_obj}: ${file} "`find_dep_with_path ${file}`
  echo "	@echo \"[Compiling ${file}]\""
  echo "	@\$(CC_${BUILD_TYPE}) \$(MODULE_PATH) -I\$(WORK_DIR) \$("${lex_flags}"CC_FLAGS_${BUILD_TYPE}) \$(CC_INCLUDE_${BUILD_TYPE}) -c ${file} -o \$(WORK_DIR)/${file_obj}"
  echo ""

done >>${GENERATED_FILE}





