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






# DO NOT DELETE

names.o: names.h cistring.h
scanner.o: names.h cistring.h iposstream.h sourcepos.h errorhandler.h
scanner.o: network.h scanner.h
network.o: network.h names.h cistring.h sourcepos.h errorhandler.h
parser.o: errorhandler.h sourcepos.h scanner.h iposstream.h names.h
parser.o: cistring.h network.h autocorrect.h parser.h devices.h monitor.h
parser.o: networkbuilder.h
monitor.o: monitor.h names.h cistring.h network.h sourcepos.h errorhandler.h
monitor.o: devices.h
devices.o: devices.h names.h cistring.h network.h sourcepos.h errorhandler.h
iposstream.o: sourcepos.h iposstream.h
cistring.o: cistring.h
errorhandler.o: iposstream.h sourcepos.h errorhandler.h
sourcepos.o: sourcepos.h
autocorrect.o: names.h cistring.h autocorrect.h
networkbuilder.o: parser.h names.h cistring.h scanner.h iposstream.h
networkbuilder.o: sourcepos.h network.h errorhandler.h devices.h monitor.h
networkbuilder.o: networkbuilder.h autocorrect.h
userint.o: userint.h names.h cistring.h network.h sourcepos.h errorhandler.h
userint.o: devices.h monitor.h scanner.h iposstream.h
clisim.o: names.h cistring.h network.h sourcepos.h errorhandler.h devices.h
clisim.o: monitor.h scanner.h iposstream.h parser.h networkbuilder.h
clisim.o: userint.h

gui_names.o: names.h cistring.h
gui_scanner.o: names.h cistring.h iposstream.h sourcepos.h errorhandler.h
gui_scanner.o: network.h scanner.h
gui_network.o: network.h names.h cistring.h sourcepos.h errorhandler.h
gui_parser.o: errorhandler.h sourcepos.h scanner.h iposstream.h names.h
gui_parser.o: cistring.h network.h autocorrect.h parser.h devices.h monitor.h
gui_parser.o: networkbuilder.h
gui_monitor.o: monitor.h names.h cistring.h network.h sourcepos.h errorhandler.h
gui_monitor.o: devices.h
gui_devices.o: devices.h names.h cistring.h network.h sourcepos.h errorhandler.h
gui_iposstream.o: sourcepos.h iposstream.h
gui_cistring.o: cistring.h
gui_errorhandler.o: iposstream.h sourcepos.h errorhandler.h
gui_sourcepos.o: sourcepos.h
gui_autocorrect.o: names.h cistring.h autocorrect.h
gui_networkbuilder.o: parser.h names.h cistring.h scanner.h iposstream.h
gui_networkbuilder.o: sourcepos.h network.h errorhandler.h devices.h monitor.h
gui_networkbuilder.o: networkbuilder.h autocorrect.h
gui_gui.o: gui.h rearrangectrl_matt.h names.h cistring.h devices.h network.h
gui_gui.o: sourcepos.h errorhandler.h monitor.h guicanvas.h logo32.xpm scanner.h
gui_gui.o: iposstream.h parser.h networkbuilder.h guierrordialog.h
gui_gui.o: guimonitordialog.h guicanvas.cc
gui_guierrordialog.o: guierrordialog.h errorhandler.h sourcepos.h
gui_mattlab.o: mattlab.h names.h cistring.h devices.h network.h sourcepos.h
gui_mattlab.o: errorhandler.h monitor.h parser.h scanner.h iposstream.h
gui_mattlab.o: networkbuilder.h gui.h rearrangectrl_matt.h guicanvas.h
gui_guimonitordialog.o: guimonitordialog.h network.h names.h cistring.h
gui_guimonitordialog.o: sourcepos.h errorhandler.h

