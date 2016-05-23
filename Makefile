OPENGL_LIBS = -lglut -lGL -lGLU

GUICXX = $(shell wx-config --version=3.0 --cxx)
CLICXX = g++

FLAGS = -std=c++11 -g
GUIFLAGS = -DUSE_GUI `wx-config --version=3.0 --cxxflags`
GUILINKFLAGS = `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)

SRC = names.cc scanner.cc network.cc parser.cc monitor.cc devices.cc iposstream.cc cistring.cc errorhandler.cc sourcepos.cc autocorrect.cc networkbuilder.cc
GUISRC = gui.cc guierrordialog.cc mattlab.cc
CLISRC = userint.cc clisim.cc

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

scanner_unittest.o : scanner_unittest.cc scanner.h
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c scanner_unittest.cc

scanner_unittest : gtest_main.a scanner_unittest.o scanner.o iposstream.o cistring.o names.o errorhandler.o network.o devices.o sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@


parser_unittest.o : parser_unittest.cc parser.h
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c parser_unittest.cc

parser_unittest : gtest_main.a parser_unittest.o parser.o errorhandler.o names.o autocorrect.o network.o devices.o monitor.o cistring.o iposstream.o sourcepos.o
	$(CLICXX) $(FLAGS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@







# DO NOT DELETE

logsim.o: logsim.h names.h cistring.h devices.h network.h sourcepos.h
logsim.o: errorhandler.h monitor.h parser.h scanner.h iposstream.h
logsim.o: networkbuilder.h userint.h
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
userint.o: userint.h names.h cistring.h network.h sourcepos.h errorhandler.h
userint.o: devices.h monitor.h scanner.h iposstream.h
iposstream.o: sourcepos.h iposstream.h
cistring.o: cistring.h
errorhandler.o: iposstream.h sourcepos.h errorhandler.h
sourcepos.o: sourcepos.h
autocorrect.o: names.h cistring.h autocorrect.h
networkbuilder.o: parser.h names.h cistring.h scanner.h iposstream.h
networkbuilder.o: sourcepos.h network.h errorhandler.h devices.h monitor.h
networkbuilder.o: networkbuilder.h autocorrect.h

gui_logsim.o: logsim.h names.h cistring.h devices.h network.h sourcepos.h
gui_logsim.o: errorhandler.h monitor.h parser.h scanner.h iposstream.h
gui_logsim.o: networkbuilder.h userint.h
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
gui_gui.o: gui.h names.h cistring.h devices.h network.h sourcepos.h
gui_gui.o: errorhandler.h monitor.h guicanvas.h logo32.xpm scanner.h iposstream.h
gui_gui.o: parser.h networkbuilder.h guierrordialog.h guicanvas.cc
gui_iposstream.o: sourcepos.h iposstream.h
gui_cistring.o: cistring.h
gui_errorhandler.o: iposstream.h sourcepos.h errorhandler.h
gui_sourcepos.o: sourcepos.h
gui_autocorrect.o: names.h cistring.h autocorrect.h
gui_networkbuilder.o: parser.h names.h cistring.h scanner.h iposstream.h
gui_networkbuilder.o: sourcepos.h network.h errorhandler.h devices.h monitor.h
gui_networkbuilder.o: networkbuilder.h autocorrect.h
