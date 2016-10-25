#include "constants.h"
#include "pong.h"

Line lineTop, lineBottom, lineMiddle;
Bat batLeft, batRight;
Ball ball;
char text_debug[100] = "";

int main() {
	initialize();
	while(1) {
		update();
		draw();
		display();
	}
	return 0;
}
int test;
void initialize() {
	initializeScreen();
	initializePad();
	initializeDebugFont();
	setBackgroundColor(createColor(0, 0, 0));
	lineTop = createLine(createColor(255, 255, 255), 0, 20, SCREEN_WIDTH, 20);
	lineBottom = createLine(createColor(255, 255, 255), 0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20);
	lineMiddle = createLine(createColor(150, 150, 150), SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	batLeft = createBat(20);
	batRight = createBat(SCREEN_WIDTH - 20);
	ball = createBall();
	scoreboard = createScoreboard();
}

void update() {
	padUpdate();
	if(padCheck(Pad1Up)) batLeft = moveBat(batLeft, -2);
	if(padCheck(Pad1Down)) batLeft = moveBat(batLeft, 2);
	if(padCheck(Pad2Up)) batRight = moveBat(batRight, -2);
	if(padCheck(Pad2Down)) batRight = moveBat(batRight, 2);
	if(padCheck(Pad1Start) || padCheck(Pad2Start)) ball = kickBall(ball);
	ball = moveBall(ball, batLeft, batRight);

	sprintf(text_debug, "%d:%d", scoreboard.score_left, scoreboard.score_right);
}

void draw() {
	drawLine(lineMiddle);
	drawLine(lineTop);
	drawLine(lineBottom);
	drawBat(batLeft);
	drawBat(batRight);
	drawBall(ball);
	FntPrint(text_debug);
}
