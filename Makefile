OPENGL_LIBS = -lglut -lGL -lGLU

GUICXX = $(shell wx-config --version=3.0 --cxx)
CLICXX = g++

#-DEXPERIMENTAL
FLAGS = -std=c++11 -g
GUIFLAGS = -DUSE_GUI `wx-config --version=3.0 --cxxflags`
GUILINKFLAGS = `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)


CLISRC = $(wildcard cli/*.cc)
GUISRC = $(wildcard gui/*.cc)
COMSRC = $(wildcard com/*.cc)
LANGSRC = $(wildcard lang/*.cc)
SIMSRC = $(wildcard sim/*.cc)

SRC = $(COMSRC) $(LANGSRC) $(SIMSRC)

G_OBJECTS = $(patsubst %.cc,gui_%.o,$(GUISRC) $(SRC))
C_OBJECTS = $(SRC:.cc=.o) $(CLISRC:.cc=.o)

# implementation

.SUFFIXES:	.o .cc

%.o: %.cc
	$(CLICXX) $(FLAGS) -c $< -o $@

gui_%.o: %.cc
	$(GUICXX) $(FLAGS) $(GUIFLAGS) -c $< -o $@


mattlab: $(G_OBJECTS)
	$(GUICXX) $(FLAGS) -o mattlab $(G_OBJECTS) $(GUILINKFLAGS)

clisim: $(C_OBJECTS)
	$(CXX) $(FLAGS) -o clisim $(C_OBJECTS)


clean:
	rm -f *.o mattlab clisim scanner_unittest parser_unittest

depend:
	makedepend $(SRC) $(GUISRC) $(CLISRC)



# tests

GTEST_DIR = ../../mcj33/googletest/googletest
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include
GTEST_CXXFLAGS = -g -Wall -Wextra -pthread -std=c++11
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	g++ $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	g++ $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


#unittests

scanner_unittest.o : lang/scanner_unittest.cpp lang/scanner.h
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c lang/scanner_unittest.cpp

scanner_unittest : gtest_main.a scanner_unittest.o lang/scanner.o com/iposstream.o com/cistring.o com/names.o com/errorhandler.o sim/network.o sim/devices.o com/sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@


parser_unittest.o : lang/parser_unittest.cpp lang/parser.h
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c lang/parser_unittest.cpp

parser_unittest : gtest_main.a parser_unittest.o lang/parser.o com/errorhandler.o com/names.o com/autocorrect.o sim/network.o sim/devices.o sim/monitor.o com/cistring.o com/iposstream.o com/sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@



# DO NOT DELETE

com/names.o: com/names.h com/cistring.h
lang/scanner.o: com/names.h com/cistring.h com/iposstream.h com/sourcepos.h com/errorhandler.h
lang/scanner.o: sim/network.h lang/scanner.h
sim/network.o: sim/network.h com/names.h com/cistring.h com/sourcepos.h com/errorhandler.h
lang/parser.o: com/errorhandler.h com/sourcepos.h lang/scanner.h com/iposstream.h com/names.h
lang/parser.o: com/cistring.h sim/network.h com/autocorrect.h lang/parser.h sim/devices.h sim/monitor.h
lang/parser.o: lang/networkbuilder.h
sim/monitor.o: sim/monitor.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
sim/monitor.o: sim/devices.h
sim/devices.o: sim/devices.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
com/iposstream.o: com/sourcepos.h com/iposstream.h
com/cistring.o: com/cistring.h
com/errorhandler.o: com/iposstream.h com/sourcepos.h com/errorhandler.h
com/sourcepos.o: com/sourcepos.h
com/autocorrect.o: com/names.h com/cistring.h com/autocorrect.h
lang/networkbuilder.o: lang/parser.h com/names.h com/cistring.h lang/scanner.h com/iposstream.h
lang/networkbuilder.o: com/sourcepos.h sim/network.h com/errorhandler.h sim/devices.h sim/monitor.h
lang/networkbuilder.o: lang/networkbuilder.h com/autocorrect.h
cli/userint.o: cli/userint.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
cli/userint.o: sim/devices.h sim/monitor.h lang/scanner.h com/iposstream.h
cli/clisim.o: com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h sim/devices.h
cli/clisim.o: sim/monitor.h lang/scanner.h com/iposstream.h lang/parser.h lang/networkbuilder.h
cli/clisim.o: cli/userint.h

com/gui_names.o: com/names.h com/cistring.h
lang/gui_scanner.o: com/names.h com/cistring.h com/iposstream.h com/sourcepos.h com/errorhandler.h
lang/gui_scanner.o: sim/network.h lang/scanner.h
sim/gui_network.o: sim/network.h com/names.h com/cistring.h com/sourcepos.h com/errorhandler.h
lang/gui_parser.o: com/errorhandler.h com/sourcepos.h lang/scanner.h com/iposstream.h com/names.h
lang/gui_parser.o: com/cistring.h sim/network.h com/autocorrect.h lang/parser.h sim/devices.h sim/monitor.h
lang/gui_parser.o: lang/networkbuilder.h
sim/gui_monitor.o: sim/monitor.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
sim/gui_monitor.o: sim/devices.h
sim/gui_devices.o: sim/devices.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
com/gui_iposstream.o: com/sourcepos.h com/iposstream.h
com/gui_cistring.o: com/cistring.h
com/gui_errorhandler.o: com/iposstream.h com/sourcepos.h com/errorhandler.h
com/gui_sourcepos.o: com/sourcepos.h
com/gui_autocorrect.o: com/names.h com/cistring.h com/autocorrect.h
sim/gui_networkbuilder.o: lang/parser.h com/names.h com/cistring.h lang/scanner.h com/iposstream.h
sim/gui_networkbuilder.o: com/sourcepos.h sim/network.h com/errorhandler.h sim/devices.h sim/monitor.h
sim/gui_networkbuilder.o: lang/networkbuilder.h com/autocorrect.h
gui/gui_gui.o: gui/gui.h rearrangectrl_matt.h com/names.h com/cistring.h sim/devices.h sim/network.h
gui/gui_gui.o: com/sourcepos.h com/errorhandler.h sim/monitor.h guicanvas.h logo32.xpm lang/scanner.h
gui/gui_gui.o: com/iposstream.h lang/parser.h lang/networkbuilder.h gui/guierrordialog.h
gui/gui_gui.o: gui/guimonitordialog.h guicanvas.cc
gui/gui_guierrordialog.o: gui/guierrordialog.h com/errorhandler.h com/sourcepos.h
gui/gui_mattlab.o: gui/mattlab.h com/names.h com/cistring.h sim/devices.h sim/network.h com/sourcepos.h
gui/gui_mattlab.o: com/errorhandler.h sim/monitor.h lang/parser.h lang/scanner.h com/iposstream.h
gui/gui_mattlab.o: lang/networkbuilder.h gui/gui.h rearrangectrl_matt.h guicanvas.h
gui/gui_guimonitordialog.o: gui/guimonitordialog.h sim/network.h com/names.h com/cistring.h
gui/gui_guimonitordialog.o: com/sourcepos.h com/errorhandler.h

