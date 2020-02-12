/*
 * constants.h
 *
 *  Created on: Oct 8, 2016
 *      Author: Wituz
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "sys/types.h"
#include "controller.h"
#include "imagekit/images.h"

#define OT_LENGTH 1
#define PACKETMAX 300

#define __ramsize   0x00200000
#define __stacksize 0x00004000

#define TYPE_LINE 0
#define TYPE_BOX 1

#define SCREEN_MODE_PAL 0
#define SCREEN_MODE_NTSC 1

#define DEBUG 1



typedef struct {
	int r;
	int g;
	int b;
} Color;

typedef struct {
	LINE_F2 line;
	int type;
} Line;

typedef struct {
	Line line[4];
	int type;
} Box;

typedef struct {
	RECT rect;
	RECT crect;
	GsIMAGE tim_data;
	GsSPRITE sprite;
} Image;

int 		SCREEN_WIDTH, SCREEN_HEIGHT;
GsOT 		orderingTable[2];
GsOT_TAG  	minorOrderingTable[2][1<<OT_LENGTH];
PACKET 		GPUOutputPacket[2][PACKETMAX];
short 		currentBuffer;
Color 		systemBackgroundColor;

Image createImage(unsigned char imageData[]) {

	// Initialize image
	Image image;
	GsGetTimInfo ((u_long *)(imageData+4),&image.tim_data);

	// Load the image into the frame buffer
	image.rect.x = image.tim_data.px;            	// tim start X coord to put image data in frame buffer
	image.rect.y = image.tim_data.py;            	// tim start Y coord to put image data in frame buffer
	image.rect.w = image.tim_data.pw;            	// data width
	image.rect.h = image.tim_data.ph;            	// data height
	printf("Rect info {x=%d, y=%d, w=%d, h=%d}\n", image.rect.x, image.rect.y, image.rect.w, image.rect.h);
	LoadImage(&image.rect, image.tim_data.pixel);

	// Load the CLUT into the frame buffer
	image.crect.x = image.tim_data.cx;            	// x pos to put CLUT in frame buffer
	image.crect.y = image.tim_data.cy;           	// y pos to put CLUT in frame buffer
	image.crect.w = image.tim_data.cw;            	// width of CLUT
	image.crect.h = image.tim_data.ch;            	// height of CLUT
	LoadImage(&image.crect, image.tim_data.clut);
	printf("CLUT info {x=%d, y=%d, w=%d, h=%d}\n", image.crect.x, image.crect.y, image.crect.w, image.crect.h);

	// Initialize sprite
	image.sprite.attribute = 0x1000000; 			// (0x1 = 8-bit, 0x2 = 16-bit)
	image.sprite.x = 0;                         	// draw at x coord
	image.sprite.y = 0;                          	// draw at y coord
	image.sprite.w = image.tim_data.pw * 2;         	// width of sprite
	image.sprite.h = image.tim_data.ph;             // height of sprite
	printf("Sprite mes {attribute = %d, x=%d, y=%zd, w=%d, h=%d}\n", image.sprite.attribute, image.sprite.x, image.sprite.y, image.sprite.w, image.sprite.h);

	image.sprite.tpage = GetTPage(
			1,   									// 0=4-bit, 1=8-bit, 2=16-bit
			1,   									// semitransparency rate
			image.tim_data.px, 						// framebuffer pixel x
			image.tim_data.py  						// framebuffer pixel y
	);
	printf("Tpage info {tpage = %d}\n", image.sprite.tpage);


	image.sprite.r = 128;							// color red blend
	image.sprite.g = 128;							// color green blend
	image.sprite.b = 128;							// color blue blend
	image.sprite.u=(image.tim_data.px - 320) * 2;                               // position within timfile for sprite
	image.sprite.v=image.tim_data.py;								// position within timfile for sprite
	image.sprite.cx = image.tim_data.cx;            // CLUT location x
	image.sprite.cy = image.tim_data.cy;            // CLUT location y
	image.sprite.mx = 0;                            // rotation x coord
	image.sprite.my = 0;                            // rotation y coord
	image.sprite.scalex = ONE;                      // scale x (ONE = 100%)
	image.sprite.scaley = ONE;                      // scale y (ONE = 100%)
	image.sprite.rotate = 0;                        // rotation



	return image;

}

Image moveImage(Image image, int x, int y) {
	image.sprite.x = x;
	image.sprite.y = y;
	return image;
}




//Creates a color from RGB
Color createColor(int r, int g, int b) {
	Color color = {.r = r, .g = g, .b = b};
	return color;
}


Line createLine(Color color, int x1, int y1, int x2, int y2) {
	Line line;
	line.type = TYPE_LINE;
	SetLineF2(&line.line);
	setRGB0(&line.line, color.r, color.g, color.b);
	setXY2(&line.line, x1, y1, x2, y2);
	return line;
}

Box createBox(Color color, int x1, int y1, int x2, int y2) {
	Line top    = createLine(color, x1, y1, x2, y1);
	Line bottom = createLine(color, x1, y2, x2, y2);
	Line left   = createLine(color, x1, y1, x1, y2);
	Line right  = createLine(color, x2, y1, x2, y2);
	Box box;
	box.type = TYPE_BOX;
	box.line[0] = top;
	box.line[1] = bottom;
	box.line[2] = left;
	box.line[3] = right;
	return box;
}

Line moveLine(Line line, int x1, int y1, int x2, int y2) {
	line.line.x0 = x1;
	line.line.y0 = y1;
	line.line.x1 = x2;
	line.line.y1 = y2;
	return line;
}

Box moveBox(Box box, int x1, int y1) {
	int currentWidth = box.line[0].line.x1 - box.line[0].line.x0;
	int currentHeight = box.line[2].line.y1 - box.line[2].line.y1;
	int x2 = x1 + currentWidth;
	int y2 = y1 + currentWidth;
	box.line[0] = moveLine(box.line[0], x1, y1, x2, y1);
	box.line[1] = moveLine(box.line[1], x1, y2, x2, y2);
	box.line[2] = moveLine(box.line[2], x1, y1, x1, y2);
	box.line[3] = moveLine(box.line[3], x2, y1, x2, y2);
	return box;
}


void drawLine(Line line) {
	DrawPrim(&line.line);
}

void drawBox(Box box) {
	int i;
	for(i = 0; i < 4; i++) {
		DrawPrim(&box.line[i].line);
	}
}

void drawImage(Image image) {
	currentBuffer = GsGetActiveBuff();
	GsSortSprite(&image.sprite, &orderingTable[currentBuffer], 0);
}

//Set the screen mode to either SCREEN_MODE_PAL or SCREEN_MODE_NTSC
void setScreenMode(int mode) {
	if (mode == SCREEN_MODE_PAL) { // SCEE string address
    	// PAL MODE
    	SCREEN_WIDTH = 320;
    	SCREEN_HEIGHT = 256;
    	if (DEBUG) printf("Setting the PlayStation Video Mode to (PAL %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT,")");
    	SetVideoMode(1);
    	if (DEBUG) printf("Video Mode is (%d)\n",GetVideoMode());
   	} else {
     	// NTSC MODE
     	SCREEN_WIDTH = 320;
     	SCREEN_HEIGHT = 240;
     	if (DEBUG) printf("Setting the PlayStation Video Mode to (NTSC %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT,")");
     	SetVideoMode(0);
     	if (DEBUG) printf("Video Mode is (%d)\n",GetVideoMode());
   }
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);
}

void setBackgroundColor(Color color) {
	systemBackgroundColor = color;
}

//Initialize screen, 
void initializeScreen() {
	if (*(char *)0xbfc7ff52=='E') setScreenMode(SCREEN_MODE_PAL);
   	else setScreenMode(SCREEN_MODE_NTSC);

	SetDispMask(1);
	ResetGraph(0);
	clearVRAM();
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); //Set up interlation..
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);	//..and double buffering.
	systemBackgroundColor = createColor(0, 0, 255);
	initializeOrderingTable();
}

void initializeDebugFont() {
	FntLoad(960, 256);
	SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); //Sets the dumped font for use with FntPrint();
}

void initializeOrderingTable(){
	GsClearOt(0,0,&orderingTable[GsGetActiveBuff()]);

	// initialise the ordering tables
	orderingTable[0].length = OT_LENGTH;
	orderingTable[1].length = OT_LENGTH;
	orderingTable[0].org = minorOrderingTable[0];
	orderingTable[1].org = minorOrderingTable[1];

	GsClearOt(0,0,&orderingTable[0]);
	GsClearOt(0,0,&orderingTable[1]);
}

void clearVRAM() {
	RECT rectTL;
	setRECT(&rectTL, 0, 0, 1024, 512);
	ClearImage2(&rectTL, 0, 0, 0);
    DrawSync(0);
    return;
}

void clearDisplay() {
	currentBuffer = GsGetActiveBuff();
	FntFlush(-1);
	GsSetWorkBase((PACKET*)GPUOutputPacket[currentBuffer]);
	GsClearOt(0, 0, &orderingTable[currentBuffer]);
}

void display() {
	currentBuffer = GsGetActiveBuff();
	DrawSync(0);
	VSync(0);
	GsSwapDispBuff();
	GsSortClear(systemBackgroundColor.r, systemBackgroundColor.g, systemBackgroundColor.b, &orderingTable[currentBuffer]);
	GsDrawOt(&orderingTable[currentBuffer]);
}
