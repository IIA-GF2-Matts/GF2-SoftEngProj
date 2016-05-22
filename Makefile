OPENGL_LIBS = -lglut -lGL -lGLU

CXX = $(shell wx-config --version=3.0 --cxx) -DUSE_GUI -std=c++11

SRC = logsim.cc names.cc scanner.cc network.cc parser.cc monitor.cc devices.cc userint.cc gui.cc iposstream.cc cistring.cc errorhandler.cc sourcepos.cc guierrorhandler.cc autocorrect.cc networkbuilder.cc

L_OBJECTS = logsim.o names.o scanner.o network.o parser.o monitor.o devices.o userint.o gui.o iposstream.o cistring.o errorhandler.o sourcepos.o guierrordialog.o autocorrect.o networkbuilder.o


# implementation

.SUFFIXES:	.o .cc

.cc.o :
	$(CXX) -c `wx-config --version=3.0 --cxxflags` -g -o $@ $<

logsim:	$(L_OBJECTS)
	$(CXX) -o logsim $(L_OBJECTS) `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)

clean:
	rm -f *.o logsim scanner_unittest

depend:
	makedepend $(SRC)



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
	$(CXX) $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(GTEST_CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


#unittests

scanner_unittest.o : scanner_unittest.cc scanner.h
	$(CXX) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c scanner_unittest.cc

scanner_unittest : scanner.o scanner_unittest.o gtest_main.a iposstream.o cistring.o names.o errorhandler.o
	$(CXX) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@


parser_unittest.o : parser_unittest.cc parser.h
	$(CXX) $(COMPILE_OPTIONS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -c parser_unittest.cc

parser_unittest : parser_unittest.o gtest_main.a parser.o errorhandler.o names.o autocorrect.o network.o devices.o monitor.o cistring.o iposstream.o sourcepos.o
	$(CXX) $(COMPILE_OPTIONS) $(GTEST_CPPFLAGS) $(GTEST_CXXFLAGS) -lpthread $^ -o $@







# DO NOT DELETE

iposstream.o: iposstream.h sourcepos.h 
scanner.o: scanner.h iposstream.h names.h cistring.h errorhandler.h sourcepos.h
logsim.o: logsim.h names.h devices.h network.h monitor.h parser.h userint.h
logsim.o: gui.h
names.o: names.h cistring.h sourcepos.h
network.o: network.h names.h
parser.o: parser.h names.h network.h devices.h monitor.h networkbuilder.h
monitor.o: monitor.h names.h network.h devices.h
devices.o: devices.h names.h network.h
userint.o: userint.h names.h network.h devices.h monitor.h
gui.o: gui.h names.h devices.h network.h monitor.h guicanvas.cc guicanvas.h guierrordialog.h
guicanvas.o: guicanvas.h names.h monitor.h
guierrorhandler.o: guierrordialog.h errorhandler.h
autocorrect.o: cistring.h
networkbuilder.o: networkbuilder.h names.h network.h devices.h monitor.h 