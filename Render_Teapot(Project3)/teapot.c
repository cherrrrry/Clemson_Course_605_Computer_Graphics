#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include <math.h>
#include <fcntl.h>

struct point{
	float x, y, z;
};
struct coord{
	float x, y;
};
struct face{
	int v[4];
	int c[4];
	int n[4];
};

#define VPASSES 100
#define JITTER 0.01
#define EYEDX 0.05

int renderMode=0;

unsigned int program;
struct point *vertex, *normal, *tang, *bitang;
struct coord *texcoord;
struct face *quad;

int vertex_count = 0, texcoord_count=0, quad_count=0;

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

void load_obj(char *objfname){
	char buf[512], *parse, *objfile;
	
	FILE *fptr =  fopen(objfname, "r");
	fseek(fptr, 0, SEEK_END);
	int fsize = ftell(fptr);
	objfile = (char*)malloc(fsize*sizeof(char));
	fseek(fptr, 0, SEEK_SET);
	fread(objfile,fsize,sizeof(char),fptr);
	fclose(fptr);

	FILE *objptr = fmemopen(objfile, fsize, "r");
	if(objptr==NULL){
		printf("error");
	}

	do{
		fgets(buf, 512, objptr);
	}while(buf[0]=='#');

	parse = strtok(buf, " \t\n");
	parse = strtok(NULL, " \t\n");
	char *mtllib = parse;

	int normal_count=0;

	while(fgets(buf, 512, objptr)){
		parse = strtok(buf, " \t\n");

		if(!strcmp("v", parse))
			vertex_count++;
		if(!strcmp("vt", parse))
			texcoord_count++;
		if(!strcmp("f", parse))
			quad_count++;
		if(!strcmp("usemtl", parse)){
			parse = strtok(NULL, " \t\n");
			char *usemtl = parse;
		}
	}
	
	fseek(objptr, 0, SEEK_SET);
	vertex = (struct point *)calloc((vertex_count), sizeof(struct point));
	normal = (struct point *)calloc((vertex_count), sizeof(struct point));
	tang = (struct point *)calloc((vertex_count), sizeof(struct point));
	bitang = (struct point *)calloc((vertex_count), sizeof(struct point));
	texcoord = (struct coord *)calloc((texcoord_count), sizeof(struct coord));
	quad = (struct face *)calloc((quad_count), sizeof(struct face));
	int v=0,vn=0,vx=0,vy=0,vt=0,f=0;
	while(fgets(buf, 512, objptr)){
		parse = strtok(buf, " \t\n");
		if(!strcmp("v", parse)){
			parse = strtok(NULL, " \t\n");
			vertex[v].x = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			vertex[v].y = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			vertex[v].z = (float)atof(parse);
			v++;
		}
		if(!strcmp("vn", parse)){
			parse = strtok(NULL, " \t\n");
			normal[vn].x = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			normal[vn].y = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			normal[vn].z = (float)atof(parse);
			vn++;
		}
		if(!strcmp("vx", parse)){
			parse = strtok(NULL, " \t\n");
			tang[vx].x = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			tang[vx].y = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			tang[vx].z = (float)atof(parse);
			vx++;
		}
		if(!strcmp("vy", parse)){
			parse = strtok(NULL, " \t\n");
			bitang[vy].x = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			bitang[vy].y = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			bitang[vy].z = (float)atof(parse);
			vy++;
		}
		if(!strcmp("vt", parse)){
			parse = strtok(NULL, " \t\n");
			texcoord[vt].x = (float)atof(parse);
			parse = strtok(NULL, " \t\n");
			texcoord[vt].y = (float)atof(parse);
			vt++;
		}
		if(!strcmp("f", parse)){
			int i;
			for(i=0;i<4;i++){
				parse = strtok(NULL, " /\t\n");
				quad[f].v[i] = (int)atoi(parse);
				parse = strtok(NULL, " /\t\n");
				quad[f].c[i] = (int)atoi(parse);
				parse = strtok(NULL, " /\t\n");
				quad[f].n[i] = (int)atoi(parse);
			}
			f++;
		}
	}
	fclose(objptr);
	free(objfile);
}

void setup_viewvolume_shape(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 0.1, 20.0);
}

void setup_jitter_view(){
	struct point eye, view, up, vdir, utemp, vtemp;
	
	eye.x = 3.0; eye.y = 3.0; eye.z = 3.0;
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

void trans_center(){
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

void scale(){
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
	float light0_position[] = { 2.5, 2.0, 2.0, 1.0 };
	float light0_direction[] = { -2.5, -2.0, -2.0, 1.0};
void do_lights(){
	float light0_ambient[] = { 0.0, 0.0, 0.0, 0.0 };
	float light0_diffuse[] = { 2.0, 2.0, 2.0, 1.0 }; 
	float light0_specular[] = { 2.25, 2.25, 2.25, 1.0 }; 
	

	// set scene default ambient 
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light0_ambient); 

	// make specular correct for spots 
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1); 
	glLightfv(GL_LIGHT0,GL_AMBIENT,light0_ambient); 
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diffuse); 
	glLightfv(GL_LIGHT0,GL_SPECULAR,light0_specular); 
	glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,1.0); 
	glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180.0); 
	glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0); 
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.2); 
	glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.01); 
	glLightfv(GL_LIGHT0,GL_POSITION,light0_position);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light0_direction);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void do_material(){
	float mat_ambient[] = {0.0,0.0,0.0,1.0};
	float mat_diffuse[] = {1.0,1.1,1.1,1.0};
	float mat_specular[] = {1.0,0.9,1.0,1.0};
	float mat_shininess[] = {60.0};

	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
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
	return(p);
}


void set_uniform(unsigned int p){
	int loc1 = glGetUniformLocation(p, "renderMode");
	int loc2 = glGetUniformLocation(p, "wood");
	int loc3 = glGetUniformLocation(p, "bubble_color");
	int loc4 = glGetUniformLocation(p, "normalmap");
	glUniform1i(loc1, renderMode);
	glUniform1i(loc2, 0);
	glUniform1i(loc3, 1);
	glUniform1i(loc4, 2);
	int location;
	location = glGetUniformLocation(p,"shadowmap");
	glUniform1i(location,3);
}

void draw_table(){
	struct point table[4] = {{-1.0,-0.5,-1.0}, {1.0,-0.5,-1.0}, {1.0,-0.5,1.0}, {-1.0,-0.5,1.0}};
	float tabletexcoord[4][2] = {{0.0,1.0},{1.0,1.0},{1.0,0.0},{0.0,0.0}};
	int i;
	glBegin(GL_QUADS);
	glNormal3f(0.0,1.0,0.0);
	for(i=0;i<4;i++) 
		{	
			glTexCoord2fv(tabletexcoord[i]);
			glVertex3f(5*table[i].x,table[i].y,5*table[i].z);
		}
	glEnd();
}

void draw_teapot(){
	int i,j;
	int index_tangent = glGetAttribLocation(program, "tangent");
	int index_bitangent = glGetAttribLocation(program, "bitangent");
	glBegin(GL_QUADS);
	for (i=0;i<quad_count;i++){
		int v,n,c;
		for(j=0; j<4; j++){
			v = quad[i].v[j];
			n = quad[i].n[j];
			c = quad[i].c[j];
			glNormal3f(normal[n-1].x,normal[n-1].y,normal[n-1].z);
			glTexCoord2f(texcoord[c-1].x,texcoord[c-1].y);
			glVertexAttrib3f(index_tangent, tang[n-1].x, tang[n-1].y, tang[n-1].z);
			glVertexAttrib3f(index_bitangent, bitang[n-1].x, bitang[n-1].y, bitang[n-1].z);
			glVertex3f(vertex[v-1].x,vertex[v-1].y,vertex[v-1].z);
		}
	}
	glEnd();
}

void draw_stuff(){

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,2);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,3);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,4);
	
	struct point eye, view, up;
	eye.x=light0_position[0];
	eye.y=light0_position[1];
	eye.z=light0_position[2];
	view.x=light0_direction[0]+light0_position[0];
	view.y=light0_direction[1]+light0_position[1];
	view.z=light0_direction[2]+light0_position[2];
	up.x=0.0; up.y=1.0; up.z=0.0;

	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER,1);
	draw_table();
	draw_teapot();
	glBindFramebufferEXT(GL_FRAMEBUFFER,0);

	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE4);
	glLoadIdentity();
	glTranslatef(0.0,0.0,-0.05);
	glScalef(0.5,0.5,0.5);
	glTranslatef(1.0,1.0,1.0);
	gluPerspective(45.0, 1.0, 0.1, 20.0);
	gluLookAt(eye.x,eye.y,eye.z,view.x,view.y,view.z,up.x,up.y,up.z);

	glUseProgram(program);
	set_uniform(program);

	setup_viewvolume_shape();
	setup_jitter_view();
	
	renderMode = 0;
	set_uniform(program);
	draw_table();

	renderMode = 1;
	set_uniform(program);
	draw_teapot();
	glFlush();
}

void renderScene(){

	glClear(GL_ACCUM_BUFFER_BIT);

	int view_pass;
	for(view_pass=0;view_pass<VPASSES;view_pass++){
		draw_stuff();
		glAccum(GL_ACCUM,1.0/(float)(VPASSES));
	}
	glAccum(GL_RETURN,1.0);
	glutSwapBuffers();
}


void load_texture(char *filename,unsigned int tid){
	FILE *fptr;
	char buf[512], *parse;
	int im_size, im_width, im_height, max_color;
	unsigned char *texture_bytes; 

	fptr=fopen(filename,"r");
	fgets(buf,512,fptr);
	do{
		fgets(buf,512,fptr);
		} while(buf[0]=='#');
	parse = strtok(buf," \t");
	im_width = atoi(parse);

	parse = strtok(NULL," \n");
	im_height = atoi(parse);

	fgets(buf,512,fptr);
	parse = strtok(buf," \n");
	max_color = atoi(parse);

	im_size = im_width*im_height;
	texture_bytes = (unsigned char *)calloc(3,im_size);
	fread(texture_bytes,3,im_size,fptr);
	fclose(fptr);

	glBindTexture(GL_TEXTURE_2D,tid);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,im_width,im_height,0,GL_RGB, 
		GL_UNSIGNED_BYTE,texture_bytes);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	free(texture_bytes);
}

void build_shadowmap(int tid){
	glBindTexture(GL_TEXTURE_2D,tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 768, 768, 0, 
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER,1);
	glDrawBuffer(GL_NONE); 

	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,tid,0);

	glBindFramebufferEXT(GL_FRAMEBUFFER,0);
}


void initOGL(int argc, char **argv){

	char *objname = argv[1];
	
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_ACCUM|GLUT_DOUBLE);
	glutInitWindowSize(960,820);
	glutInitWindowPosition(100,50);
	glutCreateWindow("teapot");

	glewInit();

	glClearColor(0.35,0.35,0.35,0.0);
	glClearAccum(0.0,0.0,0.0,0.0);
	glEnable(GL_DEPTH_TEST);

	load_obj(objname);
	load_texture("wood.ppm", 1);
	load_texture("teapottex.ppm", 2);
	load_texture("surf.ppm", 3);

	//build_shadowmap(4);

	trans_center();
	scale();

	setup_viewvolume_shape();
	setup_jitter_view();

	do_lights();
	do_material();

	program = set_shaders();
}

void getout(unsigned char key, int x, int y){
	switch(key) {
		case 'q':               
			exit(1);
		default:
			break;
    }
}

int main(int argc, char **argv){
	srandom(123456789);
	initOGL(argc,argv);
	glutDisplayFunc(renderScene);
	glutKeyboardFunc(getout);
	glutMainLoop();
	return 0;
}
