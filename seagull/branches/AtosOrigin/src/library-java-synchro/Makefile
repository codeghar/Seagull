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

all: synchro_usage.class


C_SynchroCmd.class : C_SynchroCmd.java
	@echo "[compiling C_SynchroCmd.java]"
	@javac -verbose C_SynchroCmd.java


C_SynchroThread.class : C_SynchroThread.java  C_SynchroSemaphore.java E_SynchroLost.java C_SynchroCmd.class
	@echo "[compiling C_SynchroThread.java]"
	@javac -verbose C_SynchroThread.java


synchro_usage.class : synchro_usage.java C_SynchroThread.class C_SynchroCmd.class
	@echo "[compiling synchro_usage.java]"
	@javac -verbose synchro_usage.java


clean:
	@echo "[cleaning file generated]"
	@/bin/rm -f *.class *~



