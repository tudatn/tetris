/*
* Testris.cpp
*
* Class Description: simplied version of Tetris.
* Ref: from Lab tutorial (Gasket)
* Last modified on: 21 Feb 2018
* Author: Tu Dat Nguyen
*/

#include "Angel.h"

// -----------------------------------------------------------------------------
// GLOBAL VARIABLES
static vec3  base_colors[] = {
	vec3(0.0, 0.0, 0.0),  	// Black
	vec3(1.0, 1.0, 1.0),  	// White
	vec3(1.0, 0.0, 0.0),  	// Red
	vec3(0.0, 1.0, 0.0),  	// Green
	vec3(1.0, 0.5, 0.0),  	// Orange
	vec3(1.0, 1.0, 0.0),  	// Yellow
	vec3(0.5, 0.0, 0.5),   	// Purple
	vec3(0.16, 0.16, 0.16)	// Sgiverydarkgray
};

// scale from (-1, 1) to (-5, 10): should have another way (ask TA for help)

vec2 scale = 0.95 * vec2(0.2, 0.1);

// 200 blocks x 2 triangles x 3 vertex
const int bNpoints = 1200;

// setup for boundary/ collision checking
int pixels[22][12];	// 200 pixels on the board and 64 pixels out-of-boundary

// Game
int gameOver = 0;
int pause = 0;

// grid lines: (11 vertical lines + 21 horizontal lines) x 2 vertex
const int gNpoints = 64;

// tiles: 4 blocks x 2 triangles x 3 vertex
const int tNpoints = 24;

// Location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// vertex array for the triangles, for the lines, and for the tiles respectively
GLuint board_vao, grid_vao, tile_vao;
GLuint grid_buffer, tile_buffer, board_buffer;

// board points and colors arrays
vec2 bpoints[bNpoints];
vec3 bcolors[bNpoints];
int onboardPoints = 0; // keep track of points on the board

// lines points and colors arrays
vec2 gpoints[gNpoints];
vec3 gcolors[gNpoints];

// tiles points and colors arrays
vec2 tpoints[tNpoints];
vec3 tcolors[tNpoints];

// tile properties
vec2 center, pivot;
int shape;

// PROTOTYPES
// ---------------------------------------------------------------------------
// Tiles
void makeBlock(int, vec2);
void rotateTile();
void makeTileO(vec2);
void makeTileI(vec2);
void makeTileS(vec2);
void makeTileZ(vec2);
void makeTileL(vec2);
void makeTileT(vec2);
void makeTile();
//----------------------------------------------------------------------------
// Game board
void makeBoard();
void updateBoard();
void redrawBoard();
//----------------------------------------------------------------------------
// Pixels
void makePixels();
void updatePixels();
int checkRowIsFilled();
void deleteFilledRow(int);
//----------------------------------------------------------------------------
// Game functions
void init();
void display();
int isSafe();
void up();
void donw();
void left();
void right();
void dropDelay(int);
void keyboardSpecial( int, int , int);
void keyboard(unsigned char, int, int);
void restart();
//----------------------------------------------------------------------------

// MAIN FUNCTION
int main( int argc, char **argv ) {
	srand (time(NULL));
	glutInit( &argc, argv );
	//We explicitly tell OpenGL that we are using the RGBA mode. When we start working with 3D resources there will be changes here.
	glutInitDisplayMode( GLUT_RGBA );

	//Change the size of the screen here. Keep in mind that the coordinates will also change accordingly
	//so, if you want to keep your points on the rendering canvas undisturbed you need a square screen.
	glutInitWindowSize( 400, 800 );

	// If you are using freeglut, the next two lines will check if
	// the code is truly 3.2. Otherwise, comment them out
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

	//Title of your screen, change accordingly
	glutCreateWindow( "Tetris" );

	// Iff you get a segmentation error at line 34, please uncomment the line below
	glewExperimental = GL_TRUE;
	glewInit();

	//this is the initialization function for your data.
	init();

	//Here we bind the function "display" to the OpenGL handler that will render your data
	glutDisplayFunc( display );

	//Similarly, we can also bind a function that will be executed 1000 miliseconds later. We bind this to the function rotateDelay.
	// We then bind this to the function "rotateDelay". The 0 in the end is the input of the rotateDelay class, it can only be an integer.
	glutTimerFunc(1000.0, dropDelay, 0);

	//Finally, we bind the keyboard events inside the class "keyboard" using the line below
	glutKeyboardFunc( keyboard );

	//If you really want to use F1 ~ F9 keys, or the arrow keys, then use the line of code below.
	glutSpecialFunc( keyboardSpecial );

	glutMainLoop();
	return 0;
}

// FUNCTION IMPLEMENTATION
// Tiles.cpp + Tiles.h --> to be refactored
// float tileBoundary[4];

// draw (calculate coordinates) of a block from the left bottom corner
void makeBlock(int i, vec2 origin) {
	tpoints[i] = origin; // first corner
	tpoints[i + 1] = origin + scale * vec2(1.0, 0.0);
	tpoints[i + 2] = origin + scale * vec2(0.0, 1.0);
	tpoints[i + 3] = tpoints[i + 1];
	tpoints[i + 4] = tpoints[i + 2];
	tpoints[i + 5] = origin + scale * vec2(1.0, 1.0); // opposite corner
}

// rotate a tile 90o
void rotateTile() {
	GLfloat angle = M_PI / 2;
	vec2 origin = pivot * vec2(1/scale.x, 1/scale.y);
	static mat2 rotatedegccw = mat2 ( cos(angle), sin(angle), -sin(angle), cos(angle) );
	for (int i = 0; i < 24; i++) {
		tpoints[i] = tpoints[i] * vec2(1/scale.x, 1/scale.y); // rotate with original scale
		tpoints[i] = rotatedegccw * (tpoints[i] - origin) + origin;
		tpoints[i] = tpoints[i] * scale;
	}
}

// O shape
void makeTileO(vec2 center) {
	makeBlock(0, center + scale * vec2(-1.0, -1.0));
	makeBlock(6, center + scale * vec2(-1.0, 0.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(0.0, -1.0));
}
// I shape
void makeTileI(vec2 center) {
	makeBlock(0, center + scale * vec2(-2.0, 0.0));
	makeBlock(6, center + scale * vec2(-1.0, 0.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(1.0, 0.0));
}
// S shape
void makeTileS(vec2 center) {
	makeBlock(0, center + scale * vec2(-1.0, -1.0));
	makeBlock(6, center + scale * vec2(0.0, -1.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(1.0, 0.0));
}
// Z shape
void makeTileZ(vec2 center) {
	makeBlock(0, center + scale * vec2(1.0, -1.0));
	makeBlock(6, center + scale * vec2(0.0, -1.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(-1.0, 0.0));
}
// L shape
void makeTileL(vec2 center) {
	makeBlock(0, center + scale * vec2(-1.0, -1.0));
	makeBlock(6, center + scale * vec2(-1.0, 0.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(1.0, 0.0));
}
// J shape
void makeTileJ(vec2 center) {
	makeBlock(0, center + scale * vec2(1.0, -1.0));
	makeBlock(6, center + scale * vec2(1.0, 0.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(-1.0, 0.0));
}
// T shape
void makeTileT(vec2 center) {
	makeBlock(0, center + scale * vec2(1.0, 0.0));
	makeBlock(6, center + scale * vec2(0.0, -1.0));
	makeBlock(12, center);
	makeBlock(18, center + scale * vec2(-1.0, 0.0));
}

// printing point coordinates: for debug
// void printCoordinate(vec2 p) {
// 	cout << p * vec2(1/scale.x, 1/scale.y) << endl;
// }

// draw (calculate coordinates) a random shape tile
void makeTile() {
	shape = rand() % 19 + 1;
	// ensure the tile is generated at the top
	if (shape == 1 || ((shape % 2) == 0 && shape != 10 && shape != 14 && shape != 18))
		center = scale * vec2(rand() % 7 - 3, 9.0);
	else
		center = scale * vec2(rand() % 7 - 3, 8.0);
	pivot = center + scale * vec2(0.5, 0.5);
	switch (shape) {
		case 1: // O
			pivot = center;
			makeTileO(center);
		break;
		case 2: // I h
			makeTileI(center);
		break;
		case 3: // I v
			makeTileI(center);
			rotateTile();
		break;
		case 4: // S h
			makeTileS(center);
		break;
		case 5: // S v
			makeTileS(center);
			rotateTile();
		break;
		case 6: // Z h
			makeTileZ(center);
		break;
		case 7: // Z v
			makeTileZ(center);
			rotateTile();
		break;
		case 8: // L 1
			makeTileL(center);
		break;
		case 9: // L 2
			makeTileL(center);
			rotateTile();
		break;
		case 10: // L 3
			makeTileL(center);
			rotateTile();
			rotateTile();
		break;
		case 11: // L 4
			makeTileL(center);
			rotateTile();
			rotateTile();
			rotateTile();
		break;
		case 12: // J 1
			makeTileJ(center);
		break;
		case 13: // J 2
			makeTileJ(center);
			rotateTile();
		break;
		case 14: // J 3
			makeTileJ(center);
			rotateTile();
			rotateTile();
		break;
		case 15: // J 4
			makeTileJ(center);
			rotateTile();
			rotateTile();
			rotateTile();
		break;
		case 16: // T 1
			makeTileT(center);
		break;
		case 17: // T 2
			makeTileT(center);
			rotateTile();
		break;
		case 18: // T 3
			makeTileT(center);
			rotateTile();
			rotateTile();
		break;
		case 19: // T 4
			makeTileT(center);
			rotateTile();
			rotateTile();
			rotateTile();
		break;
	}

	// colors
	int color = rand() % 7 + 1;
	for (int i = 0; i < 24; i++) {
		tcolors[i] = base_colors[color];
	}
	// check if no place for a new tile --> GAME terminates (req. d)
	if (isSafe() != 4){
		gameOver = 1;
		glutPostRedisplay();
		printf("GAME OVER!\n");
		exit(EXIT_SUCCESS);
	}
}

// initialize the game board
void makeBoard() {
	// generate all coordinates to {0, 0}
	for (int i = 0; i < 1200; i++) {
		bpoints[i] = vec2(0.0, 0.0);
		bcolors[i] = base_colors[0];
	}
	// Data for drawing board
	glGenVertexArrays( 1, &board_vao );
	glBindVertexArray( board_vao );

	// Create and initialize a buffer object
	glGenBuffers( 1, &board_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, board_buffer );

	glBufferData( GL_ARRAY_BUFFER, sizeof(bpoints) + sizeof(bcolors), bpoints, GL_STATIC_DRAW );

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(bpoints), bpoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(bpoints), sizeof(bcolors), bcolors );
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(bpoints)));
}

// add tiles to the board
void updateBoard() {
	for (int i = 0; i < 24; i++) {
		// copy points of tiles to the board
		bpoints[i + onboardPoints] = tpoints[i];
		bcolors[i + onboardPoints] = tcolors[i];
	}
	onboardPoints = onboardPoints + 24;
	// cout << "onboardPoints: " << onboardPoints << endl;
}

void redrawBoard() {
	glBindBuffer( GL_ARRAY_BUFFER, board_buffer );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(bpoints), bpoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(bpoints), sizeof(bcolors), bcolors );
}

//----------------------------------------------------------------------------
// all blocks on the board are pixels with 0
void makePixels() {
	// include the 4 out-of-boundary borders (pixels set to 1)
	// use for checking boundary and collision
	for (int j = 0; j < 12; j++) {
		pixels[0][j] = 1;
		pixels[21][j] = 1;
	}
	for (int i = 0; i < 22; i++) {
		pixels[i][0] = 1;
		pixels[i][11] = 1;
	}
	// all inside boundary pixels are set to 0
	for (int i = 1; i < 21; i++)
		for (int j = 1; j < 11; j++)
			pixels[i][j] = 0;
}

// void printPixels() {
// 	for (int i = 21; i >= 0; i--) {
// 		for (int j = 0; j < 12; j++) {
// 			cout << pixels[i][j] << " ";
// 		}
// 		cout << endl;
// 	}
// 	cout << "--------------------------------------------------";
// }

void updatePixels() {
	// reset pixels
	makePixels();
	for (int i = 0; i < onboardPoints; i = i + 6) {
		vec2 midPoint = (bpoints[i] + bpoints[i + 5]) * 1/2 * vec2(1/scale.x, 1/scale.y);
		int row = midPoint.y >= 0 ? (int (midPoint.y) + 11) : (int (midPoint.y - 1) + 11);
		int col = midPoint.x >= 0 ? (int (midPoint.x) + 6) : (int (midPoint.x - 1) + 6);
		pixels[row][col] = 1;
	}
	// printPixels();
}

// return the row of which all pixels are 1
int checkRowIsFilled() {
	bool isFilled = 1;
	for (int i = 1; i < 21; i++) {
		int j = 0;
		while (isFilled && j < 12) {
			if (pixels[i][j] == 0)
				isFilled = 0;
			else
				j++;
		}
		if (isFilled) {
			return i;
		}
		isFilled = 1; // reset flag
	}
	return 0;
}

// delete filled rows
void deleteFilledRow(int filledRow) {
	vec2 backupPoints[bNpoints];
	vec3 backupColors[bNpoints];
	int copyIndex = 0;
	for (int i = 0; i < onboardPoints; i = i + 6) {
		vec2 midPoint = (bpoints[i] + bpoints[i + 5]) * 1/2 * vec2(1/scale.x, 1/scale.y);
		int row = midPoint.y >= 0 ? (int (midPoint.y) + 11) : (int (midPoint.y - 1) + 11);
		// int col = midPoint.x >= 0 ? (int (midPoint.x) + 6) : (int (midPoint.x - 1) + 6);
		if (row != filledRow) {
			if (row > filledRow)
				for (int j = i; j < (i + 6); j++)
					bpoints[j] = bpoints[j] + scale * vec2(0.0, -1.0);
			for (int j = i; j < (i + 6); j++) {
				backupPoints[copyIndex] = bpoints[j];
				backupColors[copyIndex] = bcolors[j];
				copyIndex++;
			}
		}
	}
	// copy back to the board
	for (int i = 0; i < copyIndex; i++) {
		bpoints[i] = backupPoints[i];
		bcolors[i] = backupColors[i];
	}
	onboardPoints = copyIndex; // update points on the board
	updatePixels();
}

void deleteMultipleRow() {
	int row = checkRowIsFilled();
	while (row) {
		deleteFilledRow(row);
		row = checkRowIsFilled();
	}
}

// checking condition: if a pixel of future tiles is 1 --> not safe (0, 1, 2, 3)
int isSafe() {
	// 0: reach bottom
	// 1: reach left
	// 2: reach right
	// 3: collision
	// 4: safe
	for (int i = 0; i < 24; i = i + 6) {
		vec2 midPoint = (tpoints[i] + tpoints[i + 5]) * 1/2 * vec2(1/scale.x, 1/scale.y);
		int row = midPoint.y >= 0 ? (int (midPoint.y) + 11) : (int (midPoint.y - 1) + 11);
		int col = midPoint.x >= 0 ? (int (midPoint.x) + 6) : (int (midPoint.x - 1) + 6);
		if (pixels[row][col]) {
			if (row == 0)
				return 0;
			else if (col == 0)
				return 1;
			else if (col == 11)
				return 2;
			else
				return 3;
		}
	}
	return 4;
}

//----------------------------------------------------------------------------

void init( void ) {
	// load shaders
	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	vPosition = glGetAttribLocation( program, "vPosition" );
	vColor = glGetAttribLocation( program, "vColor" );

	// grid lines are initialized once --> hard code
	// vertical lines
	for (int i = 0; i < 11; i++) {
		gpoints[2 * i] = scale * vec2(-5.0 + i, -10.0);
		gpoints[2 * i + 1] = scale * vec2(-5.0 + i, 10.0);
	}
	// horizontal lines
	for (int i = 0; i < 21; i++) {
		gpoints[22 + 2 * i] = scale * vec2(-5.0, -10.0 + i);
		gpoints[22 + 2 * i + 1] = scale * vec2(5.0, -10.0 + i);
	}

	//lines can have color too!
	for (int i = 0; i < 64; i++) {
		gcolors[i] = base_colors[7];
	}

	// Create and bind a vertex array object
	glGenVertexArrays( 1, &grid_vao );
	glBindVertexArray( grid_vao );

	// Create and initialize a buffer object
	// GLuint grid_buffer;
	glGenBuffers( 1, &grid_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, grid_buffer );

	glBufferData( GL_ARRAY_BUFFER, sizeof(gpoints) + sizeof(gcolors), gpoints, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(gpoints), gpoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(gpoints), sizeof(gcolors), gcolors );
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(gpoints)));

	// glGenVertexArrays( 1, &board_vao ); // initialize for makeBoard

	makeBoard();
	makePixels();
	makeTile();
}

//----------------------------------------------------------------------------

void display( void ) {
	glClear( GL_COLOR_BUFFER_BIT );     // clear the window

	// Data for drawing tiles
	// Create and bind a vertex array object
	glGenVertexArrays( 1, &tile_vao );
	glBindVertexArray( tile_vao );

	// Create and initialize a buffer object
	// GLuint tile_buffer;
	glGenBuffers( 1, &tile_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, tile_buffer );

	//glBufferData( GL_ARRAY_BUFFER, sizeof(points1), points1, GL_STATIC_DRAW );
	glBufferData( GL_ARRAY_BUFFER, sizeof(tpoints) + sizeof(tcolors), tpoints, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(tpoints), tpoints );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(tpoints), sizeof(tcolors), tcolors );
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(tpoints)));
	glEnableVertexAttribArray(vColor);

	// Draw board
	glBindVertexArray( board_vao );
	glDrawArrays( GL_TRIANGLES, 0, onboardPoints ); // only points on the board

	// Draw tiles
	glBindVertexArray( tile_vao );
	glDrawArrays( GL_TRIANGLES, 0, tNpoints );

	// Draw grid lines
	glBindVertexArray( grid_vao );
	glDrawArrays( GL_LINES, 0, gNpoints );

	//Causes all issued commands to be executed as quickly as they are accepted by the actual rendering engine
	// glFlush();
	glutSwapBuffers(); // better

}

//----------------------------------------------------------------------------
// functions to move tiles up, down, left, right
void down() {
	static vec2 downStep = scale * vec2(0.0, -1.0);
	for (int i = 0; i < 24; i++)
		tpoints[i] = tpoints[i] + downStep;
	pivot = pivot + downStep;
}

void up() {
	static vec2 upStep = scale * vec2(0.0, 1.0);
	for (int i = 0; i < 24; i++)
		tpoints[i] = tpoints[i] + upStep;
	pivot = pivot + upStep;
}

void left() {
	static vec2 leftStep = scale * vec2(-1.0, 0.0);
	for (int i = 0; i < 24; i++)
		tpoints[i] = tpoints[i] + leftStep;
	pivot = pivot + leftStep;

}

void right() {
	static vec2 rightStep = scale * vec2(1.0, 0.0);
	for (int i = 0; i < 24; i++)
		tpoints[i] = tpoints[i] + rightStep;
	pivot = pivot + rightStep;

}

// restart game by resetting all initialized values
void restart() {
	gameOver = 0;
	onboardPoints = 0;
	makePixels();
}
//----------------------------------------------------------------------------

//timed function. We intended to execute this every one second.
void dropDelay(int) {
	down();
	if (isSafe() == 0 || isSafe() == 3) {
		// reverse the tpoints
		up();
		updateBoard();
		updatePixels();
		// keep deleting all filled rows
		deleteMultipleRow();
		redrawBoard();
		// when a tile reachs a bottom or collides others, make a new tile
		makeTile();
	}

	//Always remember to update your canvas
	glutPostRedisplay();

	//then we can set another identical event in 1000 miliseconds in the future, that is how we keep the triangle rotating
	glutTimerFunc(1000.0, dropDelay, 0);
}

//----------------------------------------------------------------------------
//If you want to use arrows, take a look at this function
//It works similarly, but you have no acess to the ASCII keys using it. Please see the link below for details.
//https://www.opengl.org/resources/libraries/glut/spec3/node54.html
void keyboardSpecial( int key, int x, int y ) {
	vec2 backup[24];
	switch ( key ) {
		case  GLUT_KEY_DOWN:
		down();
		if (isSafe() != 4) {
			up();
			updateBoard();
			updatePixels();
			deleteMultipleRow();
			redrawBoard();
			makeTile();
		}
		break;
		case  GLUT_KEY_UP:
		// backup tpoints
		for (int i = 0; i < 24; i++) {
			backup[i] = tpoints[i];
		}
		rotateTile();
		if (isSafe() == 1)
			right();
		else if (isSafe() == 2)
			left();
		else if (isSafe() !=4) {
			for (int i = 0; i < 24; i++)
			tpoints[i] = backup[i];
		}
		break;
		case  GLUT_KEY_LEFT:
		left();
		if (isSafe() != 4)
		right();
		break;
		case  GLUT_KEY_RIGHT:
		right();
		if (isSafe() != 4)
		left();
		break;
	}
	glutPostRedisplay();
}

//bind most keys on your keyboard to this function
void keyboard( unsigned char key, int x, int y ) {
	switch ( key ) {
		case 'r':
			restart();
			makeBoard();
			makeTile();
		break;
		case 'q':
			exit( EXIT_SUCCESS );
		break;
		//Esc pressed!
		case 033:
			exit( EXIT_SUCCESS );
		break;
	}

}
//----------------------------------------------------------------------------
