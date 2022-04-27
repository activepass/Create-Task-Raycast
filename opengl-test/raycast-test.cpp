#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533


float playerX,playerY,playerdx,playerdy,playerAngle,mouseX,mouseY;
bool leftClick,rightClick,spaced;

void drawPlayer()
{
	// Draw Player
	glColor3f(1, 1, 1);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(playerX, playerY);
	glEnd();

	// Draw Direction Line
	//glLineWidth(3);
	//glBegin(GL_LINES);
	//glVertex2i(playerX, playerY);
	//glVertex2i(playerX + playerdx * 5, playerY + playerdy * 5);
	//glEnd();
}

int mapX = 8, mapY = 8, mapS = 64;
int map[] =
{
	1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,
};

void drawMap()
{
	int x, y, xo, yo;
	for (y = 0; y < mapY; y++)
	{
		for (x = 0; x < mapX; x++)
		{
			if (map[y*mapX + x] == 1) { glColor3f(1, 1, 1); }
			else { glColor3f(0, 0, 0); }
			xo = x * mapS; yo = y * mapS;
			glBegin(GL_QUADS);
			glVertex2i(xo+1, yo+1);
			glVertex2i(xo+1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+1);
			glEnd();
		}
	}
}

float dist(float ax, float ay, float bx, float by, float angle)
{
	return ( sqrt((bx - ax)*(bx - ax) + (by - ay)*(by - ay)) );
}

void drawRays()
{
	
	int r, mpX, mpY, mapPos, depthOfField; float rayX, rayY, rayAngle, xOffset, yOffset;
	rayAngle = playerAngle - DR * 360; if (rayAngle < 0) { rayAngle += 2 * PI; } if (rayAngle > 2*PI) { rayAngle -= 2 * PI; }
	for (r = 0; r < 360; r++)
	{
		// Check Horizontal map lines
		depthOfField = 0;
		float disH = 1000000, hx = playerX, hy = playerY;
		float aTan = -1 / tan(rayAngle);
		if (rayAngle > PI) { rayY = (((int)playerY >> 6) << 6) - 0.0001; rayX = (playerY - rayY)*aTan + playerX; yOffset = -64; xOffset = -yOffset * aTan; } //looking up
		if (rayAngle < PI) { rayY = (((int)playerY >> 6) << 6)+64; rayX = (playerY - rayY)*aTan + playerX; yOffset = 64; xOffset = -yOffset * aTan; } // looking down
		if (rayAngle == 0 || rayAngle == PI) { rayX = playerX; rayY = playerY; depthOfField = 8; }
		while (depthOfField < 8)
		{
			mpX = (int)(rayX) >> 6; mpY = (int)(rayY) >> 6; mapPos = mpY * mapX + mpX;
			if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) { hx = rayX; hy = rayY; disH = dist(playerX, playerY, hx, hy, rayAngle); depthOfField = 8; } // Hits Wall
			else { rayX += xOffset; rayY += yOffset; depthOfField += 1; } //If not Wall, Check next
		}

		// Check Vertical map lines
		depthOfField = 0;
		float disV = 1000000, vx = playerX, vy = playerY;
		float negTan = -tan(rayAngle);
		if (rayAngle > P2 && rayAngle < P3) { rayX = (((int)playerX >> 6) << 6) - 0.0001; rayY = (playerX - rayX)*negTan + playerY; xOffset = -64; yOffset = -xOffset * negTan; } // looking left
		if (rayAngle < P2 || rayAngle > P3) { rayX = (((int)playerX >> 6) << 6) + 64; rayY = (playerX - rayX)*negTan + playerY; xOffset = 64; yOffset = -xOffset * negTan; } // looking right
		if (rayAngle == 0 || rayAngle == PI) { rayX = playerX; rayY = playerY; depthOfField = 8; }
		while (depthOfField < 8)
		{
			mpX = (int)(rayX) >> 6; mpY = (int)(rayY) >> 6; mapPos = mpY * mapX + mpX;
			if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) { vx = rayX; vy = rayY; disV = dist(playerX, playerY, vx, vy, rayAngle); depthOfField = 8; } // Hits Wall
			else { rayX += xOffset; rayY += yOffset; depthOfField += 1; } //If not Wall, Check next
		}
		if (disV < disH) { rayX = vx; rayY = vy; }
		if (disH < disV) { rayX = hx; rayY = hy; }
		glColor3f(1, 1, 0);
		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex2i(playerX, playerY);
		glVertex2i(rayX, rayY);
		glEnd();
		rayAngle += DR; if (rayAngle < 0) { rayAngle += 2 * PI; } if (rayAngle > 2 * PI) { rayAngle -= 2 * PI; }
	}
}

void mouse(int button, int state, int x, int y)
{
	int mouseMap;
	if (button == GLUT_LEFT_BUTTON)
	{
		leftClick = (state == GLUT_DOWN);
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		mouseMap = ((int)y>>6) * mapX + ((int)x>>6);
		if (mouseMap > 0 && mouseMap < mapX*mapY && map[mouseMap] == 0) {
			map[mouseMap] = 1;
		}
		else if (mouseMap > 0 && mouseMap < mapX*mapY && map[mouseMap] == 1) {
			map[mouseMap] = 0;
		}
		
	}
}

void motion(int x, int y)
{
	if (leftClick)
	{
		playerX = x;
		playerY = y;
	}
	glutPostRedisplay();
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap();
	drawRays();
	drawPlayer();
	glutSwapBuffers();
}

void buttons(unsigned char key, int x, int y)
{
	if (key == 'a') { playerAngle -= 0.1; if (playerAngle < 0) { playerAngle += 2 * PI; } playerdx = cos(playerAngle)*5; playerdy = sin(playerAngle)*5; }
	if (key == 'd') { playerAngle += 0.1; if (playerAngle > 2*PI) { playerAngle -= 2 * PI; } playerdx = cos(playerAngle)*5; playerdy = sin(playerAngle)*5; }
	if (key == 'w') { playerX += playerdx; playerY += playerdy; }
	if (key == 's') { playerX -= playerdx; playerY -= playerdy; }
	glutPostRedisplay();
}



void init() 
{
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);
	playerX = 300; playerY = 300; playerdx = cos(playerAngle) * 5; playerdy = sin(playerAngle) * 5;
}

int main(int argc, char* argv[]) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1024, 512);
	glutCreateWindow("Raycast");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
}