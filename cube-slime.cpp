#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

typedef glm::mat3 mat3;
typedef glm::vec3 vec3;
const float pi = 3.14159265 ; // For portability across platforms

using namespace std;

void initCube(void);
static void redraw(void);


struct Cube {
	vector<vec3> verts;
} sCube;

int partition = 20;

vec3 force_center(0,-10,0);
const float DISPLACE_PER_UNIT = 8.9;

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

void generatePolyCubeVerts(vec3 from, vec3 to, int face_partition, Cube &c) {
	float sx = (to.x - from.x) / face_partition;
	float sy = (to.y - from.y) / face_partition;
	float sz = (to.z - from.z) / face_partition;
	

	//1 (front)

	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
		
			c.verts.push_back( vec3(from.x + px * sx,     from.y + py * sy,     from.z));
			c.verts.push_back( vec3(from.x + px * sx,     from.y + (py+1) * sy, from.z));			
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + (py+1) * sy, from.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + py * sy,     from.z));
		}


	//4 (back)
	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(from.x + px * sx,     from.y + py * sy,     to.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + py * sy,     to.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + (py+1) * sy, to.z));
			c.verts.push_back( vec3(from.x + px * sx,     from.y + (py+1) * sy, to.z));


		}

	//2 (right)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(to.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( vec3(to.x, from.y + (py+1) * sy, from.z + pz * sz));
			c.verts.push_back( vec3(to.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(to.x, from.y + py     * sy, from.z + (pz+1) * sz));
		}


	// 6 (left)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(from.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( vec3(from.x, from.y + py     * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x, from.y + (py+1) * sy, from.z + pz * sz));			
	
		}

	// 5 (top)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( vec3(from.x + px*sx,     to.y, from.z + pz * sz));
			c.verts.push_back( vec3(from.x + px*sx,     to.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, to.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, to.y, from.z + pz * sz));
		}
	return;
	//3 (bottom)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( vec3(from.x + px*sx,     from.y, from.z + pz * sz));
			c.verts.push_back( vec3(from.x + px*sx,     from.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, from.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, from.y, from.z + pz * sz));
		}
}

void initCube(void)
{
	generatePolyCubeVerts(vec3(-10,-10,-10), vec3(10,10,10), partition, sCube);
}


float calcDisplace(const vec3 &v) {
	float dist = glm::gtx::norm::l2Norm(v, force_center);

	// cout << "disp = " << dist*DISPLACE_PER_UNIT  << endl;;
	// return 0;
	return DISPLACE_PER_UNIT*dist;
}
struct time_frame {
	int from;
	int to;
	float dt;
};

time_frame timeline[] = {
	{0,   100, 1},
	{100, 140, 0.2},
	{140, 240, 1},
	{240, 260, 0.1},
	{260, 360, 1},
};


float rTime(int t) {
	float dt;
	int tl = sizeof(timeline) / sizeof(time_frame);
	
	dt = 1;
	for (int i = 0; i < tl; i++) {
		if (t > timeline[i].from && t < timeline[i].to) {
			dt = timeline[i].dt;
			break;
		}
	}
	// if (dt == 5)
	// cout << t+dt << endl;
	return t*dt;
}


vec3 rotFunc1(const vec3 &v, vec3 axis, int t) {
	vec3 rv;

	float nt = t - calcDisplace(v);
	
	if (nt < 0.01) {
		return v;
	}

	if (nt > 360) {
		return v;
	}

	nt = rTime(nt);
	rv = glm::rotate(v, nt, axis);

	return rv;
}


int maxtime = 1000;
float rSpeed = 2;
float rAccel = 0.2;
float yRot = 0;
float rUpper = 10;
float rLower = 3;

static void redraw(void)
{
	static float t=0;
	int a,b;
	unsigned int currentVer;

	if (t > maxtime)
		t = 0;
	t+=rSpeed;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glPushMatrix();
	
	glTranslatef(0,0,-50);
	// glRotatef(rotateBy,0,1,0.6);

	glBegin(GL_QUADS);
		
	for (int i = 0; i < sCube.verts.size(); i++) {
		vec3 cv = sCube.verts[i];
		cv = rotFunc1(cv, vec3(0,1,0.2), t);
		cv = rotFunc1(cv, vec3(0,0.2,1), t-270);
		cv = rotFunc1(cv, vec3(1,1,1), t-490);
		
		
		float v[3] = {cv.x, cv.y, cv.z};
		float col[3] = {(i%255)/255.0,(i%255)/255.0,(i%255)/255.0};

		glColor3fv(col);
		glVertex3fv(v);

		// sCube.verts[i] = cv;
	}

	// rSpeed += rAccel;


	// if (rSpeed > rUpper)
	// 	rAccel =- rAccel;
	// if (rSpeed < rLower)
	// 	rAccel =- rAccel;
		
	glEnd();
	glPopMatrix();
	
	glutSwapBuffers();
	glutPostRedisplay();
}


int main(int argc, char **argv) 
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Vector slime demo");

	glutDisplayFunc(redraw);	

	glMatrixMode(GL_PROJECTION);						//hello
	gluPerspective(45, //view angle
		       1.0,	//aspect ratio
		       10.0, //near clip
		       10000.0);//far clip
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_CULL_FACE);

	initCube();

	glutMainLoop();

	return 0; 
}
