#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#define PI 3.1415926535
#define PIOVER2 PI/2
#define THREEPIOVER2 3*PI/2
#define RADIANDEGREE 0.0174533

using namespace std;


float playerX,playerY,playerdx,playerdy,playerAngle,mouseX,mouseY;
bool leftClick,rightClick,spaced;
int rayCount;

void drawPlayer()
{
	// Draw Player
	glColor3f(0.388, 0, 1);
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

void drawMap() // draw a visual map from tile map
{
	int x, y, xo, yo;
	
	for (y = 0; y < mapY; y++) // Iterate through Each x,y value
	{
		for (x = 0; x < mapX; x++)
		{
			if (map[y*mapX + x] == 1) // if tile value is 1, make tile display as white
			{
				glColor3f(1, 1, 1); 
			}
			else			  // otherwise, display as black
			{ 
				glColor3f(0, 0, 0);
			}
			xo = x * mapS;
			yo = y * mapS;
			glBegin(GL_QUADS); // draw tile box
			glVertex2i(xo+1, yo+1); // bottom left vertex
			glVertex2i(xo+1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+1);
			glEnd();
		}
	}
}

float dist(float ax, float ay, float bx, float by, float angle) // Calculate Length of the Hypotenuse via pythagorean's theorem
{
	return ( sqrt((bx - ax)*(bx - ax) + (by - ay)*(by - ay)) );
}

float checkAng(float angle)
{
	if (angle < 0) // if ray angle decreases less than zero, set back to 2pi
	{ 
		angle += 2 * PI; 
	} 
	if (angle > 2*PI) // if ray angle increases over 2pi, set back to 0
	{ 
		angle -= 2 * PI; 
	}
	return angle;
}

void drawRays()
{
	
	int r, mpX, mpY, mapPos, depthOfField; 
	float rayX, rayY, rayAngle, xOffset, yOffset;

	rayAngle = playerAngle - RADIANDEGREE * rayCount; // set the initial ray angle to be the player's angle offset by 60 degrees (in radians)
	rayAngle = checkAng(rayAngle);
	for (r = 0; r < rayCount; r++) // iterate for number of rays `r<{raycount}`
	{
		// Check Horizontal map lines
		depthOfField = 0;
		float disH = 1000000, hx = playerX, hy = playerY;
		float aTan = -1 / tan(rayAngle);
		if (rayAngle > PI) //looking up
		{ 
			rayY = (((int)playerY >> 6) << 6) - 0.0001; 
			rayX = (playerY - rayY)*aTan + playerX; 
			yOffset = -64; 
			xOffset = -yOffset * aTan; 
		} 
		if (rayAngle < PI) //looking down
		{ 
			rayY = (((int)playerY >> 6) << 6)+64;
			rayX = (playerY - rayY)*aTan + playerX;
			yOffset = 64;
			xOffset = -yOffset * aTan;
		}
		if (rayAngle == 0 || rayAngle == PI) 
		{
			rayX = playerX;
			rayY = playerY;
			depthOfField = 8; 
		}
		while (depthOfField < 8)
		{
			mpX = (int)(rayX) >> 6;
			mpY = (int)(rayY) >> 6;
			mapPos = mpY * mapX + mpX;
			if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) // if position is in array size, and value of tile at position is 1; Has hit a horizontal wall
			{ 
				hx = rayX;
				hy = rayY;
				disH = dist(playerX, playerY, hx, hy, rayAngle); //distance between the ray's end and the player
				depthOfField = 8; // exit condition
			} 
			else { rayX += xOffset; rayY += yOffset; depthOfField += 1; } //If not Wall, Check next
		}

		// Check Vertical map lines
		depthOfField = 0;
		float disV = 1000000, vx = playerX, vy = playerY;
		float negTan = -tan(rayAngle);
		if (rayAngle > PIOVER2 && rayAngle < THREEPIOVER2) // looking left
		{
			rayX = (((int)playerX >> 6) << 6) - 0.0001;
			rayY = (playerX - rayX)*negTan + playerY;
			xOffset = -64;
			yOffset = -xOffset * negTan; 
		}
		if (rayAngle < PIOVER2 || rayAngle > THREEPIOVER2)  // looking right 
		{
			rayX = (((int)playerX >> 6) << 6) + 64;
			rayY = (playerX - rayX)*negTan + playerY;
			xOffset = 64;
			yOffset = -xOffset * negTan;
		}
		if (rayAngle == 0 || rayAngle == PI) // looking direct left or right
		{
			rayX = playerX;
			rayY = playerY;
			depthOfField = 8;
		}
		while (depthOfField < 8) //iterator
		{
			mpX = (int)(rayX) >> 6;
			mpY = (int)(rayY) >> 6;
			mapPos = mpY * mapX + mpX;
			if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) // if position is in array size, and value of tile at position is 1; Has hit a horizontal wall
			{
				vx = rayX;
				vy = rayY;
				disV = dist(playerX, playerY, vx, vy, rayAngle); //distance between the ray's end and the player
				depthOfField = 8; // exit condition
			}
			else  //If not Wall, Check next
			{ 
				rayX += xOffset;
				rayY += yOffset;
				depthOfField += 1;
			}
		}
		if (disV < disH) // get whichever distance is shorter, and draw that line
		{ 
			rayX = vx;
			rayY = vy;
		}
		else if (disH < disV)
		{
			rayX = hx;
			rayY = hy;
		}
		
		// Draw ray
		glColor3f(0,0.8,0.6);
		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex2i(playerX, playerY);
		glVertex2i(rayX, rayY);
		glEnd();
		
		rayAngle += RADIANDEGREE*2;
		rayAngle = checkAng(rayAngle);
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
		mouseMap = ((int)y>>6) * mapX + ((int)x>>6); // pos in tile map
		if (mouseMap > 0 && mouseMap < mapX*mapY && map[mouseMap] == 0) 
		{
			map[mouseMap] = 1;
		}
		else if (mouseMap > 0 && mouseMap < mapX*mapY && map[mouseMap] == 1) 
		{
			map[mouseMap] = 0;
		}
		
	}
	if (button == 3 && state == GLUT_DOWN)
	{
		playerAngle -= 0.2;
		if (playerAngle < 0) 
		{
			playerAngle += 2 * PI; 
		}
		//playerdx = cos(playerAngle) * 5;
		//playerdy = sin(playerAngle) * 5;
	}
	if (button == 4 && state == GLUT_DOWN)
	{
		playerAngle += 0.2;
		if (playerAngle > 2 * PI) 
		{ 
			playerAngle -= 2 * PI;
		} 
		//playerdx = cos(playerAngle) * 5;
		//playerdy = sin(playerAngle) * 5;
	}
	glutPostRedisplay();
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
	// old Key movement
	//if (key == 'a') { playerAngle -= 0.1; if (playerAngle < 0) { playerAngle += 2 * PI; } playerdx = cos(playerAngle)*5; playerdy = sin(playerAngle)*5; }
	//if (key == 'd') { playerAngle += 0.1; if (playerAngle > 2*PI) { playerAngle -= 2 * PI; } playerdx = cos(playerAngle)*5; playerdy = sin(playerAngle)*5; }
	//if (key == 'w') { playerX += playerdx; playerY += playerdy; }
	//if (key == 's') { playerX -= playerdx; playerY -= playerdy; }
	if (key == '=') { rayCount += 1;}
	if (key == '-') { rayCount -= 1;}
	if (key == ' ') 
	{
		if (rayCount == 1) 
		{
			rayCount = 360;
		}
		else 
		{
			rayCount = 1;
		}
	}
	if (rayCount == 0) {
		rayCount = 360;
	}
	else if (rayCount > 360) {
		rayCount = 1;
	}
	cout << "rays: " << rayCount << "\n";
	glutPostRedisplay();
}



void init() 
{
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);
	playerX = 300; 
	playerY = 300; 
	//playerdx = cos(playerAngle) * 5; 
	//playerdy = sin(playerAngle) * 5;
	rayCount = 1;
}

int main(int argc, char* argv[]) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1024, 512);
	glutCreateWindow("AP CSP Create -- Raycast");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();
}
