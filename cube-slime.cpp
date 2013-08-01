#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
// #define _CRT_TERMINATE_DEFINED
// void __cdecl abort(void);
#include <GL/glut.h>
#include "getopt.h"
#endif

#ifndef _WIN32
#include <unistd.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <algorithm>
#include <math.h>

#include <iostream>
#include <istream>
#include <fstream>
#include <cstring>
#include <ctime>
using namespace std;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

//light stuff
float light_direction[4] = {0, -0.5, -0.7, 0.0};
//----------

typedef glm::mat3 mat3;
typedef glm::vec3 vec3;
const float pi = 3.14159265 ; // For portability across platforms
GLuint cubeVertBuffer;
GLuint cubeNormalBuffer;
GLuint cubeVAO;
int cubeVertexNum;
GLuint theProgram;
GLuint cubeIndexBO;

void initCube(void);
static void redraw(void);
GLuint initShader(GLenum eShaderType, const std::string &strShaderFile);
GLuint CreateProgram(const std::vector<GLuint> &shaderList);

GLuint basicOffsetUn;
GLuint perspectiveMatrixUn;
GLuint tUniform;
GLuint magnitudeUniform;
GLuint fcenterUniform;
GLuint axisUniform;
GLuint lightDirUniform;
GLuint lposUniform;
GLuint lightIntensityUniform;

GLuint ax1un, ax2un, ax3un, ax4un;
GLuint t1un, t2un, t3un, t4un;
GLuint dmod1un, dmod2un, dmod3un, dmod4un;

float perspectiveMatrix[16];
float fFrustumScale = 1.0f; float fzNear = 0.5f; float fzFar = 90.0f;


int maxtime = 1100;
float rSpeed = 2;

bool wireframe = false;

struct Cube {
	vector<GLfloat> verts;
	vector<int> ids;
	vector<vec3> cols;
	vector<float> normals;
} sCube;

int fpartition = 20;

vec3 force_center(0,-10,0);
float DISPLACE_PER_UNIT = 5;

// cube unfolding
// looking to -Z axis
//  ___________
// |     |     |
// |  1  |  2  |
// |_____|_____|_____
//       |     |     |
//       |  3  |  4  |
//       |_____|_____|_____
//             |     |     |
//             |  5  |  6  |
//             |_____|_____|
//
// from - (front, left, bottom)
// to - (back, right, top)

void generatePolyCubeVerts(vec3 from, vec3 to, int face_fpartition, Cube &c) {
	float sx = (to.x - from.x) / face_fpartition;
	float sy = (to.y - from.y) / face_fpartition;
	float sz = (to.z - from.z) / face_fpartition;
	
	int cx = 127.0 / face_fpartition;

	//indices
	for (int oft, quad = 0; quad < face_fpartition * face_fpartition * 6 ; quad++) {
		oft = quad * 4;
		
		c.ids.push_back (oft);
		c.ids.push_back (oft+3);
		c.ids.push_back (oft+2);

		c.ids.push_back (oft);
		c.ids.push_back (oft+2);
		c.ids.push_back (oft+1);
	}

	//1 (front)

	for (int px = 0; px < face_fpartition; px++)
		for (int py = 0; py < face_fpartition; py++) {

			c.verts.push_back(from.x + px * sx); c.verts.push_back(from.y + (py+1) * sy); c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back(from.x + px * sx); c.verts.push_back(from.y + py * sy);     c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back(from.x + (px+1) * sx); c.verts.push_back( from.y + py * sy);     c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back(from.x + (px+1) * sx); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z); c.verts.push_back(1.0);
		}
	
	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(-1.0); c.normals.push_back(0.0);
	}

	//4 (back)
	for (int px = 0; px < face_fpartition; px++)
		for (int py = 0; py < face_fpartition; py++) {
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + py * sy);     c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + (py+1) * sy); c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + py * sy);     c.verts.push_back(to.z); c.verts.push_back(1.0);

		}
	
	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(1.0); c.normals.push_back(0.0);
	}

		     
 	// 2 (right)
	for (int pz = 0; pz < face_fpartition; pz++)
		for (int py = 0; py < face_fpartition; py++) {
			c.verts.push_back( to.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(1.0); c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}

	// // 6 (left)
	for (int pz = 0; pz < face_fpartition; pz++)
		for (int py = 0; py < face_fpartition; py++) {
			c.verts.push_back( from.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + (pz+1)* sz);  c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(-1.0); c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}
	
	// 5 (top)
	for (int pz = 0; pz < face_fpartition; pz++)
		for (int px = 0; px < face_fpartition; px++) {
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     to.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     to.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( to.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( to.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(1.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}
	
	// //3 (bottom)
	for (int pz = 0; pz < face_fpartition; pz++)
		for (int px = 0; px < face_fpartition; px++) {
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     from.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     from.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( from.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( from.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < face_fpartition * face_fpartition * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(-1.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}
	
}

void initVertexBuffer() {
	cubeVertexNum = sCube.verts.size();
	glGenBuffers(1, &cubeVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, cubeVertexNum * sizeof(float), &sCube.verts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cubeNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, cubeVertexNum * sizeof(float), &sCube.normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &cubeIndexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sCube.ids.size() * sizeof(int), &sCube.ids[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void initCube(void)
{
	generatePolyCubeVerts(vec3(-10,-10,-10), vec3(10,10,10), fpartition, sCube);
	initVertexBuffer();

	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	std::vector<GLuint> shaderList;
	 
	shaderList.push_back(initShader(GL_VERTEX_SHADER, "cube.vert"));
	shaderList.push_back(initShader(GL_FRAGMENT_SHADER, "cube.frag"));
	shaderList.push_back(initShader(GL_GEOMETRY_SHADER, "cube.geom"));
	theProgram = CreateProgram(shaderList);

	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	basicOffsetUn = glGetUniformLocation(theProgram, "basic_offset");
	perspectiveMatrixUn = glGetUniformLocation(theProgram, "perspectiveMatrix");

	tUniform = glGetUniformLocation(theProgram, "T");
	magnitudeUniform = glGetUniformLocation(theProgram, "magnitude");
	fcenterUniform = glGetUniformLocation(theProgram, "force_center");
	// axisUniform = glGetUniformLocation(theProgram, "axis");
	lightDirUniform = glGetUniformLocation(theProgram, "light_direction");
	lposUniform = glGetUniformLocation(theProgram, "l_pos");
	lightIntensityUniform = glGetUniformLocation(theProgram, "light_intensity");

	ax1un = glGetUniformLocation(theProgram, "ax1");
	ax2un = glGetUniformLocation(theProgram, "ax2");
	ax3un = glGetUniformLocation(theProgram, "ax3");
	ax4un = glGetUniformLocation(theProgram, "ax4");

	t1un = glGetUniformLocation(theProgram, "t1");
	t2un = glGetUniformLocation(theProgram, "t2");
	t3un = glGetUniformLocation(theProgram, "t3");
	t4un = glGetUniformLocation(theProgram, "t4");

	dmod1un = glGetUniformLocation(theProgram, "dmod1");
	dmod2un = glGetUniformLocation(theProgram, "dmod2");
	dmod3un = glGetUniformLocation(theProgram, "dmod3");
	dmod4un = glGetUniformLocation(theProgram, "dmod4");
	
	memset(perspectiveMatrix, 0, sizeof(float) * 16);
	perspectiveMatrix[0] = fFrustumScale;
	perspectiveMatrix[5] = fFrustumScale;
	perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
	perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
	perspectiveMatrix[11] = -1.0f;
}

GLuint CreateProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();
    
	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);
    
	glLinkProgram(program);
    
	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		cerr << "Linker failure: " << strInfoLog;
		delete[] strInfoLog;
	}
	
	for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}

GLuint initShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	
	ifstream myReadFile;
	myReadFile.open(strShaderFile.c_str());
	string strFileData;
	string fileLine;
	char buf[256];
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			//getline(myReadFile, fileLine);
			myReadFile.getline(buf, 256);
			strFileData.append(buf);
			strFileData.append("\n");
		}
	}
	myReadFile.close();

	const char *src_data = strFileData.c_str();
	glShaderSource(shader, 1, &src_data, NULL);
    
	glCompileShader(shader);
    
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        
		const char *strShaderType = NULL;
		switch(eShaderType) {
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}
        
		cerr << "Compile failure in " <<  strShaderType << " shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}

	return shader;
}


float genVal() {
	return (100 - (rand() % 200)) / 200.0;
}

float genNVal() {
	return (rand() % 100) / 100.0;
}

glm::vec3 genNonColinear(glm::vec3 &prev) {
	const float COS_PI = -1;
		
	glm::vec3 nv;

	do {
		nv = glm::normalize (glm::vec3
				     (genVal(), genVal(), genVal()));
	}
	
	while (glm::dot(prev, nv) < COS_PI);
	// while (false);

	return nv;
}

void setUniforms(int t) {
	glUniform4f(basicOffsetUn, 0.0f, -6.0f, -30.0f, 0);

	glUniform1f(tUniform, t);
	glUniform1f(magnitudeUniform, DISPLACE_PER_UNIT);

	glUniform4f(fcenterUniform,
		    force_center.x, force_center.y, force_center.z, 1.0);

	glUniform3fv(lightDirUniform, 1, light_direction);
	glUniform4f(lightIntensityUniform, 
		     1.0, 1.0, 1.0, 1.0);
	
	static float ax1[] = {0, 1, -0.2};
	static float ax2[] = {0.4, 0.4, 0.0};
	static float ax3[] = {0, -0.6, 0.2};
	static float ax4[] = {-0.2, -0.3, -0.0};

	static glm::vec3 vax1 = glm::vec3(0, 1, -0.2);
	static glm::vec3 vax2 = glm::vec3(0.4, 0.4, 0.0);
	static glm::vec3 vax3 = glm::vec3(0, -0.6, 0.2);
	static glm::vec3 vax4 = glm::vec3(-0.2, -0.3, -0.0);

	static int t1=500, t2=750, t3=700, t4=870;
	static float dmod1=1, dmod2=0.95, dmod3=1, dmod4=0.65;

	static glm::vec3 rv1, nv1=vax1;
	static glm::vec3 rv2, nv2=vax2;
	static glm::vec3 rv3, nv3=vax3;
	static glm::vec3 rv4, nv4=vax4;

	static float ndm1 = dmod1;
	static float ndm2 = dmod1;
	static float ndm3 = dmod1;
	static float ndm4 = dmod1;

	//generate new arbitrary rotation vector
	if ((t % maxtime) < rSpeed) {
		rv1 = glm::normalize (glm::vec3 (genVal(), genVal(), genVal()));
		rv2 = glm::normalize (glm::vec3 (genVal(), genVal(), genVal()));
		rv3 = genNonColinear(rv1);
		rv4 = genNonColinear(rv2);
		
		ndm1  = genNVal();
		ndm2  = genNVal();
		ndm3  = genNVal();
		ndm4  = genNVal();
	}

	float nt = t % maxtime;

	//interpolate new current vectors
	if (nt < 400) {
			glm::vec3 dv1  = 0.0025f * (rv1  - vax1 );
			glm::vec3 dv2  = 0.0025f * (rv2  - vax2 );
			glm::vec3 dv3  = 0.0025f * (rv3  - vax3 );
			glm::vec3 dv4  = 0.0025f * (rv4  - vax4 );

			
			nv1 = vax1 + nt * dv1;
			nv2 = vax2 + nt * dv2;
			nv3 = vax3 + nt * dv3;
			nv4 = vax4 + nt * dv4;
	}
	if (nt >= 400 && fabs(nt - 400) < rSpeed) {
		vax1  = nv1;
		vax2  = nv2;
		vax3  = nv3;
		vax4  = nv4;
	}
		
		
	glUniform3fv(ax1un, 1, &nv1[0]);
	glUniform3fv(ax2un, 1, &nv2[0]);
	glUniform3fv(ax3un, 1, &nv3[0]);
	glUniform3fv(ax4un, 1, &nv4[0]);

	glUniform1i(t1un, t1 );
	glUniform1i(t2un, t2 );	
	glUniform1i(t3un, t3 );
	glUniform1i(t4un, t4 );

	glUniform1f(dmod1un, dmod1);
	glUniform1f(dmod2un, dmod2);	
	glUniform1f(dmod3un, dmod3);
	glUniform1f(dmod4un, dmod4);
}

static void redraw(void) {
	static float t=0;
	int a,b;
	unsigned int currentVer;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	if (wireframe)
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	
	glShadeModel(GL_SMOOTH);

	glUseProgram(theProgram);
	
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cubeNormalBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	setUniforms(t);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBO);
	glDrawElements(GL_TRIANGLES, sCube.ids.size(), GL_UNSIGNED_INT, NULL);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glUseProgram(0);
	
	glutSwapBuffers();
	glutPostRedisplay();

	t+=rSpeed;
}

void print_usage() {
	cout << "Usage: cube-slime [-w] [-p fpartition_number] [-s speed] [-l latency] -h\n"
		"\t-w: use wireframe mode (default: off)\n"
		"\t-p: cube face fpartition count (default: 20)\n"
		"\t-s: rotation speed (default: 2)\n"
		"\t-l: vertices inertion (default: 5)\n"
		"\t-h: print this help message and exit\n";
}

void init(int argc, char **argv) {
	srand(time(NULL));
	int rez;
	while ((rez = getopt(argc,argv,"hwp:s:l:")) != -1){
		switch (rez){
		case 'w':
			wireframe = true;
			break;
			
		case 'p':
			fpartition = atoi(optarg);
			if (fpartition < 0) {
				cout << "bad partition number" << endl;
				exit(EXIT_FAILURE);
			}
			break;
			
		case 's':
			rSpeed = atoi(optarg);
			if (rSpeed < 0) {
				cout << "bad speed" << endl;
				exit(EXIT_FAILURE);
			}
			break;
			
		case 'l':
			DISPLACE_PER_UNIT = atof(optarg);
			if (DISPLACE_PER_UNIT < 0) {
				cout << "bad latency" << endl;
				exit(EXIT_FAILURE);
			}
			break;

		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
		case '?': print_usage(); exit(EXIT_FAILURE);
		};
	};
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:             // ESCAPE key
	case 'q':	  
		exit (0);
		break;
	}
}

void reshape (int w, int h)
{
	perspectiveMatrix[0] = fFrustumScale / (w / (float)h);
	perspectiveMatrix[5] = fFrustumScale;

	glUseProgram(theProgram);
	glUniformMatrix4fv(perspectiveMatrixUn, 1, GL_FALSE, perspectiveMatrix);
	glUseProgram(0);
	
	glViewport(0, 0, (GLsizei) w, (GLsizei) w);
}


int main(int argc, char **argv) 
{
	init (argc, argv);

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Vector slime demo");
	glutKeyboardFunc (Keyboard);
	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);

	glEnable(GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		cerr << "Error: %s\n" << glewGetErrorString(err) << endl;
		exit(EXIT_FAILURE);
	}
   
	glewInit();
	initCube();
	
	glutMainLoop();

	return 0; 
}
