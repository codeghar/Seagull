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

find . -name \*.log -print -exec /bin/rm \{\} \;
find . -name server.txt -print -exec /bin/rm \{\} \;
find . -name client.txt -print -exec /bin/rm \{\} \;
find . -name Memory_leaks.log -print -exec /bin/rm \{\} \;
find . -name '*.stackdump' -print -exec /bin/rm \{\} \;
find . -name 'core' -print -exec /bin/rm \{\} \;
