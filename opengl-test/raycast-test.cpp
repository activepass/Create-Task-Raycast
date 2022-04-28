#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#include <array>
#define PI 3.1415926535
#define PIOVER2 PI/2
#define THREEPIOVER2 3*PI/2
#define RADIANDEGREE 0.0174533
#define divideBy64(x) (x >> 6)
#define multiplyBy64(x) (x << 6)

using namespace std;

class Player {
public:
	float x;
	float y;
	float angle;
	Player(float px, float py, float pangle) {
		x = px;
		y = py;
		angle = pangle;
	}
	Player() {
		x = 300;
		y = 300;
		angle = 0;
	}
};
class Ray {
public:
	float x;
	float y;
	float length;
	Ray(float plength, float px, float py) {
		x = px;
		y = py;
		length = plength;
	}
	Ray(Ray *pRay) {
		x = pRay->x;
		y = pRay->y;
		length = pRay->length;
	}
};

bool leftClick,rightClick;
int rayCount;
Player *player;

void drawPlayer(Player *player)
{
	// Draw Player
	glColor3f(0.388, 0, 1);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(player->x, player->y);
	glEnd();


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

float returnUnitAngle(float angle)
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

Ray *getHorizontalRay(float rayAngleH, Player *player) {
	// Check Horizontal map lines
	int depthOfField, mpX, mpY, mapPos;
	float rayX, rayY, xOffset, yOffset, disH = 1000000, hx = player->x, hy = player->y;
	depthOfField = 0;
	float aTan = -1 / tan(rayAngleH);
	if (rayAngleH > PI) //looking up
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) - 0.0001;
		rayX = (player->y - rayY)*aTan + player->x;
		yOffset = -64;
		xOffset = -yOffset * aTan;
	}
	if (rayAngleH < PI) //looking down
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) + 64;
		rayX = (player->y - rayY)*aTan + player->x;
		yOffset = 64;
		xOffset = -yOffset * aTan;
	}
	if (rayAngleH == 0 || rayAngleH == PI)
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = 8;
	}
	while (depthOfField < 8)
	{
		mpX = divideBy64((int)(rayX));
		mpY = divideBy64((int)(rayY));
		mapPos = mpY * mapX + mpX;
		if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) // if position is in array size, and value of tile at position is 1; Has hit a horizontal wall
		{
			hx = rayX;
			hy = rayY;
			disH = dist(player->x, player->y, hx, hy, rayAngleH); //distance between the ray's end and the player
			depthOfField = 8; // exit condition
		}
		else { rayX += xOffset; rayY += yOffset; depthOfField += 1; } //If not Wall, Check next
	}
	Ray *hMap = new Ray( disH, hx, hy );
	return hMap;
}

Ray *getVerticalRay(float rayAngleV, Player *player) {
	// Check Vertical map lines
	int depthOfField=0, mpX, mpY, mapPos;
	float rayX, rayY, xOffset, yOffset, disV = 1000000, vx = player->x, vy = player->y;
	float negTan = -tan(rayAngleV);
	if (rayAngleV > PIOVER2 && rayAngleV < THREEPIOVER2) // looking left
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) - 0.0001;
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = -64;
		yOffset = -xOffset * negTan;
	}
	if (rayAngleV < PIOVER2 || rayAngleV > THREEPIOVER2)  // looking right 
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) + 64;
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = 64;
		yOffset = -xOffset * negTan;
	}
	if (rayAngleV == 0 || rayAngleV == PI) // looking direct left or right
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = 8;
	}
	while (depthOfField < 8) //iterator
	{
		mpX = divideBy64((int)(rayX));
		mpY = divideBy64((int)(rayY));
		mapPos = mpY * mapX + mpX;
		if (mapPos > 0 && mapPos < mapX*mapY && map[mapPos] == 1) // if position is in array size, and value of tile at position is 1; Has hit a horizontal wall
		{
			vx = rayX;
			vy = rayY;
			disV = dist(player->x, player->y, vx, vy, rayAngleV); //distance between the ray's end and the player
			depthOfField = 8; // exit condition
		}
		else  //If not Wall, Check next
		{
			rayX += xOffset;
			rayY += yOffset;
			depthOfField += 1;
		}
	}
	Ray *vMap = new Ray(disV, vx, vy);
	return vMap;
}

void drawRays(Player *player)
{
	
	int r; 
	float rayAngle;
	if (map[(divideBy64((int)player->y)) * mapX + (divideBy64((int)player->x))] == 1) {
		return;
	}

	rayAngle = player->angle - RADIANDEGREE * rayCount; // set the initial ray angle to be the player's angle offset by raycount degrees (in radians)
	rayAngle = returnUnitAngle(rayAngle);
	for (r = 0; r < rayCount; r++) // iterate for number of rays `r<{raycount}`
	{
		Ray *horizontalRay = getHorizontalRay(rayAngle, player);

		Ray *verticalRay = getVerticalRay(rayAngle, player);
		Ray *finalRay = NULL;
		if (verticalRay->length < horizontalRay->length) // get whichever distance is shorter, and draw that line
		{ 
			finalRay = new Ray(verticalRay);
		}
		else if (verticalRay->length > horizontalRay->length)
		{
			finalRay = new Ray(horizontalRay);
		}
		else {
			return;
		}
		
		// Draw ray
		glColor3f(0,0.8,0.6);
		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex2i(player->x,player->y);
		glVertex2i(finalRay->x,finalRay->y);
		glEnd();
		
		rayAngle += RADIANDEGREE*2;
		rayAngle = returnUnitAngle(rayAngle);
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
		mouseMap = (divideBy64((int)y)) * mapX + (divideBy64((int)x)); // pos in tile map
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
		player->angle -= 0.2;
		if (player->angle < 0)
		{
			player->angle += 2 * PI;
		}
	}
	if (button == 4 && state == GLUT_DOWN)
	{
		player->angle += 0.2;
		if (player->angle > 2 * PI)
		{ 
			player->angle -= 2 * PI;
		} 
	}
	glutPostRedisplay();
}

void motion(int x, int y)
{
	if (leftClick)
	{
		player->x = x;
		player->y = y;
	}
	glutPostRedisplay();
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap();
	drawRays(player);
	drawPlayer(player);
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
	rayCount = 1;
	player = new Player();
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

// Classes



