################################################################################
##
## Filename: 	Makefile
##
## Project:	Xtimesheet, a very simple text-based timesheet tracking program
##
## Purpose:	Master make file, should build everything.
##
## Particular targets include:
##
##	all
##	archive
##	clean
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
##
## Copyright (C) 2017-2022, Gisselquist Technology, LLC
##
## This program is free software (firmware): you can redistribute it and/or
## modify it under the terms of  the GNU General Public License as published
## by the Free Software Foundation, either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
## target there if the PDF file isn't present.)  If not, see
## <http://www.gnu.org/licenses/> for a copy.
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
##
################################################################################
##
##
all:	sw
MAKE := make
YYMMDD=$(shell date +%Y%m%d)
SOURCES:= 
SUBMAKE := $(MAKE) --no-print-directory -C
SOURCES := `find . -name "*.cpp"` `find . -name "*.h"` `find . -name Makefile`\
	`find . -name "*.tex"` `find . -name "*.pdf"`	\
	`find . -name "*.png"` `find . -name "*.eps"`	\
	`find . -name "*.glade"`
ARCHIVED:= archives

.PHONY: sw
sw:
	$(SUBMAKE) sw

.PHONY: doc
doc:
	$(SUBMAKE) doc

.PHONY: archive
archive: $(ARCHIVED)/$(YYMMDD)-xtimesheet.tjz
$(ARCHIVED)/$(YYMMDD)-xtimesheet.tjz:
	@bash -c "if [ ! -e $(ARCHIVED) ]; then mkdir -p $(ARCHIVED); fi"
	@echo Creating $(YYMMDD)-xtimesheet.tjz
	@tar --transform s,^,$(YYMMDD)-xtimesheet/, -chjf $@ $(SOURCES)


.PHONY: clean
clean:
	$(SUBMAKE) sw clean


-include $(OBJDIR)/depends.txt
