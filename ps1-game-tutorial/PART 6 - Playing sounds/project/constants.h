/*
 * constants.h
 *
 *  Created on: Oct 8, 2016
 *      Author: Wituz
 */

#pragma once

#include <STDLIB.H>
#include <STDIO.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>
#include <LIBETC.H>
#include <LIBSPU.H>
#include <SYS/TYPES.H>
#include "controller.h"
#include "imagekit/images.h"

#define OT_LENGTH 1
#define PACKETMAX 300
#define TYPE_LINE 0
#define TYPE_BOX 1
#define SCREEN_MODE_PAL 0
#define SCREEN_MODE_NTSC 1
#define DEBUG 0
#define SOUND_MALLOC_MAX 10

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

int 		  SCREEN_WIDTH, SCREEN_HEIGHT;
GsOT 		  orderingTable[2];
GsOT_TAG  	  minorOrderingTable[2][1<<OT_LENGTH];
PACKET 		  GPUOutputPacket[2][PACKETMAX];
short 		  currentBuffer;
Color 		  systemBackgroundColor;
SpuCommonAttr l_c_attr;
SpuVoiceAttr  g_s_attr;
unsigned long l_vag1_spu_addr;

void audioInit() {
	SpuInit();
	SpuInitMalloc (SOUND_MALLOC_MAX, SPU_MALLOC_RECSIZ * (SOUND_MALLOC_MAX + 1));
	l_c_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	l_c_attr.mvol.left  = 0x3fff; // set master left volume
	l_c_attr.mvol.right = 0x3fff; // set master right volume
	SpuSetCommonAttr (&l_c_attr);
}

void audioTransferVagToSPU(char* sound, int sound_size, int voice_channel) {
	SpuSetTransferMode (SpuTransByDMA); // set transfer mode to DMA
	l_vag1_spu_addr = SpuMalloc(sound_size); // allocate SPU memory for sound 1
	SpuSetTransferStartAddr(l_vag1_spu_addr); // set transfer starting address to malloced area
	SpuWrite (sound + 0x30, sound_size); // perform actual transfer
	SpuIsTransferCompleted (SPU_TRANSFER_WAIT); // wait for DMA to complete
	// mask which specific voice attributes are to be set
	g_s_attr.mask =
			(
					SPU_VOICE_VOLL |
					SPU_VOICE_VOLR |
					SPU_VOICE_PITCH |
					SPU_VOICE_WDSA |
					SPU_VOICE_ADSR_AMODE |
					SPU_VOICE_ADSR_SMODE |
					SPU_VOICE_ADSR_RMODE |
					SPU_VOICE_ADSR_AR |
					SPU_VOICE_ADSR_DR |
					SPU_VOICE_ADSR_SR |
					SPU_VOICE_ADSR_RR |
					SPU_VOICE_ADSR_SL
			);

	g_s_attr.voice = (voice_channel);

	g_s_attr.volume.left  = 0x1fff;
	g_s_attr.volume.right = 0x1fff;

	g_s_attr.pitch        = 0x1000;
	g_s_attr.addr         = l_vag1_spu_addr;
	g_s_attr.a_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.s_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.r_mode       = SPU_VOICE_LINEARDecN;
	g_s_attr.ar           = 0x0;
	g_s_attr.dr           = 0x0;
	g_s_attr.sr           = 0x0;
	g_s_attr.rr           = 0x0;
	g_s_attr.sl           = 0xf;

	SpuSetVoiceAttr (&g_s_attr);
}

void audioPlay(int voice_channel) {
	SpuSetKey(SpuOn, voice_channel);
}

void audioChannelConfigure() {
	// mask which specific voice attributes are to be set

}

void audioFree(unsigned long sound_address) {
	SpuFree(sound_address);
}

Image createImage(unsigned char imageData[]) {

	// Initialize image
	Image image;
	GsGetTimInfo ((u_long *)(imageData+4),&image.tim_data);

	// Load the image into the frame buffer
	image.rect.x = image.tim_data.px;            	// tim start X coord to put image data in frame buffer
	image.rect.y = image.tim_data.py;            	// tim start Y coord to put image data in frame buffer
	image.rect.w = image.tim_data.pw;            	// data width
	image.rect.h = image.tim_data.ph;            	// data height
	LoadImage(&image.rect, image.tim_data.pixel);

	// Load the CLUT into the frame buffer
	image.crect.x = image.tim_data.cx;            	// x pos to put CLUT in frame buffer
	image.crect.y = image.tim_data.cy;           	// y pos to put CLUT in frame buffer
	image.crect.w = image.tim_data.cw;            	// width of CLUT
	image.crect.h = image.tim_data.ch;            	// height of CLUT
	LoadImage(&image.crect, image.tim_data.clut);

	// Initialize sprite
	image.sprite.attribute = 0x1000000; 			// (0x1 = 8-bit, 0x2 = 16-bit)
	image.sprite.x = 0;                         	// draw at x coord
	image.sprite.y = 0;                          	// draw at y coord
	image.sprite.w = image.tim_data.pw * 2;         // width of sprite
	image.sprite.h = image.tim_data.ph;             // height of sprite

	image.sprite.tpage = GetTPage(
			1,   								// 0=4-bit, 1=8-bit, 2=16-bit
			1,   								// semitransparency rate
			image.tim_data.px, 						// framebuffer pixel x
			image.tim_data.py  						// framebuffer pixel y
	);

	image.sprite.r = 128;							// color red blend
	image.sprite.g = 128;							// color green blend
	image.sprite.b = 128;							// color blue blend
	image.sprite.u=(image.tim_data.px - 320) * 2;   // position within timfile for sprite
	image.sprite.v=image.tim_data.py;				// position within timfile for sprite
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
