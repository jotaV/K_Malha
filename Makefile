.SUFFIXES: .cpp .cxx .h

# compiler names:
CXX	= g++ 
CC	= gcc

# flags for C++ compiler:
CFLAGS	= -g -Wall 

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	INCFLAGS = `pkg-config --cflags libfreenect`
	LDFLAGS = -lGL -lGLU -lglut -lX11 -lpthread -lfreenect -lusb-1.0 -lm
endif
ifeq ($(UNAME),Darwin)
	INCFLAGS = -I/usr/local/include/libfreenect
	LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT -lpthread \
	-lfreenect -lusb-1.0 -lm
endif

# libraries to link with:
OBJFILES = libView/algebra3.o MeshControl.o AbstractOut.o CamOut.o MeshOut.o main.o

# ***********************************************************************************

all: K_Malha

K_Malha: $(OBJFILES)
	@echo "		   Linking ........"
	@$(CXX) $(CFLAG) $(INCFLAGS) $(OBJFILES) $(LDFLAGS) -o k-malha
	
.cpp.o:	$*.h
	@echo "		  Compiling C++ code ........"
	@$(CXX) -o $*.o -c $(INCFLAGS) $*.cpp
	
clean:	
	@echo "		  Clearing ........"
	@rm -f *.o libView/*.o k-malha
