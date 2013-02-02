DEBDLAGS=-g

cube-slime: 	cube-slime.cpp Makefile
		g++ $(DEBFLAGS) cube-slime.cpp -o cube-slime -lglut -lGL -lGLU
clean:
		rm *.o
