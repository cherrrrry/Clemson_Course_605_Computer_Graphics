all: clean teapot

teapot:
	gcc -L/usr/lib64 -O2 teapot.c -lX11 -lGLEW -lGL -lGLU -lglut -lXmu -lXi -lm -o teapot

clean:
	rm -f teapot
