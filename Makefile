DEBFLAGS=-g
INCFLAGS = -I./glm-0.9.2.7 -I./glsw
LIBFILES = glsw/glsw.c glsw/bstrlib.c

cube-slime: 	cube-slime.cpp Makefile
		g++ $(DEBFLAGS) $(INCFLAGS) cube-slime.cpp  $(LIBFILES) -o cube-slime -lglut -lGL -lGLU -lGLEW
clean:
		rm *.o
