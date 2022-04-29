#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>
#include <iostream>
#include <string>
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
		angle = .5;
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

bool leftClick, rightClick, spaceClick, plusClick, minusClick;
int rayCount;
Player *player;

void drawPlayer(Player *player)
{
	glColor3f(0.6, 0.1, 1);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(player->x, player->y);
	glEnd();


}

int mapX = 8, mapY = 8, mapScale = 64;
int tileMap[] =
{
	2,2,2,2,2,2,2,2,
	2,0,0,0,0,0,0,2,
	2,0,0,0,0,0,0,2,
	2,0,0,0,0,0,0,2,
	2,0,0,0,0,0,0,2,
	2,0,0,0,0,0,0,2,
	2,0,0,0,0,0,0,2,
	2,2,2,2,2,2,2,2,
};

void drawMap() // draw a visual map from tile map
{
	int x, y, xOffset, yOffset, currentPos;

	for (y = 0; y < mapY; y++) // Iterate through Each x,y value
	{
		for (x = 0; x < mapX; x++)
		{
			currentPos = tileMap[y*mapX + x];
			if (currentPos == 1) // if tile value is 1, make tile display as white
			{
				glColor3f(1, 1, 1);
			}
			else if (currentPos == 2) {
				glColor3f(.8, .8, .8);
			}
			else			  // otherwise, display as black
			{
				glColor3f(0, 0, 0);
			}
			xOffset = x * mapScale;
			yOffset = y * mapScale;
			glBegin(GL_QUADS); // draw tile box
			glVertex2i(xOffset, yOffset);
			glVertex2i(xOffset, yOffset + mapScale);
			glVertex2i(xOffset + mapScale, yOffset + mapScale);
			glVertex2i(xOffset + mapScale, yOffset);
			glEnd();
		}
	}
}

float dist(float ax, float ay, float bx, float by, float angle) // Calculate Length of the Hypotenuse via pythagorean's theorem
{
	return (sqrt((bx - ax)*(bx - ax) + (by - ay)*(by - ay)));
}

float returnUnitAngle(float angle)
{
	if (angle < 0) // if ray angle decreases less than zero, set back to 2pi
	{
		angle += 2 * PI;
	}
	if (angle > 2 * PI) // if ray angle increases over 2pi, set back to 0
	{
		angle -= 2 * PI;
	}
	return angle;
}

Ray *getHorizontalRay(float rayAngleH, Player *player) {	// Check Horizontal map lines
	int depthOfField=0, rayTileX, rayTileY, mapPos;
	float rayX, rayY, xOffset, yOffset, disH = 1000, horizontalX = player->x, horizontalY = player->y;
	float aTan = -1 / tan(rayAngleH);
	if (rayAngleH > PI) // if the ray is facing down
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) - 0.0001;
		rayX = (player->y - rayY)*aTan + player->x;
		yOffset = -64;
		xOffset = -yOffset * aTan;
	}
	if (rayAngleH < PI) // if the ray is facing upwards
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) + 64;
		rayX = (player->y - rayY)*aTan + player->x;
		yOffset = 64;
		xOffset = -yOffset * aTan;
	}
	if (rayAngleH == 0 || rayAngleH == PI) // if ray is facing directly left or right
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = 8;
	}
	while (depthOfField < 8)
	{
		rayTileX = divideBy64((int)(rayX));
		rayTileY = divideBy64((int)(rayY));
		mapPos = rayTileY * mapX + rayTileX;
		if (mapPos > 0 && mapPos < mapX*mapY && tileMap[mapPos] > 0) // if position is in array bounds, and value of tile at position is not 0; Has hit a horizontal wall
		{
			horizontalX = rayX;
			horizontalY = rayY;
			disH = dist(player->x, player->y, horizontalX, horizontalY, rayAngleH); //distance between the ray's end and the player
			depthOfField = 8; // exit condition
		}
		else { rayX += xOffset; rayY += yOffset; depthOfField += 1; } // If not a Wall, Check next iteration
	}
	Ray *hMap = new Ray(disH, horizontalX, horizontalY);
	return hMap;
}

Ray *getVerticalRay(float rayAngleV, Player *player) {
	// Check Vertical map lines
	int depthOfField = 0, rayTileX, rayTileY, mapPos;
	float rayX, rayY, xOffset, yOffset, disV = 1000, vx = player->x, vy = player->y;
	float negTan = -tan(rayAngleV);
	if (rayAngleV > PIOVER2 && rayAngleV < THREEPIOVER2) // if ray is facing left
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) - 0.0001;
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = -64;
		yOffset = -xOffset * negTan;
	}
	if (rayAngleV < PIOVER2 || rayAngleV > THREEPIOVER2)  // if ray is facing right
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) + 64;
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = 64;
		yOffset = -xOffset * negTan;
	}
	if (rayAngleV == 0 || rayAngleV == PI) // if ray is facing directly left or right
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = 8;
	}
	while (depthOfField < 8) //iterator
	{
		rayTileX = divideBy64((int)(rayX));
		rayTileY = divideBy64((int)(rayY));
		mapPos = rayTileY * mapX + rayTileX;
		if (mapPos > 0 && mapPos < mapX*mapY && tileMap[mapPos] > 0) // if position is in array bounds, and value of tile at position is not 0; Has hit a vertical wall
		{
			vx = rayX;
			vy = rayY;
			disV = dist(player->x, player->y, vx, vy, rayAngleV); // distance between the ray's end and the player
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
	if (tileMap[(divideBy64((int)player->y)) * mapX + (divideBy64((int)player->x))] > 0) {
		return;
	}

	rayAngle = player->angle - RADIANDEGREE * rayCount; // set the initial ray angle to be the player's angle offset by raycount degrees (in radians)
	rayAngle = returnUnitAngle(rayAngle);
	for (r = 0; r < rayCount; r++) // iterate for number of rays `r<{raycount}`
	{
		Ray *horizontalRay = getHorizontalRay(rayAngle, player);
		Ray *verticalRay = getVerticalRay(rayAngle, player);
		Ray *finalRay = NULL;

		if (verticalRay->length < horizontalRay->length) // get whichever ray is shorter, and draw that ray
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
		
		glLineWidth(1);
		glBegin(GL_LINES);
		if (tileMap[(divideBy64((int)finalRay->y)) * mapX + (divideBy64((int)finalRay->x))] == 2) // if ray hits wall, color ray white
		{
			glColor3f(1, 1, 1);
		}
		else { // else color ray green (indicates has hit tile)
			glColor3f(0.196, 0.886, 0.282);
		}
		glVertex2i(player->x, player->y);
		glVertex2i(finalRay->x, finalRay->y);
		glEnd();

		rayAngle += RADIANDEGREE * 2;
		rayAngle = returnUnitAngle(rayAngle);
	}
}

void mouseActionHandler(int button, int state, int x, int y)
{
	int mouseMap;
	if (button == GLUT_LEFT_BUTTON)
	{
		leftClick = (state == GLUT_DOWN);
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		rightClick = (state == GLUT_DOWN);
		if (rightClick) {
			mouseMap = (divideBy64((int)y)) * mapX + (divideBy64((int)x)); // pos in tile map
			if (mouseMap > 0 && mouseMap < mapX*mapY && tileMap[mouseMap] == 0)
			{
				tileMap[mouseMap] = 1;
			}
			else if (mouseMap > 0 && mouseMap < mapX*mapY && tileMap[mouseMap] == 1)
			{
				tileMap[mouseMap] = 0;
			}
		}

	}
	else if (button == 3 && state == GLUT_DOWN)
	{
		player->angle -= 0.2;
		if (player->angle < 0)
		{
			player->angle += 2 * PI;
		}
	}
	else if (button == 4 && state == GLUT_DOWN)
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

void drawTextFromString(float x, float y, string string) 
{
	glColor3f(0, 0.8, 0.6);
	glRasterPos2f(x, y);
	for (int i = 0; i < string.size(); ++i) // Loop through String 
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]); 
	}
}

void drawReactiveText(float x, float y, string string, bool state) {
	if (state) {glColor3f(0.196, 0.886, 0.282);}
	else { glColor3f(0.933, 0.102, 0.102); }
	glRasterPos2f(x, y);
	for (int i = 0; i < string.size(); ++i)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void callText() 
{
	drawTextFromString(540, 30, "Hold Click to Move Player");
	drawTextFromString(540, 80, "Scroll to Turn the ray");
	drawTextFromString(540, 130, "Press +/- to change number of rays");
	drawTextFromString(540, 180, "Right Click to place/remove a tile");
	drawTextFromString(540, 230, "Press `Space` to toggle between 1 and 360 rays");
	drawTextFromString(540, 330, "Number of Rays: " + to_string(rayCount));
	drawTextFromString(540, 380, "Player's Angle: " + to_string((player->angle) / RADIANDEGREE));
	drawReactiveText(900, 330, "[LMB]", leftClick);
	drawReactiveText(900, 380, "[RMB]", rightClick);

}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap();
	callText();
	drawRays(player);
	drawPlayer(player);
	glutSwapBuffers();
}

void buttons(unsigned char key, int x, int y)
{
	if (key == '=') {
		rayCount += 1; minusClick = true;
	} // increment Ray Count
	if (key == '-') {
		rayCount -= 1; minusClick = true;
	} // decrement Ray Count
	if (key == ' ') // Spacebar case - toggle b/w 1 & 360 rays
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
	if (rayCount == 0) { // Link 1 ray and 360 rays
		rayCount = 360;
	}
	else if (rayCount > 360) {
		rayCount = 1;
	}
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
	glutMouseFunc(mouseActionHandler);
	glutMotionFunc(motion);
	glutMainLoop();
}