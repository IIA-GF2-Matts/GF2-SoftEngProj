OPENGL_LIBS = -lglut -lGL -lGLU

GUICXX = $(shell wx-config --version=3.0 --cxx)
CLICXX = g++

FLAGS = -std=c++11 -g
GUIFLAGS = -DUSE_GUI `wx-config --version=3.0 --cxxflags`
GUILINKFLAGS = `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)


CLISRC = $(wildcard cli/*.cc)
GUISRC = $(wildcard gui/*.cc)
COMSRC = $(wildcard com/*.cc)
LANGSRC = $(wildcard lang/*.cc)
SIMSRC = $(wildcard sim/*.cc)

SRC = $(COMSRC) $(LANGSRC) $(SIMSRC)

G_OBJECTS = $(patsubst %.cc,build/gui/%.o,$(GUISRC) $(SRC))
C_OBJECTS = $(patsubst %.cc,build/cli/%.o,$(CLISRC) $(SRC))


# internationalisation
LANGS = $(wildcard intl/*/mattlab.po)
LANGS_O = $(LANGS:.po=.mo)

%.mo: %.po
	msgfmt -o $@ $<


mattlab: $(G_OBJECTS) $(LANGS_O)
	$(GUICXX) $(FLAGS) -o mattlab $(G_OBJECTS) $(GUILINKFLAGS)

# implementation

.SUFFIXES:	.o .cc


#mattlab: $(G_OBJECTS)
#	$(GUICXX) $(FLAGS) -o mattlab $(G_OBJECTS) $(GUILINKFLAGS)

clisim: $(C_OBJECTS)
	$(CXX) $(FLAGS) -o clisim $(C_OBJECTS)

clean:
	rm -rf build $(LANGS_O) *.o mattlab clisim scanner_unittest parser_unittest

depend:
	makedepend $(SRC) $(GUISRC) $(CLISRC)


build:
	mkdir -p build/cli/cli
	mkdir build/cli/com
	mkdir build/cli/lang
	mkdir build/cli/sim
	mkdir -p build/gui/gui
	mkdir build/gui/com
	mkdir build/gui/lang
	mkdir build/gui/sim

build/cli/%.o: %.cc build
	$(CLICXX) $(FLAGS) -c $< -o $@

build/gui/%.o: %.cc build
	$(GUICXX) $(FLAGS) $(GUIFLAGS) -c $< -o $@



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

scanner_unittest : gtest_main.a scanner_unittest.o build/cli/lang/scanner.o build/cli/com/iposstream.o build/cli/com/cistring.o build/cli/com/names.o build/cli/com/errorhandler.o build/cli/sim/network.o build/cli/sim/devices.o build/cli/com/sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@


parser_unittest.o : lang/parser_unittest.cpp lang/parser.h
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c lang/parser_unittest.cpp

parser_unittest : gtest_main.a parser_unittest.o build/cli/lang/parser.o build/cli/com/errorhandler.o build/cli/com/names.o build/cli/com/autocorrect.o build/cli/sim/network.o build/cli/sim/devices.o build/cli/sim/monitor.o build/cli/com/cistring.o build/cli/com/iposstream.o build/cli/com/sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@



# DO NOT DELETE

build/cli/com/names.o: com/names.h com/cistring.h
build/cli/lang/scanner.o: com/names.h com/cistring.h com/iposstream.h com/sourcepos.h com/errorhandler.h
build/cli/lang/scanner.o: sim/network.h lang/scanner.h com/formatstring.h
build/cli/sim/network.o: sim/network.h com/names.h com/cistring.h com/sourcepos.h com/errorhandler.h com/formatstring.h
build/cli/lang/parser.o: com/errorhandler.h com/sourcepos.h lang/scanner.h com/iposstream.h com/names.h
build/cli/lang/parser.o: com/cistring.h sim/network.h com/autocorrect.h lang/parser.h sim/devices.h sim/monitor.h
build/cli/lang/parser.o: lang/networkbuilder.h com/formatstring.h
build/cli/sim/monitor.o: sim/monitor.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
build/cli/sim/monitor.o: sim/devices.h
build/cli/sim/devices.o: sim/devices.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
build/cli/com/iposstream.o: com/sourcepos.h com/iposstream.h
build/cli/com/cistring.o: com/cistring.h
build/cli/com/errorhandler.o: com/iposstream.h com/sourcepos.h com/errorhandler.h
build/cli/com/sourcepos.o: com/sourcepos.h
build/cli/com/autocorrect.o: com/names.h com/cistring.h com/autocorrect.h com/formatstring.h
build/cli/lang/networkbuilder.o: lang/parser.h com/names.h com/cistring.h lang/scanner.h com/iposstream.h
build/cli/lang/networkbuilder.o: com/sourcepos.h sim/network.h com/errorhandler.h sim/devices.h sim/monitor.h
build/cli/lang/networkbuilder.o: lang/networkbuilder.h com/autocorrect.h com/formatstring.h com/localestrings.h
build/cli/cli/userint.o: cli/userint.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
build/cli/cli/userint.o: sim/devices.h sim/monitor.h lang/scanner.h com/iposstream.h
build/cli/cli/clisim.o: com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h sim/devices.h
build/cli/cli/clisim.o: sim/monitor.h lang/scanner.h com/iposstream.h lang/parser.h lang/networkbuilder.h
build/cli/cli/clisim.o: cli/userint.h

build/gui/com/names.o: com/names.h com/cistring.h
build/gui/lang/scanner.o: com/names.h com/cistring.h com/iposstream.h com/sourcepos.h com/errorhandler.h
build/gui/lang/scanner.o: sim/network.h lang/scanner.h com/formatstring.h
build/gui/sim/network.o: sim/network.h com/names.h com/cistring.h com/sourcepos.h com/errorhandler.h com/formatstring.h
build/gui/lang/parser.o: com/errorhandler.h com/sourcepos.h lang/scanner.h com/iposstream.h com/names.h
build/gui/lang/parser.o: com/cistring.h sim/network.h com/autocorrect.h lang/parser.h sim/devices.h sim/monitor.h
build/gui/lang/parser.o: lang/networkbuilder.h com/formatstring.h
build/gui/sim/monitor.o: sim/monitor.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
build/gui/sim/monitor.o: sim/devices.h
build/gui/sim/devices.o: sim/devices.h com/names.h com/cistring.h sim/network.h com/sourcepos.h com/errorhandler.h
build/gui/com/iposstream.o: com/sourcepos.h com/iposstream.h
build/gui/com/cistring.o: com/cistring.h
build/gui/com/errorhandler.o: com/iposstream.h com/sourcepos.h com/errorhandler.h
build/gui/com/sourcepos.o: com/sourcepos.h
build/gui/com/autocorrect.o: com/names.h com/cistring.h com/autocorrect.h com/formatstring.h
build/gui/sim/networkbuilder.o: lang/parser.h com/names.h com/cistring.h lang/scanner.h com/iposstream.h
build/gui/sim/networkbuilder.o: com/sourcepos.h sim/network.h com/errorhandler.h sim/devices.h sim/monitor.h
build/gui/sim/networkbuilder.o: lang/networkbuilder.h com/autocorrect.h com/formatstring.h com/localestrings.h
build/gui/gui/gui.o: gui/gui.h gui/rearrangectrl_matt.h com/names.h com/cistring.h sim/devices.h sim/network.h
build/gui/gui/gui.o: com/sourcepos.h com/errorhandler.h sim/monitor.h gui/guicanvas.h lang/scanner.h
build/gui/gui/gui.o: com/iposstream.h lang/parser.h lang/networkbuilder.h gui/guierrordialog.h
build/gui/gui/gui.o: gui/guimonitordialog.h gui/guicanvas.inc
build/gui/gui/guierrordialog.o: gui/guierrordialog.h com/errorhandler.h com/sourcepos.h
build/gui/gui/mattlab.o: gui/mattlab.h com/names.h com/cistring.h sim/devices.h sim/network.h com/sourcepos.h
build/gui/gui/mattlab.o: com/errorhandler.h sim/monitor.h lang/parser.h lang/scanner.h com/iposstream.h
build/gui/gui/mattlab.o: lang/networkbuilder.h gui/gui.h gui/rearrangectrl_matt.h gui/guicanvas.h
build/gui/gui/guimonitordialog.o: gui/guimonitordialog.h sim/network.h com/names.h com/cistring.h
build/gui/gui/guimonitordialog.o: com/sourcepos.h com/errorhandler.h

