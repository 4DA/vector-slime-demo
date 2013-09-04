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

#include <glsw.h>
using namespace std;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

void my_msleep (int msecs) {
#ifdef _WIN32
	Sleep(msecs);
#else
	usleep (msecs * 1000);
#endif
	return;
}
//light stuff
float light_direction[4] = {0, 0.5, 0.7, 0.0};
//----------

typedef glm::mat3 mat3;
typedef glm::vec3 vec3;
const float pi = 3.14159265 ; // For portability across platforms
GLuint cubeVertBuffer;
GLuint cubeNormalBuffer;
GLuint cubeVAO;
GLuint shadowVolVAO;
int cubeVertexNum;
GLuint slimeTransformPr;
GLuint shadowVolProgram;

GLuint cubeIndexBO;
GLuint cubeAdjIndexBO;

GLuint tfvbo;
GLuint query_generated;
GLuint query_written;

void initCube(void);
static void redraw(void);
GLuint initShader(GLenum eShaderType, const char *);
GLuint CreateProgram(GLuint, const char *, const char *, const char *);

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
	vector<int> ids_adj;
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
	int &fp = face_fpartition;

	float sx = (to.x - from.x) / fp;
	float sy = (to.y - from.y) / fp;
	float sz = (to.z - from.z) / fp;

	cout << from.z << endl;
	cout << to.z << endl;
	
	//indices normally follow CCW-ordering
	for (int oft, quad = 0; quad < fp * fp * 6 ; quad++) {
		oft = quad * 4;

		// cout << "fp^2 = " << fp * fp << endl;
		// cout << "quad no. " << quad << endl;
		
		if (quad / (fp * fp) % 2) {
			c.ids.push_back (oft);
			c.ids.push_back (oft+1);
			c.ids.push_back (oft+2);
				
			c.ids.push_back (oft);
			c.ids.push_back (oft+3);
			c.ids.push_back (oft+1);
			
		}
		else {
			c.ids.push_back (oft);
			c.ids.push_back (oft+2);
			c.ids.push_back (oft+1);
				
			c.ids.push_back (oft);
			c.ids.push_back (oft+1);
			c.ids.push_back (oft+3);

		}
	}

// Triangle adjacency information
	
//	  2    	      3	       	    4
//	  O-----------O------------O
//	    \-        |\-	   |
//	      \-      |	 \--	   |
//	        \-    |	    \-	   |
//	          \-  |	      \--  |
//	   	    \-| 1	 \-|
//	   	      O------------O 5
//	   	        \-	   |
//	   		  \-	   |
//	       	       	    \- 	   |
//			      \-   |
//			        \- |
//			       	  \0 6
//	

	//indices for adjacency TODO: fix corner cases
	for (int oft, quad = 0; quad < fp * fp * 6 ; quad++) {
		oft = quad * 4;

  		c.ids_adj.push_back (oft);      
  		c.ids_adj.push_back (oft+2);   
  		c.ids_adj.push_back (oft+1);     
  		c.ids_adj.push_back (oft+5);   
  		c.ids_adj.push_back (oft+3);   
  		c.ids_adj.push_back (oft-fp*4);

  		c.ids_adj.push_back (oft+2);   
  		c.ids_adj.push_back (oft-1);   
  		c.ids_adj.push_back (oft);     
  		c.ids_adj.push_back (oft+3);   
  		c.ids_adj.push_back (oft+1);   
  		c.ids_adj.push_back (oft+fp*4+1);
	}

	cout << "c.ids_adj size = " << c.ids_adj.size() << endl;

	//1 (front)
	for (int px = 0; px < fp; px++)
		for (int py = 0; py < fp; py++) {
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + (py+1) * sy); c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + py * sy);     c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + py * sy);     c.verts.push_back(from.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z); c.verts.push_back(1.0);
		}

	
	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(-1.0); c.normals.push_back(0.0);
	}

	//4 (back)
	for (int px = 0; px < fp; px++)
		for (int py = 0; py < fp; py++) {
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + (py+1) * sy); c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + py * sy);     c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px * sx); c.verts.push_back(     from.y + py * sy);     c.verts.push_back(to.z); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1) * sx); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(to.z); c.verts.push_back(1.0);
		}
	
	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(1.0); c.normals.push_back(0.0);
	}

	// 6 (left)
	for (int pz = 0; pz < fp; pz++)
		for (int py = 0; py < fp; py++) {
			c.verts.push_back( from.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + (pz+1)* sz);  c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(-1.0); c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}

 	// 2 (right)
	for (int pz = 0; pz < fp; pz++)
		for (int py = 0; py < fp; py++) {
			c.verts.push_back( to.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + py     * sy); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( to.x); c.verts.push_back( from.y + (py+1) * sy); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(1.0); c.normals.push_back(0.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}


	// 5 (top)
	for (int pz = 0; pz < fp; pz++)
		for (int px = 0; px < fp; px++) {
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     to.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( to.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     to.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( to.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(1.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}

	//3 (bottom)
	for (int pz = 0; pz < fp; pz++)
		for (int px = 0; px < fp; px++) {
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     from.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( from.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + px*sx); c.verts.push_back(     from.y); c.verts.push_back(from.z + pz * sz); c.verts.push_back(1.0);
			c.verts.push_back( from.x + (px+1)*sx); c.verts.push_back( from.y); c.verts.push_back(from.z + (pz+1)* sz); c.verts.push_back(1.0);
		}

	for (int vid = 0; vid < fp * fp * 4; vid++) {
		c.normals.push_back(0.0); c.normals.push_back(-1.0); c.normals.push_back(0.0); c.normals.push_back(0.0);
	}
}

void checkGlErrors( void )
{
	GLenum e = glGetError();
	while ( e != GL_NO_ERROR )
	{
		printf("GL error: %s!\n", gluErrorString(e) );
		e = glGetError();
	}
}


void setupTransformFeedbackBuffer(void)
{
	const char *attr[] = {"gl_Position"};

	glGenBuffers( 1, &tfvbo );
	glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, tfvbo );

	glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, sCube.verts.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW );

	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfvbo );

	glTransformFeedbackVaryings( slimeTransformPr, 1, attr, GL_INTERLEAVED_ATTRIBS );
	checkGlErrors();		

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

	glGenBuffers(1, &cubeAdjIndexBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeAdjIndexBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sCube.ids_adj.size() * sizeof(int), &sCube.ids_adj[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void initCube(void)
{
	generatePolyCubeVerts(vec3(10,10,10), vec3(-10,-10,-10),  fpartition, sCube);
	initVertexBuffer();

	cout << "Going to render " << sCube.verts.size() / 3 << " polygons" << endl;

	glGenVertexArrays(1, &cubeVAO);


	slimeTransformPr = glCreateProgram();
	setupTransformFeedbackBuffer();
	slimeTransformPr = CreateProgram(slimeTransformPr, "cube.Vertex.transform", "cube.Geometry.stream_out", "cube.Fragment");
	basicOffsetUn = glGetUniformLocation(slimeTransformPr, "basic_offset");
	perspectiveMatrixUn = glGetUniformLocation(slimeTransformPr, "perspectiveMatrix");

	tUniform = glGetUniformLocation(slimeTransformPr, "T");
	magnitudeUniform = glGetUniformLocation(slimeTransformPr, "magnitude");
	fcenterUniform = glGetUniformLocation(slimeTransformPr, "force_center");
	// axisUniform = glGetUniformLocation(slimeTransformPr, "axis");
	lightDirUniform = glGetUniformLocation(slimeTransformPr, "light_direction");
	lposUniform = glGetUniformLocation(slimeTransformPr, "l_pos");
	lightIntensityUniform = glGetUniformLocation(slimeTransformPr, "light_intensity");

	ax1un = glGetUniformLocation(slimeTransformPr, "ax1");
	ax2un = glGetUniformLocation(slimeTransformPr, "ax2");
	ax3un = glGetUniformLocation(slimeTransformPr, "ax3");
	ax4un = glGetUniformLocation(slimeTransformPr, "ax4");

	t1un = glGetUniformLocation(slimeTransformPr, "t1");
	t2un = glGetUniformLocation(slimeTransformPr, "t2");
	t3un = glGetUniformLocation(slimeTransformPr, "t3");
	t4un = glGetUniformLocation(slimeTransformPr, "t4");

	dmod1un = glGetUniformLocation(slimeTransformPr, "dmod1");
	dmod2un = glGetUniformLocation(slimeTransformPr, "dmod2");
	dmod3un = glGetUniformLocation(slimeTransformPr, "dmod3");
	dmod4un = glGetUniformLocation(slimeTransformPr, "dmod4");
	
	memset(perspectiveMatrix, 0, sizeof(float) * 16);
	perspectiveMatrix[0] = fFrustumScale;
	perspectiveMatrix[5] = fFrustumScale;
	perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
	perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
	perspectiveMatrix[11] = -1.0f;

	// setupTransformFeedbackBuffer();
	glGenQueries(1, &query_generated);
	glGenQueries(1, &query_written);
	

	glGenVertexArrays(1, &shadowVolVAO);

	shadowVolProgram = CreateProgram(
		glCreateProgram(),
		"cube.Vertex.passthrough",
		"cube.Geometry.shadow_volumes",
		"cube.Fragment");
}

GLuint CreateProgram(GLuint program, const char* vsKey, const char* gsKey, const char* fsKey)
{
    static int first = 1;

    GLuint shader_id;
    std::vector<GLuint> shaderList;
    
    if (first)
    {
        glswInit();
        glswAddPath("../", ".glsl");
        glswAddPath("./", ".glsl");
        glswAddDirective("*", "#version 330");

        first = 0;
    }

    const char *sources[3] = {glswGetShader(vsKey), glswGetShader(gsKey), glswGetShader(fsKey)};

    GLuint types[3] = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};

    for (int i = 0; i < 3; i++) {
	    if (sources[i] == NULL) {
		    cout << types[i] << "is not installed\n";
		    continue;
	    }
	    
	    shader_id = initShader(types[i], sources[i]);
	    glAttachShader(program, shader_id);
	    shaderList.push_back(shader_id);
    }

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

    for(size_t iLoop = 0; iLoop < shaderList.size(); iLoop++) {
	    glDetachShader(program, shaderList[iLoop]);
	    glDeleteShader(shaderList[iLoop]);
    }

    return program;
}

GLuint initShader(GLenum eShaderType, const char *source)
{
	GLuint shader = glCreateShader(eShaderType);
	glShaderSource(shader, 1, &source, NULL);
    
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

void setCubeTransformUniforms(int t) {
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


/*
  General transform feedback path:
  
glUseProgram( program );

glBeginQuery( GL_PRIMITIVES_GENERATED, … );
{
    glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, … );
    {
        glBeginTransformFeedback( type );
        {
            glEnable( GL_RASTERIZER_DISCARD );
            {
                //  Draw here
            }
        }
        glEndTransformFeedback( );
    }
    glEndQuery( GL_PRIMITIVES_GENERATED );
}
glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );

//  Retrieve our queries, enable RASTERIZER_DISCARD and render with our resulting TF buffer(s).
 */


void setCubeTransformData(float t) {
	glUseProgram(slimeTransformPr);
	
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cubeNormalBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBO);

	setCubeTransformUniforms(t);
}

void setShadowVolUniformsData() {
	glUseProgram(shadowVolProgram);

	glBindBuffer(GL_ARRAY_BUFFER, tfvbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeAdjIndexBO);
}

static void redraw(void) {
	static float t=0;
	int a,b;
	unsigned int currentVer;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.2f, 0.1f, 0.0f, 0.0f);

	if (wireframe)
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	
	glShadeModel(GL_SMOOTH);

	glBindVertexArray(cubeVAO);
	setCubeTransformData(t);

	// glBindBuffer( GL_ARRAY_BUFFER, tfvbo );
	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfvbo );
	
	// start transform feedback so that vertices get targetted to 'tfvbo'
	glBeginTransformFeedback( GL_TRIANGLES );
	checkGlErrors();
	

	glBeginQuery( GL_PRIMITIVES_GENERATED, query_generated );
	glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query_written );

	// cout << "going to render " << sCube.ids.size() << " polygons\n";
	glEnable( GL_RASTERIZER_DISCARD );
	
	glDrawElements(GL_TRIANGLES, sCube.ids.size(), GL_UNSIGNED_INT, NULL);
	glDisable( GL_RASTERIZER_DISCARD );

	glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );
	glEndQuery( GL_PRIMITIVES_GENERATED);

	GLuint primitives_written;
	GLuint primitives_generated;

	glGetQueryObjectuiv( query_generated, GL_QUERY_RESULT, &primitives_generated );
	printf("Primitives generated: %d !\n", primitives_generated );
	
	// read back query results
	do {
		glGetQueryObjectuiv( query_written, GL_QUERY_RESULT, &primitives_written );
		printf("Primitives written to TFB: %d !\n", primitives_written );
		// if ( primitives_written == 0 )
		// 	fprintf( stderr, "Primitives written to TFB: %d !\n", primitives_written );
  	
		// retrieve the data stored in the TFB
		checkGlErrors();
		my_msleep(1000);
	}
	while (primitives_written < primitives_generated);

	glEndTransformFeedback();

	glBindVertexArray(shadowVolVAO);
	setShadowVolUniformsData();

	// cout << "going to render " << primitives_written << " TF polygons\n";
	glDrawElements(GL_TRIANGLES_ADJACENCY, sCube.ids_adj.size(), GL_UNSIGNED_INT, NULL);
	
	// glBindBuffer( GL_ARRAY_BUFFER, tfvbo );
	
	// float * TFBdata = static_cast<float*>( glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY) );
	// if ( TFBdata == NULL ) {
	// 	cout << "TFBdata == NULL\n";
	// 	checkGlErrors();
	// }
	// else
	// {
	// 	fputs("TFB contents: ", stdout);
	// 	for ( int i = 0; i < 2*3*4; i ++ )
	// 		printf( "% 10f  ", TFBdata[i] );
	// 	putchar('\n');
	// }
	// bool success = glUnmapBuffer( GL_ARRAY_BUFFER );
	// if ( ! success ) {
	// 	cout << "glUnmapBuffer failed";
	// 	checkGlErrors();
	// 	cout << endl << endl;
	// }

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

	glUseProgram(slimeTransformPr);
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
