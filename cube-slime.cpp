#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <vector>
#include <math.h>
#include <unistd.h>
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
	vector<vec3> cols;
} sCube;

int partition = 20;

vec3 force_center(0,-10,0);
const float DISPLACE_PER_UNIT = 6.9;

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
	
	int cx = 127.0 / face_partition;

	//1 (front)

	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(from.x + px * sx,     from.y + py * sy,     from.z));
			c.verts.push_back( vec3(from.x + px * sx,     from.y + (py+1) * sy, from.z));			
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + (py+1) * sy, from.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + py * sy,     from.z));

			c.cols.push_back( vec3( (cx*px % 255) / 255.0, (cx*py % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*px % 255) / 255.0, ((py+1) % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*(px+1) % 255) / 255.0, (cx*(py+1) % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*(px+1) % 255) / 255.0, (cx*py % 255) / 255, 0));
		}

	//4 (back)
	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(from.x + px * sx,     from.y + py * sy,     to.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + py * sy,     to.z));
			c.verts.push_back( vec3(from.x + (px+1) * sx, from.y + (py+1) * sy, to.z));
			c.verts.push_back( vec3(from.x + px * sx,     from.y + (py+1) * sy, to.z));

			c.cols.push_back( vec3( (cx*px % 255) / 255.0, (cx*py % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*px % 255) / 255.0, (cx*(py+1) % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*(px+1) % 255) / 255.0, (cx*(py+1) % 255) / 255, 0));
			c.cols.push_back( vec3( (cx*(px+1) % 255) / 255.0, (cx*py % 255) / 255, 0));
		}

	//2 (right)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(to.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( vec3(to.x, from.y + (py+1) * sy, from.z + pz * sz));
			c.verts.push_back( vec3(to.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(to.x, from.y + py     * sy, from.z + (pz+1) * sz));

			c.cols.push_back( vec3(0,  (cx*py%255)/255.0     , (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3(0,  (cx*(py+1)%255) / 255.0 , (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3(0,  (cx*(py+1)%255) / 255.0 , (cx*(pz+1)%255)/255.0));
			c.cols.push_back( vec3(0,  (cx*py%255)/255.0     , (cx*(pz+1) % 255) / 255.0));
		}


	// 6 (left)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( vec3(from.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( vec3(from.x, from.y + py     * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x, from.y + (py+1) * sy, from.z + pz * sz));			


			c.cols.push_back( vec3(0,  (cx*py%255)/255.0     , (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3(0,  (cx*(py+1)%255) / 255.0 , (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3(0,  (cx*(py+1)%255) / 255.0 , (cx*(pz+1)%255)/255.0));
			c.cols.push_back( vec3(0,  (cx*py%255)/255.0     , (cx*(pz+1) % 255) / 255.0));
		}

	// 5 (top)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( vec3(from.x + px*sx,     to.y, from.z + pz * sz));
			c.verts.push_back( vec3(from.x + px*sx,     to.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, to.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, to.y, from.z + pz * sz));


			c.cols.push_back( vec3((cx*px%255)/255.0     , 0, cx*(pz%255)/255.0 ));
			c.cols.push_back( vec3((cx*(px+1)%255) / 255.0 , 0, cx*(pz%255)/255.0 ));
			c.cols.push_back( vec3((cx*(px+1)%255) / 255.0 , 0,(cx*(pz+1)%255)/255.0));
			c.cols.push_back( vec3((cx*px%255)/255.0     , 0,(cx*(pz+1) % 255) / 255.0));
		}

	//3 (bottom)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( vec3(from.x + px*sx,     from.y, from.z + pz * sz));
			c.verts.push_back( vec3(from.x + px*sx,     from.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, from.y, from.z + (pz+1) * sz));
			c.verts.push_back( vec3(from.x + (px+1)*sx, from.y, from.z + pz * sz));
			
			c.cols.push_back( vec3((cx*px%255)/255.0     , 0, (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3((cx*(px+1)%255) / 255.0 , 0, (cx*pz%255)/255.0 ));
			c.cols.push_back( vec3((cx*(px+1)%255) / 255.0 , 0,(cx*(pz+1)%255)/255.0));
			c.cols.push_back( vec3((cx*px%255)/255.0     , 0,(cx*(pz+1) % 255) / 255.0));
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
  float d1;
  float d2;
};

/* time_frame timeline[] = { */
/*   {0,   100, 0,   1}, */
/*   {100, 140, 1,   0.2}, */
/*   {140, 240, 0.2, 1}, */
/*   {240, 260, 1,   0.1}, */
/*   {260, 360, 0.1, 1}, */
/* }; */

time_frame timeline[] = {
  {0,   100, 0,   1},
  {100, 140, 1,   0.2},
  {140, 240, 0.2, 1},
  {240, 260, 1,   0.1},
  {260, 360, 0.1, 1},
};



/* float rTime(int t) { */
/* 	float dt; */
/* 	int di = -1; */
/* 	int tl = sizeof(timeline) / sizeof(time_frame); */
	
/* 	dt = 1; */
/* 	for (int i = 0; i < tl; i++) { */
/* 		if (t >= timeline[i].from && t <= timeline[i].to) { */
/* 			di = i; */
/* 			break; */
/* 		} */
/* 	} */

/* 	if (di == -1) { */
/* 		/\* cout << "out, t = " << t << endl; *\/ */
/* 		return t; */
/* 	} */

/* 	float len = timeline[di].to - timeline[di].from; */
/* 	float step = (timeline[di].d2 - timeline[di].d1) / len; */
/* 	float diff = timeline[di].d1 + (t - timeline[di].from) * step; */

/* 	/\* if (diff < 0) *\/ */
/* 	/\* 	cout << "diff = " << diff << endl; *\/ */
/* 	diff = 1; */
	
/* 	return t*diff; */
/* } */


vec3 rotFunc1(const vec3 &v, vec3 axis, int t, bool debug = false) {
	vec3 rv;
	/* debug = false; */
	float nt = t - calcDisplace(v);

	debug = false;
	if (debug) {
		cout << "-------------" << endl;
		cout << "orig t: " << t << endl;
		cout << "t': " << nt << endl;
	}
	
	if (nt < 0.01) {
		return v;
	}

	if (nt > 360) {
		return v;
	}

	/* nt = rTime(nt); */

	if (debug) {
		cout << "nt: " << nt << endl;
		cout << "----------\n" << nt << endl;
	}
	
	/* cout << nt << endl; */
	
	rv = glm::rotate(v, nt, axis);

	return rv;
}



int maxtime = 1080;
float rSpeed = 4;

static void redraw(void)
{
	static float t=50;
	int a,b;
	unsigned int currentVer;

	if (t > maxtime)
		t = 50;
	
	t+=rSpeed;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); */
	glShadeModel(GL_SMOOTH);

	glPushMatrix();
	
	glTranslatef(0,0,-50);
	// glRotatef(rotateBy,0,1,0.6);

	glBegin(GL_QUADS);

	for (int i = 0; i < sCube.verts.size(); i++) {
		vec3 cv = sCube.verts[i];
		vec3 cc = sCube.cols[i];
		cv = rotFunc1(cv, vec3(0,-1,-0.2), t);
		cv = rotFunc1(cv, vec3(0,-0.5,-0.4), t-350);
		cv = rotFunc1(cv, vec3(0.4,-0.8,0.0), t-620);

		float v[3] = {cv.x, cv.y, cv.z};
		float col[3] = {cc.x, cc.y, cc.z};
		
		glColor3fv(col);
		glVertex3fv(v);
	}
	
	glEnd();
	glPopMatrix();
	
	glutSwapBuffers();
	glutPostRedisplay();
}


int main(int argc, char **argv) 
{
	srand(time(NULL));
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

	// glEnable(GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);

	initCube();

	glutMainLoop();

	return 0; 
}
