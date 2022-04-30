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


// CLASSES

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
		angle = THREEPIOVER2;
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


// GLOBALS

bool leftClick, rightClick, spaceClick;
int rayCount;
Player *player;


// TILE MAP

int mapX = 8, mapY = 8, mapScale = 64;
// Tiles stored as 0 are empty
// Tiles stored as 1 are walls, editable by the user
// Tiles stored as 2 are map boundaries and can not be changed by the player

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
			if (currentPos == 1) { glColor3f(1, 1, 1); } // User tiles are White
			else if (currentPos == 2) {glColor3f(.8, .8, .8);} // Map Wall tiles are grey
			else {glColor3f(0, 0, 0);} // empty tiles are black

			xOffset = x * mapScale;
			yOffset = y * mapScale;

			// draw tile box
			glBegin(GL_QUADS);
			glVertex2i(xOffset, yOffset);
			glVertex2i(xOffset, yOffset + mapScale);
			glVertex2i(xOffset + mapScale, yOffset + mapScale);
			glVertex2i(xOffset + mapScale, yOffset);
			glEnd();
		}
	}
}


// CALCULATION FUNCTIONS

float dist(float ax, float ay, float bx, float by, float angle) // Calculate Length of the Hypotenuse via pythagorean's theorem
{
	return (sqrt((bx - ax)*(bx - ax) + (by - ay)*(by - ay)));
}

float returnUnitAngle(float angle)
{
	if (angle < 0) { angle += 2 * PI; }  // if ray angle decreases less than zero, set back to 2pi
	else if (angle > 2 * PI) { angle -= 2 * PI; } // if ray angle increases over 2pi, set back to 0
	return angle;
}


// RAY CALCULATIONS

Ray *getHorizontalRay(float rayAngle, Player *player) {		// Check Horizontal map lines
	int depthOfField=0, rayTileX, rayTileY, mapPos;
	float rayX, rayY, xOffset, yOffset, rayLength = 1000, newX = player->x, newY = player->y;
	float normalTan = -1 / tan(rayAngle);
	if (rayAngle > PI) // if the ray is facing upwards
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) - 0.0001; // accuracy
		rayX = (player->y - rayY)*normalTan + player->x;
		yOffset = -mapScale;
		xOffset = -yOffset * normalTan;
	}
	else if (rayAngle < PI) // if the ray is facing downwards
	{
		rayY = multiplyBy64((int)(divideBy64((int)player->y))) + mapScale;
		rayX = (player->y - rayY)*normalTan + player->x;
		yOffset = mapScale;
		xOffset = -yOffset * normalTan;
	}
	else if (rayAngle == 0 || rayAngle == PI) // if ray is facing directly left or right, dont draw
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = mapX;
	}
	while (depthOfField < mapX)
	{
		rayTileX = divideBy64((int)(rayX));
		rayTileY = divideBy64((int)(rayY));
		mapPos = rayTileY * mapX + rayTileX;
		if (mapPos > 0 && mapPos < mapX*mapY && tileMap[mapPos] > 0) // if position is in array bounds, and value of tile at position is not 0; Has hit a horizontal wall
		{
			newX = rayX;
			newY = rayY;
			rayLength = dist(player->x, player->y, newX, newY, rayAngle); //distance between the ray's end and the player
			depthOfField = mapX; // exit condition
		}
		else // If not a Wall, Check next iteration
		{
			rayX += xOffset;
			rayY += yOffset;
			depthOfField += 1;
		} 
	}
	Ray *hMap = new Ray(rayLength, newX, newY);
	return hMap;
}

Ray *getVerticalRay(float rayAngle, Player *player) {	  // Check Vertical map lines
	int depthOfField = 0, rayTileX, rayTileY, mapPos;
	float rayX, rayY, xOffset, yOffset, rayLength = 1000, newX = player->x, newY = player->y;
	float negTan = -tan(rayAngle);
	if (rayAngle > PIOVER2 && rayAngle < THREEPIOVER2) // if ray is facing left
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) - 0.0001; // accuracy
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = -mapScale;
		yOffset = -xOffset * negTan;
	}
	else if (rayAngle < PIOVER2 || rayAngle > THREEPIOVER2)  // if ray is facing right
	{
		rayX = multiplyBy64((int)(divideBy64((int)player->x))) + mapScale;
		rayY = (player->x - rayX)*negTan + player->y;
		xOffset = mapScale;
		yOffset = -xOffset * negTan;
	}
	else if (rayAngle == 0 || rayAngle == PI) // if ray is facing directly left or right, dont draw
	{
		rayX = player->x;
		rayY = player->y;
		depthOfField = mapX;
	}
	while (depthOfField < mapX) //iterator
	{
		rayTileX = divideBy64((int)(rayX));
		rayTileY = divideBy64((int)(rayY));
		mapPos = rayTileY * mapX + rayTileX; // From x,y coordinates convert to number in tileMap Array
		if (mapPos > 0 && mapPos < mapX*mapY && tileMap[mapPos] > 0) // if position is in array bounds, and value of tile at position is not 0; Has hit a vertical wall
		{
			newX = rayX;
			newY = rayY;
			rayLength = dist(player->x, player->y, newX, newY, rayAngle); // distance between the ray's end and the player
			depthOfField = mapX; // exit condition
		}
		else  //If not Wall, Check next
		{
			rayX += xOffset;
			rayY += yOffset;
			depthOfField += 1;
		}
	}
	Ray *vMap = new Ray(rayLength, newX, newY);
	return vMap;
}

void drawRays(Player *player) {
	int rayDrawer;
	float rayAngle;

	if (tileMap[(divideBy64((int)player->y)) * mapX + (divideBy64((int)player->x))] > 0) { return; } // if the player is in a tile that is not empty, do not draw rays

	rayAngle = player->angle - RADIANDEGREE * (rayCount-1); // set the initial ray angle to be the player's angle offset by raycount degrees (in radians)
	rayAngle = returnUnitAngle(rayAngle);

	for (rayDrawer = 0; rayDrawer < rayCount; rayDrawer++) // iterate through all rays, raycount
	{
		Ray *horizontalRay = getHorizontalRay(rayAngle, player);
		Ray *verticalRay = getVerticalRay(rayAngle, player);
		Ray *finalRay = NULL;

		// get whichever ray is shorter create new `Ray`
		if (verticalRay->length < horizontalRay->length) 
		{
			finalRay = new Ray(verticalRay); // vertical ray is shorter
		}
		else if (verticalRay->length > horizontalRay->length)
		{
			finalRay = new Ray(horizontalRay); // horizontal ray is shorter
		}
		else { return; } // if rays are the same length, dont draw

		// Draw ray
		
		glLineWidth(1);
		glBegin(GL_LINES);
		if (tileMap[(divideBy64((int)finalRay->y)) * mapX + (divideBy64((int)finalRay->x))] == 2) { glColor3f(1, 1, 1); } // if ray hits a map boundary, make the ray white
		else { glColor3f(0.196, 0.886, 0.282); } // if ray hits a user-placed tile, draw that ray green
		glVertex2i(player->x, player->y);
		glVertex2i(finalRay->x, finalRay->y);
		glEnd();

		if (rayCount > 1) {rayAngle += RADIANDEGREE; rayAngle = returnUnitAngle(rayAngle);} // if drawing more than one ray, draw the next ray one radian over
	}
}


// WINDOW EVENT HANDLERS

void buttons(unsigned char key, int x, int y)
{
	if (key == '=') { rayCount += 1; } // increment Ray Count
	if (key == '-') { rayCount -= 1; } // decrement Ray Count
	if (key == ' ') {  // Spacebar case - toggle b/w 1 & 360 rays
		if (rayCount == 1) { rayCount = 360; }
		else { rayCount = 1; }
	}
	// Link 1 ray and 360 rays
	if (rayCount == 0) { rayCount = 360; }
	else if (rayCount > 360) {rayCount = 1;}

	glutPostRedisplay();
}

void mouseActionHandler(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON){ leftClick = (state == GLUT_DOWN); } // left clicked, see mouseMotion()
	else if (button == GLUT_RIGHT_BUTTON)
	{
		rightClick = (state == GLUT_DOWN);
		if (rightClick) {
			int mousePosInMap = (divideBy64((int)y)) * mapX + (divideBy64((int)x)); // pos in tile map
			if (mousePosInMap > 0 && mousePosInMap < mapX*mapY && tileMap[mousePosInMap] == 0) { tileMap[mousePosInMap] = 1; } // if tile is empty, place tile
			else if (mousePosInMap > 0 && mousePosInMap < mapX*mapY && tileMap[mousePosInMap] == 1) { tileMap[mousePosInMap] = 0; } // if tile is full, remove tile
		}

	}
	else if (button == 3 && state == GLUT_DOWN) // mouse wheel up
	{
		player->angle -= 0.2;
		if (player->angle < 0) { player->angle += 2 * PI; }
	}
	else if (button == 4 && state == GLUT_DOWN) // mouse wheel down
	{
		player->angle += 0.2;
		if (player->angle > 2 * PI) { player->angle -= 2 * PI; }
	}

	glutPostRedisplay();
}

void mouseMotion(int x, int y) // used to move the player smoothly when holding down left click
{
	if (leftClick)
	{
		player->x = x;
		player->y = y;
	}

	glutPostRedisplay();
}


// DRAWING

void drawTextFromString(float x, float y, string string) 
{
	glColor3f(0, 0.8, 0.6);
	glRasterPos2f(x, y);
	for (int i = 0; i < string.size(); ++i) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]); }  // draw text to screen by character
}

void drawReactiveText(float x, float y, string string, bool state) {
	if (state) { glColor3f(0.196, 0.886, 0.282); } // draw green
	else { glColor3f(0.933, 0.102, 0.102); } // draw red
	glRasterPos2f(x, y);
	for (int i = 0; i < string.size(); ++i) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]); }  // draw text to screen by character
}

void callText() // Information text that appears on the right side
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

void drawPlayer(Player *player)
{
	glColor3f(0.6, 0.1, 1);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(player->x, player->y);
	glEnd();
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



// INITIAL CONDITIONS

void init()
{
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);
	rayCount = 1;
	player = new Player();
}


// MAIN

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
	glutMotionFunc(mouseMotion);
	glutMainLoop();
}