OPENGL_LIBS = -lglut -lGL -lGLU

CXX = $(shell wx-config --version=3.0 --cxx)

SRC = logsim.cc names.cc scanner.cc network.cc parser.cc monitor.cc devices.cc userint.cc gui.cc guitest.cc iposstream.cc cistring.cc errorhandler.cc

L_OBJECTS = logsim.o names.o scanner.o network.o parser.o monitor.o devices.o userint.o gui.o iposstream.o cistring.o errorhandler.o

G_OBJECTS = guitest.o names.o network.o monitor.o devices.o gui.o cistring.o

# implementation

.SUFFIXES:	.o .cc

.cc.o :
	$(CXX) -std=c++11 -c `wx-config --version=3.0 --cxxflags` -g -o $@ $<

all:    logsim guitest

logsim:	$(L_OBJECTS)
	$(CXX) -std=c++11 -o logsim $(L_OBJECTS) `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)

guitest: $(G_OBJECTS)
	 $(CXX) -std=c++11 -o guitest $(G_OBJECTS) `wx-config --version=3.0 --libs --gl_libs` $(OPENGL_LIBS)

clean:
	rm -f *.o logsim guitest

depend:
	makedepend $(SRC)

# DO NOT DELETE

scanner.o: scanner.h iposstream.h names.h cistring.h errorhandler.h
logsim.o: logsim.h names.h devices.h network.h monitor.h parser.h userint.h
logsim.o: gui.h
names.o: names.h cistring.h
network.o: network.h names.h
parser.o: parser.h names.h network.h devices.h monitor.h
monitor.o: monitor.h names.h network.h devices.h
devices.o: devices.h names.h network.h
userint.o: userint.h names.h network.h devices.h monitor.h
gui.o: gui.h names.h devices.h network.h monitor.h
guitest.o: guitest.h names.h devices.h network.h monitor.h gui.h
