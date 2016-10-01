/*
 Project #2: Dragon Fire
 CPSC 6050 Computer Graphics
 
 Group:
 Yu Gu(gu2@g.clemson.edu)
 Rui Chang(rchang@g.clemson.edu)

 Execute:
 ./dragon welsh-dragon.ply [eye.x eye.y eye.z]
 			(default eye position: 2.0, 2.0, 2.0)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <fcntl.h>

struct point{
	float x, y, z;
};

struct poly{
	unsigned char ic;
	int v0;
	int v1;
	int v2;
};

#define SPOLY_SIZE 13
#define VPASSES 10
#define JITTER 0.01
#define EYEDX 0.05

struct point eye;
int polygon_count;
GLfloat *vertices;
int renderMode, pause;
int angle = 0;

double genrand(){
	return(((double)(random()+1))/2147483649.);
}

struct point cross(struct point u, struct point v){
	struct point w;
	w.x = u.y*v.z - u.z*v.y;
	w.y = -(u.x*v.z - u.z*v.x);
	w.z = u.x*v.y - u.y*v.x;
	return(w);
}

struct point unit_length(struct point u){
	double length;
	struct point v;
	length = sqrt(u.x*u.x+u.y*u.y+u.z*u.z);
	v.x = u.x/length;
	v.y = u.y/length;
	v.z = u.z/length;
	return(v);
}

char *read_shader_program(char *filename) {
	FILE *fp;
	char *content = NULL;
	int fd, count;
	fd = open(filename,O_RDONLY);
	count = lseek(fd,0,SEEK_END);
	close(fd);
	content = (char *)calloc(1,(count+1));
	fp = fopen(filename,"r");
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
	fclose(fp);
	return content;
}

void setup_viewvolume_shape(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 0.1, 20.0);
}

void setup_jitter_view(){
	struct point  view, up, vdir, utemp, vtemp;
	
	view.x = JITTER*genrand(); view.y = JITTER*genrand(); view.z = JITTER*genrand();
	up.x = 0.0; up.y = 1.0; up.z = 0.0;
	vdir.x = view.x - eye.x; vdir.y = view.y - eye.y; vdir.z = view.z - eye.z;

	vtemp = cross(vdir,up);
	utemp = cross(vtemp,vdir);
	up = unit_length(utemp);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, view.x, view.y, view.z, up.x, up.y, up.z);
}

void build_list(char *vdfname, struct point **vertex, struct poly **tris, int *vertex_count){
	int ret;
	FILE *fptr =  fopen(vdfname, "r");
	
	char buf[512];
	char* parse;
	
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	parse = strtok(buf, " ");
	parse = strtok(NULL, " ");
	parse = strtok(NULL, " ");
	(*vertex_count) = (int)atoi(parse);
	fprintf(stderr, "found %d vertices\n", (*vertex_count));
	
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);
	parse = strtok(buf, " ");
	parse = strtok(NULL, " ");
	parse = strtok(NULL, " ");
	polygon_count = (int)atoi(parse);
	fprintf(stderr, "found %d polygons\n", polygon_count);
	
	fgets(buf, 512, fptr);
	fgets(buf, 512, fptr);

	*vertex = (struct point *)calloc((*vertex_count), sizeof(struct point));
	fread(&(*vertex)[0], sizeof(struct point), (*vertex_count), fptr);

	*tris = (struct poly *)calloc(polygon_count, sizeof(struct poly));

	char *temp;
	temp = (unsigned char *)calloc(polygon_count, SPOLY_SIZE);
	fread(temp, SPOLY_SIZE, polygon_count, fptr);
	
	int i;
	for(i=0; i<polygon_count; i++){
		(*tris)[i].ic = temp[i*SPOLY_SIZE];
		(*tris)[i].v0 = *(int *)&temp[i*SPOLY_SIZE+1];
		(*tris)[i].v1 = *(int *)&temp[i*SPOLY_SIZE+5];
		(*tris)[i].v2 = *(int *)&temp[i*SPOLY_SIZE+9];
	}
}

void setup_normal(struct point *vertex, struct poly *tris, struct point **normal){
	int i;
	*normal = (struct point *)calloc(polygon_count, sizeof(struct point));
	
	for(i=0; i<polygon_count; i++){
		int v0 = tris[i].v0,
			v1 = tris[i].v1,
			v2 = tris[i].v2;
		
		struct point v1_v0;
		v1_v0.x = vertex[v1].x-vertex[v0].x;
		v1_v0.y = vertex[v1].y-vertex[v0].y;
		v1_v0.z = vertex[v1].z-vertex[v0].z;

		struct point v2_v0;
		v2_v0.x = vertex[v2].x-vertex[v0].x;
		v2_v0.y = vertex[v2].y-vertex[v0].y;
		v2_v0.z = vertex[v2].z-vertex[v0].z;

		(*normal)[i] = cross(v1_v0, v2_v0);
		(*normal)[i] = unit_length((*normal)[i]);
	}
}

void trans_center(struct point *vertex, int vertex_count){
	int i;
	struct point avg={0, 0, 0};
	for(i=0; i<vertex_count; i++){
		avg.x += vertex[i].x;
		avg.y += vertex[i].y;
		avg.z += vertex[i].z;
	}

	avg.x /= vertex_count;
	avg.y /= vertex_count;
	avg.z /= vertex_count;
	
	for(i=0; i<vertex_count; i++){
		vertex[i].x -= avg.x;
		vertex[i].y -= avg.y;
		vertex[i].z -= avg.z;
	}
}

void scale(struct point *vertex, int vertex_count){
	int i;
	float max = vertex[0].x;
	for(i=0; i<vertex_count; i++){
		max = max > fabs(vertex[i].x) ? max : fabs(vertex[i].x);
		max = max > fabs(vertex[i].y) ? max : fabs(vertex[i].y);
		max = max > fabs(vertex[i].z) ? max : fabs(vertex[i].z);
	}
	
	for(i=0; i<vertex_count; i++){
		vertex[i].x /= max;
		vertex[i].y /= max;
		vertex[i].z /= max;
	}
}

void setup_vertices(struct point *vertex, struct poly *tris, struct point *normal){
	int i, j;
	vertices = (GLfloat *)calloc(3*polygon_count*3*2, sizeof(GLfloat));
	for(i=0; i<polygon_count; i++){
		int v[3];
		v[0] = tris[i].v0;
		v[1] = tris[i].v1;
		v[2] = tris[i].v2;
		
		for(j=0; j<3; j++){
			vertices[i*3*3+j*3] = vertex[v[j]].x;
			vertices[i*3*3+j*3+1] = vertex[v[j]].y;
			vertices[i*3*3+j*3+2] = vertex[v[j]].z;

			vertices[polygon_count*3*3+i*3*3+j*3] = normal[i].x;
			vertices[polygon_count*3*3+i*3*3+j*3+1] = normal[i].y;
			vertices[polygon_count*3*3+i*3*3+j*3+2] = normal[i].z;
		}
	}
}

void update(){
	if(pause == 0){
		angle+=1;
		angle %= 360;
		glutPostRedisplay();
	}
}

void draw_stuff(){
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glRotatef(180.0, 0.0, 1.0, 0.0);
	glRotatef(-angle, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, polygon_count*3);
	glFlush();
}

void go(){
	glClear(GL_ACCUM_BUFFER_BIT);
	int view_pass;
	for(view_pass=0;view_pass<VPASSES;view_pass++){
		//setup_viewvolume_shape();
		setup_jitter_view();
		draw_stuff();
		glAccum(GL_ACCUM,1.0/(float)(VPASSES));
	}
	glAccum(GL_RETURN,1.0);
	glutSwapBuffers();
}

void do_lights(){

	if(abs(eye.x)+abs(eye.z)<0.5){
		eye.x = 0.3;
		eye.z = 0.3;
	}
	float t = sqrt(eye.x*eye.x + eye.y*eye.y);
	float PI = 3.1416;
	float KEYY = t*tan((abs(atan(eye.y/t))+30)*PI/180);
	float FILLX = eye.x - cos(abs(atan(eye.x/eye.z))*PI/180);
	float FILLY = eye.z + cos(abs(atan(eye.x/eye.z))*PI/180);
	float light0_position[] = { eye.x, KEYY, eye.z, 1.0 };
	float light1_position[] = { FILLX, FILLY, 0.9*KEYY, 1.0 };
	float light2_position[] = { -eye.x, 1.5, -eye.z, 1.0 };

	glLightfv(GL_LIGHT0,GL_POSITION,light0_position);
	glLightfv(GL_LIGHT1,GL_POSITION,light1_position);
	glLightfv(GL_LIGHT2,GL_POSITION,light2_position);
}

void do_material(){
	float mat_ambient[] = {0.0,0.0,0.0,1.0};
	float mat_diffuse[] = {2.0,1.0,0.1,1.0};
	float mat_specular[] = {1.0,0.9,0.0,1.0};
	float mat_shininess[] = {3.0};

	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
}

unsigned int set_shaders(){
	GLint vertCompiled, fragCompiled;
	char *vs, *fs;
	GLuint v, f, p;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = read_shader_program("phongEC.vert");
	fs = read_shader_program("phongEC.frag");

	glShaderSource(v,1,(const char **)&vs,NULL);
	glShaderSource(f,1,(const char **)&fs,NULL);
	free(vs);
	free(fs); 
	glCompileShader(v);
	glCompileShader(f);
	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);
	glLinkProgram(p);
	glUseProgram(p);
	return(p);
}

void set_uniform(unsigned int p){
	int loc1 = glGetUniformLocation(p, "renderMode");
	glUniform1i(loc1, renderMode);
}

int mybuf = 1;

void initOGL(int argc, char **argv){
	struct point *vertex, *normal;
	struct poly *tris;
	char *vdfname = argv[1];
	int vertex_count;

	if(argv[2] == NULL){ 	
		eye.x = 2.0; eye.y = 2.0; eye.z = 2.0;
	}else{
		eye.x = atof(argv[2]); eye.y = atof(argv[3]); eye.z = atof(argv[4]);
	}

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_ACCUM|GLUT_DOUBLE);
	glutInitWindowSize(512,512);
	glutInitWindowPosition(100,50);
	glutCreateWindow("dragon");
	glClearColor(0.35,0.35,0.35,0.0);
	glClearAccum(0.0,0.0,0.0,0.0);
	glEnable(GL_DEPTH_TEST);

	setup_viewvolume_shape();
	setup_jitter_view();

	build_list(vdfname, &vertex, &tris, &vertex_count);

	trans_center(vertex, vertex_count);
	scale(vertex, vertex_count);

	setup_normal(vertex, tris, &normal);
	setup_vertices(vertex, tris, normal);

	free(vertex);
	free(tris);
	free(normal);

	do_lights();
	do_material();

	unsigned int program = set_shaders();
	set_uniform(program);


	glBindBuffer(GL_ARRAY_BUFFER,mybuf);
	glBufferData(GL_ARRAY_BUFFER,polygon_count*3*3*2*sizeof(float),vertices,GL_STATIC_DRAW);
	
	glVertexPointer(3,GL_FLOAT,3*sizeof(GLfloat),NULL+0);
	glNormalPointer(GL_FLOAT,3*sizeof(GLfloat),NULL+3*polygon_count*3*sizeof(GLfloat));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
}

void getout(unsigned char key, int x, int y){
	switch(key) {
		case 's':
			renderMode = !renderMode;
			unsigned int program = set_shaders();
			set_uniform(program);
			glutPostRedisplay();
			break;
		case 'p':
			++pause;
			pause %= 2;
			glutPostRedisplay();
			break;
		case 'q':               
			glDeleteBuffers(1,&mybuf);
			exit(1);
		default:
			break;
    }
}

int main(int argc, char **argv){
	srandom(123456789);
	initOGL(argc,argv);
	glutDisplayFunc(go);
	glutIdleFunc(update);
	glutKeyboardFunc(getout);
	glutMainLoop();
	return 0;
}
