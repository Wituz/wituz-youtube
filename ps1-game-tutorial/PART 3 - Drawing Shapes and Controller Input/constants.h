/*
 * constants.h
 *
 *  Created on: Oct 8, 2016
 *      Author: Wituz
 */

#pragma once

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libetc.h>
#include "sys/types.h"
#include "controller.h"
#define OT_LENGTH 1
#define PACKETMAX 18
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

int SCREEN_WIDTH, SCREEN_HEIGHT;
GsOT orderingTable[2];
short currentBuffer;
Color systemBackgroundColor;

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
   	
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); //Set up interlation..
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);	//..and double buffering.
	systemBackgroundColor = createColor(0, 0, 255);
}

void initializeDebugFont() {
	FntLoad(960, 256);
	SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); //Sets the dumped font for use with FntPrint();
}

void initializeOrderingTable(GsOT* orderingTable){
	GsClearOt(0,0,&orderingTable[GsGetActiveBuff()]);
}

void display() {
	currentBuffer = GsGetActiveBuff();
	FntFlush(-1);
	GsClearOt(0, 0, &orderingTable[currentBuffer]);
	DrawSync(0);
	VSync(0);
	GsSwapDispBuff();
	GsSortClear(systemBackgroundColor.r, systemBackgroundColor.g, systemBackgroundColor.b, &orderingTable[currentBuffer]);
	GsDrawOt(&orderingTable[currentBuffer]);
}
