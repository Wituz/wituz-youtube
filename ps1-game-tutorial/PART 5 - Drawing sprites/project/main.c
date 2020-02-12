#include "constants.h"


Image crash;
Image ps1;

int x = 0;
int y = 0;
int speed = 4;

int main() {
	initialize();

	while(1) {
		update();
		clearDisplay();
		draw();
		display();
	}
}

void initialize() {
	initializeScreen();
	initializePad();
	setBackgroundColor(createColor(0, 0, 0));
	ps1 = createImage(img_ps1);
	crash = createImage(img_crash);
}

void update() {
	padUpdate();
	if(padCheck(Pad1Up)) {
		y -= speed;
		crash = moveImage(crash, x, y);
	}

	if(padCheck(Pad1Down)) {
		y += speed;
		crash = moveImage(crash, x, y);
	}

	if(padCheck(Pad1Left)) {
		x -= speed;
		crash = moveImage(crash, x, y);
	}

	if(padCheck(Pad1Right)) {
		x += speed;
		crash = moveImage(crash, x, y);
	}
}

void draw() {
	drawImage(crash);
	drawImage(ps1);
}
