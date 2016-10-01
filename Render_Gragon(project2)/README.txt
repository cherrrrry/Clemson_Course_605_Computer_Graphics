Project #2: Dragon Fire
CPSC 6050 Computer Graphics
 
Group:
 Yu Gu(gu2@g.clemson.edu)
 Rui Chang(rchang@g.clemson.edu)

Compile:
 gcc -L/usr/lib64 -o2 dragon.c -lX11 -lGL -lGLU -lglut -lm -lXmu -o dragon

Execute:
 ./dragon welsh-dragon.ply [eye.x eye.y eye.z]
 			(default eye position: 2.0, 2.0, 2.0)

Control:
 keyboard "s": Switch shader
 keyboard "p": Pause the rotation

Description:

 Light Source:
	Basced on the eye position, set three light sources as Hollywood Lighting.
	Calculate the light position based on the eye position.
 Shading:
	Using normalized Blinn-Phong shader, the reflection model is dot(H,N).
	Switch to Phong shanding with dot(R,V) as reflection model by press "s".
 Also include vertex attays and anti-aliasing.