#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <vector>
using namespace std;

void initCube(void);
static void redraw(void);


struct Vertex {
	float x, y ,z;
	Vertex(int _x, int _y, int _z):x(_x), y(_y), z(_z){}
};

struct Cube {
	vector<Vertex> verts;
}sCube;

int partition = 100;

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

void generatePolyCubeVerts(Vertex from, Vertex to, int face_partition, Cube &c) {
	float sx = (to.x - from.x) / face_partition;
	float sy = (to.x - from.x) / face_partition;
	float sz = (to.x - from.x) / face_partition;
	

	//1 (front)

	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
		
			c.verts.push_back( Vertex(from.x + px * sx,     from.y + py * sy,     from.z));
			c.verts.push_back( Vertex(from.x + px * sx,     from.y + (py+1) * sy, from.z));			
			c.verts.push_back( Vertex(from.x + (px+1) * sx, from.y + (py+1) * sy, from.z));
			c.verts.push_back( Vertex(from.x + (px+1) * sx, from.y + py * sy,     from.z));
		}


	//4 (back)
	for (int px = 0; px < face_partition; px++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( Vertex(from.x + px * sx,     from.y + py * sy,     to.z));
			c.verts.push_back( Vertex(from.x + (px+1) * sx, from.y + py * sy,     to.z));
			c.verts.push_back( Vertex(from.x + (px+1) * sx, from.y + (py+1) * sy, to.z));
			c.verts.push_back( Vertex(from.x + px * sx,     from.y + (py+1) * sy, to.z));


		}

	//2 (right)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( Vertex(to.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( Vertex(to.x, from.y + (py+1) * sy, from.z + pz * sz));
			c.verts.push_back( Vertex(to.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(to.x, from.y + py     * sy, from.z + (pz+1) * sz));
		}


	// 6 (left)
	for (int pz = 0; pz < face_partition; pz++)
		for (int py = 0; py < face_partition; py++) {
			c.verts.push_back( Vertex(from.x, from.y + py     * sy, from.z + pz * sz));
			c.verts.push_back( Vertex(from.x, from.y + py     * sy, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x, from.y + (py+1) * sy, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x, from.y + (py+1) * sy, from.z + pz * sz));			
	
		}

	// 5 (top)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( Vertex(from.x + px*sx,     to.y, from.z + pz * sz));
			c.verts.push_back( Vertex(from.x + px*sx,     to.y, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x + (px+1)*sx, to.y, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x + (px+1)*sx, to.y, from.z + pz * sz));
		}
	return;
	//3 (bottom)
	for (int pz = 0; pz < face_partition; pz++)
		for (int px = 0; px < face_partition; px++) {
			c.verts.push_back( Vertex(from.x + px*sx,     from.y, from.z + pz * sz));
			c.verts.push_back( Vertex(from.x + px*sx,     from.y, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x + (px+1)*sx, from.y, from.z + (pz+1) * sz));
			c.verts.push_back( Vertex(from.x + (px+1)*sx, from.y, from.z + pz * sz));
		}
}

void initCube(void)
{
	generatePolyCubeVerts(Vertex(-10,-10,-10), Vertex(10,10,10), partition, sCube);
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
		
	float col[3] = {255,255,255};

	for (int i = 0; i < sCube.verts.size(); i++) {
		Vertex cv = sCube.verts[i];
		float v[3] = {cv.x, cv.y, cv.z};
			
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
	glutCreateWindow("Spinning cube");

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
