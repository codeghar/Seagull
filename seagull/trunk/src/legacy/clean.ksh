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
# Clean script
#
#
# Command definitions
RM="/bin/rm -f"
FIND=/usr/bin/find

TEMP_DIR=`ls | sed -n -e '/work\-[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$/p' | tr '\n' ' '`
TEMP_DIR=${TEMP_DIR:+${TEMP_DIR}" "}`ls | sed -n -e '/build\-[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$/p' | tr '\n' ' '`
TEMP_DIR=${TEMP_DIR:+${TEMP_DIR}" "}`ls | sed -n -e '/bin\-[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$/p' | tr '\n' ' '`

TEMP_LINK=`ls | sed -n -e '/^bin$/p' | tr '\n' ' '`
TEMP_LINK=${TEMP_LINK:+${TEMP_LINK}" "}`ls | sed -n -e '/^ext$/p' | tr '\n' ' '`


# delete temporary object file
#        temporary editor files


${FIND} . \( -name \*~ -o -name \*.o  -o -name \#*\# \) -print -exec ${RM} \{\} \; | while read file_name
do
	echo "[file ${file_name} deleted]"
done

# delete temporary compilation directories

if test -n "${TEMP_DIR}"
then
	for dir_name in ${TEMP_DIR}
	do
		echo "[Deleting ${dir_name}]"
		${RM} -r ${dir_name}
	done
else
	echo "[no directory to delete]"
fi

if test -n "${TEMP_LINK}"
then
	for link_name in ${TEMP_LINK}
	do
		echo "[Deleting ${link_name}]"
		${RM} ${link_name}
	done
else
	echo "[no link to delete]"
fi

