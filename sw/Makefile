################################################################################
##
## Filename: 	src/Makefile
##
## Project:	Xtimesheet, a very simple text-based timesheet tracking program
##
## Purpose:	Keeps track of hours performed on a daily basis
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
##
## Copyright (C) 2017, Gisselquist Technology, LLC
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
all:
ARCH	?= pc
OBJDIR	:= obj-$(ARCH)
BINDIR	:= ../bin-$(ARCH)
CXX:= g++
SED:= sed

YYMMDD:=`date +%Y%m%d`

STATIC  =
DEBUG=    -g
CFLAGS	= $(DEBUG) -Wall `pkg-config --cflags gtksourceviewmm-3.0 gtk+-3.0 gtkmm-3.0 gmodule-2.0 gmodule-export-2.0`
LIBS	= $(DEBUG) $(STATIC) -export-dynamic `pkg-config --libs gtksourceviewmm-3.0 gtk+-3.0 gtkmm-3.0 gmodule-2.0 gmodule-export-2.0`
SOURCES = xtimesheet.cpp timecard.cpp gladef.cpp
OBNAMES= $(subst .c,.o,$(subst .cpp,.o,$(SOURCES)))
POSSHDRS :=$(subst .cpp,.h,$(SOURCES))
HEADERS  := $(foreach header,$(POSSHDRS),$(wildcard $(header)))
XTRASRC = thisweek.cpp totalhrs.cpp thismonth.cpp
XTRAOBJ = $(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst .cpp,.o,$(XTRASRC))))
OBJECTS= $(addprefix $(OBJDIR)/,$(subst .c,.o,$(subst .cpp,.o,$(SOURCES))))

APP=	xtimesheet
PROGRAMS := $(APP) thisweek thismonth totalhrs byday
.PHONY: all
all:	$(addprefix $(BINDIR)/,$(PROGRAMS))

.PHONY: install
install: all
	cp $(BINDIR)/$(APP) $(BINDIR)/thisweek $(BINDIR)/thismonth $(BINDIR)/totalhrs $(BINDIR)/byday $(HOME)/bin

.PHONY: $(OBNAMES)
$(OBJDIR)/%.o: %.cpp
	$(mk-objdir)
	$(CXX) $(CFLAGS) -c $< -o $@

$(BINDIR)/$(APP):	$(OBJECTS)
	$(mk-bindir)
	$(CXX) $(LIBS) -o $@ $^ $(LIBS)
$(BINDIR)/thisweek: $(OBJDIR)/thisweek.o $(OBJDIR)/timecard.o
	$(mk-bindir)
	$(CXX) $(LIBS) -o $@ $^ $(LIBS)
$(BINDIR)/thismonth: $(OBJDIR)/thismonth.o $(OBJDIR)/timecard.o
	$(mk-bindir)
	$(CXX) $(LIBS) -o $@ $^ $(LIBS)
$(BINDIR)/totalhrs: $(OBJDIR)/totalhrs.o $(OBJDIR)/timecard.o
	$(mk-bindir)
	$(CXX) $(LIBS) -o $@ $^ $(LIBS)
$(BINDIR)/byday: $(OBJDIR)/byday.o $(OBJDIR)/timecard.o
	$(mk-bindir)
	$(CXX) $(LIBS) -o $@ $^ $(LIBS)

PKGLIBS := -Wl,-Bstatic -pthread -Wl,-Bstatic -lgtksourceviewmm-3.0 -lgtksourceview-3.0 -Wl,-E -lgtkmm-3.0 -latkmm-1.6 -lgdkmm-3.0 -lgiomm-2.4 -lpangomm-1.4 -lgtk-3 -lglibmm-2.4 -lcairomm-1.0 -Wl,-Bdynamic -lgdk-3 -latk-1.0 -lgio-2.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lpango-1.0 -lcairo -lsigc-2.0 -lgobject-2.0 -lgmodule-2.0 -lglib-2.0
ALTLIBS	= `pkg-config --libs gtksourceviewmm-3.0 gtk+-3.0 gtkmm-3.0 gmodule-2.0 gmodule-export-2.0`
$(BINDIR)/$(APP)-static:	$(OBJECTS)
	$(mk-bindir)
	$(CXX) $(PKGLIBS) -o $@ $^ $(LIBS)

$(OBJDIR)/mkglade.o: mkglade.cpp
	$(mk-bindir)
	$(CXX) -c -g mkglade.cpp -o $@

$(BINDIR)/mkglade: $(OBJDIR)/mkglade.o
	$(mk-bindir)
	$(CXX) $^ -o $@

gladef.cpp: timesheet.glade $(BINDIR)/mkglade
	$(mk-bindir)
	$(BINDIR)/mkglade timesheet.glade gladef
sm_splash.cpp: ../gfx/gt-sm-splash.png
	gdk-pixbuf-csource --name=sm_splash ../gfx/gt-sm-splash.png > $@

define	mk-bindir
	@bash -c "if [ ! -e $(BINDIR) ]; then mkdir -p $(BINDIR); fi"
endef

define	mk-objdir
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef

define	build-depends
	@echo "Buidling dependency file"
	$(CXX) $(CFLAGS) -MM $(SOURCES) > $(OBJDIR)/xdep.txt
	$(SED) -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdep.txt > $(OBJDIR)/depends.txt
endef

.PHONY: depends
depends: tags
	$(mk-objdir)
	$(build-depends)

tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/* $(BINDIR)/$(APP) sm_splash.cpp gladef.cpp gladef.h


-include $(OBJDIR)/depends.txt
