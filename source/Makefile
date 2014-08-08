#
# A simple Makefile for OpenGL
#

# Paths for libraries and includes for linking (don't change!)

XLIBS =  -L/usr/X11R6/lib -lXext -lXmu -lXt -lXi -lSM -lICE -lpthread
COPTS =  -g3 -Wall  -Werror  -I/usr/include 
LOPTS =  -g3 -Wall -L/usr/lib -lglut -lGLU -lGL -lm $(XLIBS)

# override some of the above variables if we're running on MacOS
UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
XLIBS =  -L/usr/X11R6/lib
COPTS =  -g3 -Wall -ansi -pedantic -I/usr/include
LOPTS = -framework Carbon -framework OpenGL -framework GLUT
endif

# The list of code files, object files, and the compiler to use

# Define our source code (change)
CFILES = colour_ops.c matrix_ops.c vector_ops.c \
	 mygl.c ppm.c \
	 fileio.c main.c

# Define the .o files to correspond with the .c files
OFILES = $(CFILES:.c=.o)

# Use the C compiler
CC = gcc

# Name of the compiled output (change)
TARGET = raytracer


# Makefile terminal commands:  make == make default

default: clean go

.c.o: $(CFILES) 
	$(CC) -c $< $(COPTS)

go: $(OFILES) 
	$(CC) $(OFILES)  -o $(TARGET) $(LOPTS)

clean:
	rm -f $(OFILES) $(TARGET) core

cleanall:
	rm -f $(OFILES) out.ppm *~ $(TARGET) core
