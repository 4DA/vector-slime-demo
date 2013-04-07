DEBFLAGS=-g
INCFLAGS = -I./glm-0.9.2.7

cube-slime: 	cube-slime.cpp Makefile
		g++ $(DEBFLAGS) $(INCFLAGS) cube-slime.cpp -o cube-slime -lglut -lGL -lGLU -lGLEW
clean:
		rm *.o
