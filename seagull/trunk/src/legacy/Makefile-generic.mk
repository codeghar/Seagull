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
# Makefile-generic.mk 
#
# used variables: 
# WORK_DIR  = directory for temporary files
# BUILD_DIR = directory for generated files
# EXT_BUILD_DIR = directory for external libraries generation
# EXT_DIR   = directory for external libraries generation (no version)
# BIN_DIR   = directory for binary file generation (no version)
#
#####################################################################

BIN_DIR = $(CURRENT_DIR)/bin
#EXT_DIR = $(CURRENT_DIR)/ext

all: generic_files specific_files


$(WORK_DIR)/project.mk $(WORK_DIR)/compiler.mk: build.conf
	@./gen-project.ksh $(WORK_DIR)/project.mk build.conf $(WORK_DIR)/compiler.mk


#generic_files: build.conf gen-project.ksh make-dep.ksh build.ksh clean.ksh $(BIN_DIR) $(EXT_DIR)
generic_files: build.conf gen-project.ksh make-dep.ksh build.ksh clean.ksh $(BIN_DIR)

specific_files: $(WORK_DIR) $(BUILD_DIR) $(WORK_DIR)/project.mk $(WORK_DIR)/compiler.mk


# directory creation entries

$(WORK_DIR):
	@echo "[Making directory: $(WORK_DIR)]"
	@mkdir $(WORK_DIR)

$(BUILD_DIR):
	@echo "[Making directory: $(BUILD_DIR)]"
	@mkdir $(BUILD_DIR)

$(BIN_DIR):
	@echo "[Creating symbolic link: $(BIN_DIR)]"
	@ln -s $(BUILD_DIR) $(BIN_DIR)


#$(EXT_DIR):
#	@echo "[Creating symbolic link: $(EXT_DIR)]"
#	@ln -s $(EXT_BUILD_DIR) $(EXT_DIR)

clean:
	@./clean.ksh
