#include "constants.h"

Line line;
Box box;

int box_x, box_y;

int main() {
	initialize();

	while(1) {
		update();
		draw();
		display();
	}
}

void initialize() {
	initializeScreen();
	initializePad();

	box_x = 128;
	box_y = 128;

	setBackgroundColor(createColor(255, 255, 255));
	line = createLine(createColor(0, 0, 0), 32, 32, 64, 64);
	box = createBox(createColor(0, 0, 255), 128, 128, 164, 164);
}

void update() {
	padUpdate();
	if(padCheck(Pad1Up)) {
		box_y -= 2;
		box = moveBox(box, box_x, box_y);
	}

	if(padCheck(Pad1Down)) {
		box_y += 2;
		box = moveBox(box, box_x, box_y);
	}

	if(padCheck(Pad1Left)) {
		box_x -= 2;
		box = moveBox(box, box_x, box_y);
	}

	if(padCheck(Pad1Right)) {
		box_x += 2;
		box = moveBox(box, box_x, box_y);
	}
}

void draw() {
	drawLine(line);
	drawBox(box);
}
