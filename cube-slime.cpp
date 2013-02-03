#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <vector>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
typedef glm::mat3 mat3;
typedef glm::vec3 vec3;
const float pi = 3.14159265 ; // For portability across platforms

using namespace std;

void initCube(void);
static void redraw(void);


struct Cube {
	vector<vec3> verts;
} sCube;

int partition = 50;

vec3 force_center(0,0,0);

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

mat3 rotate(const float degrees, const vec3& axis) 
{
    	double phi = degrees/180 * pi;
	vec3 u = glm::normalize(axis);

	return mat3 (
		cos(phi) + u.x * u.x * (1 - cos(phi)),
		u.x * u.y * (1 - cos(phi)) - u.z * sin(phi),
		u.x * u.z * (1 - cos(phi)) + u.y * sin(phi),

		u.y * u.x * (1 - cos(phi)) + u.z * sin(phi),
		cos(phi) + u.y * u.y * (1 - cos(phi)),
		u.y * u.z * (1 - cos(phi)) - u.x * sin(phi),
		
		u.z * u.x * (1 - cos(phi)) - u.y * sin(phi),
		u.z * u.y * (1 - cos(phi)) + u.x * sin(phi),
		cos(phi) + u.z * u.z * (1 - cos(phi)));
}



static void redraw(void)
{
	static float rotateBy=0;
	int a,b;
	unsigned int currentVer;

	rotateBy+=1;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glPushMatrix();
	
	glTranslatef(0,0,-50);
	glRotatef(rotateBy,0,1,0.6);

	glBegin(GL_QUADS);
		
	for (int i = 0; i < sCube.verts.size(); i++) {
		vec3 cv = sCube.verts[i];
		float v[3] = {cv.x, cv.y, cv.z};
		float col[3] = {(i%255)/255.0,(i%255)/255.0,(i%255)/255.0};			
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
