/*
 * constants.h
 *
 * Thanks to ORION for his well explained blog and awesome PSX library. (http://onorisoft.free.fr)
 * Thanks to Lameguy64 for his amazing tools and extensive PSX knowledge, free software mindset. (https://github.com/Lameguy64)
 * Thanks to whoever wrote LIBOVR.PDF and LIBREF.PDF back in the day.
 * Thanks to everybody from psxdev.net. <3
 * Big thanks to Shadow for making this possible. 
 * All of you are legends.
 * 
 *  Created on: Oct 8, 2016
 *      Author: Wituz
 */

#ifndef constants_h
#define constants_h

#include <STDLIB.H>
#include <STDIO.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>
#include <LIBETC.H>
#include <LIBSPU.H>
#include <LIBDS.H>
#include <STRINGS.H>
#include <SYS/TYPES.H>
#include "controller.h"

unsigned long __ramsize =   0x00200000; // 2 Megabytes of RAM
unsigned long __stacksize = 0x00004000; // 16 Kilobytes of Stack

#define SECTOR 2048
#define OT_LENGTH 1
#define PACKETMAX 300
#define TYPE_LINE 0
#define TYPE_BOX 1
#define SCREEN_MODE_PAL 0
#define SCREEN_MODE_NTSC 1
#define DEBUG 0
#define SOUND_MALLOC_MAX 10
#define ROT_ONE 4096
#define Sprite GsSPRITE

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
int 		  didInitDs = 0;
GsOT 		  orderingTable[2];
GsOT_TAG  	  minorOrderingTable[2][1<<OT_LENGTH];
PACKET 		  GPUOutputPacket[2][PACKETMAX];
short 		  currentBuffer;
Color* 		  systemBackgroundColor;
SpuCommonAttr l_c_attr;
SpuVoiceAttr  g_s_attr;
unsigned long l_vag1_spu_addr;

void cd_open() {
	if(!didInitDs) {
		didInitDs = 1;
		DsInit();
	}
}

void cd_close() {
	if(didInitDs) {
		didInitDs = 0;
		DsClose();
	}
}

void cd_read_file(unsigned char* file_path, u_long** file) {

	u_char* file_path_raw;
	int* sectors_size;
	DslFILE* temp_file_info;
	sectors_size = malloc3(sizeof(int));
	temp_file_info = malloc3(sizeof(DslFILE));

	// Exit if libDs isn't initialized
	if(!didInitDs) {
		printf("LIBDS not initialized, run cdOpen() first\n");	
		return;
	}

	// Get raw file path
	file_path_raw = malloc3(4 + strlen(file_path));
	strcpy(file_path_raw, "\\");
	strcat(file_path_raw, file_path);
	strcat(file_path_raw, ";1");
	printf("Loading file from CD: %s\n", file_path_raw);

	// Search for file on disc
	DsSearchFile(temp_file_info, file_path_raw);

	// Read the file if it was found
	if(temp_file_info->size > 0) {
		printf("...file found\n");
		printf("...file size: %lu\n", temp_file_info->size);
		*sectors_size = temp_file_info->size + (SECTOR % temp_file_info->size);
		printf("...file buffer size needed: %d\n", *sectors_size);
		printf("...sectors needed: %d\n", (*sectors_size + SECTOR - 1) / SECTOR);
		*file = malloc3(*sectors_size + SECTOR);
		
		DsRead(&temp_file_info->pos, (*sectors_size + SECTOR - 1) / SECTOR, *file, DslModeSpeed);
		while(DsReadSync(NULL));
		printf("...file loaded!\n");
	} else {
		printf("...file not found");
	}

	// Clean up
	free3(file_path_raw);
	free3(sectors_size);
	free3(temp_file_info);
}

void audio_init() {
	SpuInit();
	SpuInitMalloc (SOUND_MALLOC_MAX, (char*)(SPU_MALLOC_RECSIZ * (SOUND_MALLOC_MAX + 1)));
	l_c_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	l_c_attr.mvol.left  = 0x3fff; // set master left volume
	l_c_attr.mvol.right = 0x3fff; // set master right volume
	SpuSetCommonAttr (&l_c_attr);
}

void audio_transfer_vag_to_spu(char* sound, int sound_size, int voice_channel) {
	SpuSetTransferMode (SpuTransByDMA); // set transfer mode to DMA
	l_vag1_spu_addr = SpuMalloc(sound_size); // allocate SPU memory for sound 1
	SpuSetTransferStartAddr(l_vag1_spu_addr); // set transfer starting address to malloced area
	SpuWrite (sound + 0x30, sound_size); // perform actual transfer
	SpuIsTransferCompleted (SPU_TRANSFER_WAIT); // wait for DMA to complete
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

void audio_play(int voice_channel) {
	SpuSetKey(SpuOn, voice_channel);
}

void audio_free(unsigned long spu_address) {
	SpuFree(spu_address);
}

void sprite_create(unsigned char imageData[], int x, int y, GsSPRITE **sprite) {

	// Initialize image
	GsIMAGE* tim_data;
	RECT* rect;
	RECT* crect;
	tim_data = malloc3(sizeof(GsIMAGE));
	GsGetTimInfo ((u_long *)(imageData+4),tim_data);
	rect = malloc3(sizeof(RECT));
	crect = malloc3(sizeof(RECT));

	// Load the image into the GPU memory (frame buffer)
	rect->x = tim_data->px; // x position of image in frame buffer
	rect->y = tim_data->py; // y position of image in frame buffer
	rect->w = tim_data->pw; // width in frame buffer
	rect->h = tim_data->ph; // height in frame buffer
	printf("Framebuffer info {x=%d, y=%d, w=%d, h=%d}\n", rect->x, rect->y, rect->w, rect->h);
	LoadImage(rect, tim_data->pixel);

	// Load the color lookup table (CLUT) into the GPU memory (frame buffer)
	crect->x = tim_data->cx; // x position of CLUT in frame buffer
	crect->y = tim_data->cy; // y position of CLUT in frame buffer
	crect->w = tim_data->cw; // width of CLUT in frame buffer
	crect->h = tim_data->ch; // height of CLUT in frame buffer
	printf("CLUT info {x=%d, y=%d, w=%d, h=%d}\n", crect->x, crect->y, crect->w, crect->h);
	LoadImage(crect, tim_data->clut);

	// Initialize sprite (see GSSprite at PSYQ/DOCS/LIBREF.PDF)
	*sprite = malloc3(sizeof(GsSPRITE));
	(*sprite) -> attribute = 0x0000000;
	(*sprite) -> x = x;
	(*sprite) -> y = y;
	(*sprite) -> w = tim_data->pw * 4;
	(*sprite) -> h = tim_data->ph;
	(*sprite) -> tpage = GetTPage(
			0, 		 // 0=4-bit, 1=8-bit, 2=16-bit
			1,       // semitransparency rate
			rect->x, // framebuffer x position of image
			rect->y  // framebuffer y position of image
	);
	(*sprite)->r = 128;  						// Color red blend
	(*sprite)->g = 128;  						// Color green blend
	(*sprite)->b = 128;  						// Color blue blend
	(*sprite)->u = (tim_data->px * 4) % 256;	// Position within tpage for sprite (0-256)
	(*sprite)->v = (tim_data->py % 256);		// Position within tpage for sprite (0-256)
	(*sprite)->cx = tim_data->cx;               // CLUT location x
	(*sprite)->cy = tim_data->cy;               // CLUT location y
	(*sprite)->rotate = ROT_ONE * 0;            // Rotation, ROT_ONE * (0 to 360) 
	(*sprite)->mx = 0;                          // Rotation x coord
	(*sprite)->my = 0;                          // Rotation y coord
	(*sprite)->scalex = ONE * 1;				// Sprite scale (width)
	(*sprite)->scaley = ONE * 1;				// Sprite scale (height)

	// Clean up
	free3(rect);
	free3(crect);
	free3(tim_data);
}

void sprite_set_rotation(Sprite* sprite, int rotation) {
	sprite -> rotate = ROT_ONE * rotation;
}

void sprite_set_middle(Sprite* sprite, int relative_x, int relative_y) {
	sprite->mx = relative_x;
	sprite->my = relative_y;
}

void sprite_set_position(Sprite* sprite, int x, int y) {
	sprite->x = x;
	sprite->y = y;
} 

void sprite_set_blend_color(Sprite* sprite, Color* color) {
	sprite -> r = color->r;
	sprite -> g = color->g;
	sprite -> b = color->b;
}

void sprite_set_blend_rgb(Sprite* sprite, int r, int g, int b) {
	sprite -> r = r;
	sprite -> g = g;
	sprite -> b = b;
}

void color_create(int r, int g, int b, Color** color) {
	*color = malloc(sizeof(Color));
	(*color) -> r = r;
	(*color) -> g = g;
	(*color) -> b = b;
}

void line_create(Color* color, int x1, int y1, int x2, int y2, Line* line) {
	line = malloc(sizeof(Line));
	line -> type = TYPE_LINE;
	SetLineF2(&line->line);
	setRGB0(&line->line, color->r, color->g, color->b);
	setXY2(&line->line, x1, y1, x2, y2);
}

void box_create(Color *color, int x1, int y1, int x2, int y2, Box* box) {
	box = malloc(sizeof(Box));
	line_create(color, x1, y1, x2, y1, &box -> line[0]);
	line_create(color, x1, y2, x2, y2, &box -> line[1]);
	line_create(color, x1, y1, x1, y2, &box -> line[2]);
	line_create(color, x2, y1, x2, y2, &box -> line[3]);
	box -> type = TYPE_BOX;
}

void line_move(Line* line, int x1, int y1, int x2, int y2) {
	line->line.x0 = x1;
	line->line.y0 = y1;
	line->line.x1 = x2;
	line->line.y1 = y2;
}

void box_move(Box* box, int x1, int y1) {
	
	// Figure out where to move the box to
	int* current_width; 
	int* current_height;
	int* x2;
	int* y2;
	current_width = malloc(sizeof(int));
	current_height = malloc(sizeof(int));
	x2 = malloc(sizeof(int));
	y2 = malloc(sizeof(int));


	*current_width = box->line[0].line.x1 - box->line[0].line.x0;
	*current_height = box->line[2].line.y1 - box->line[2].line.y1;
	*x2 = x1 + *current_width;
	*y2 = y1 + *current_height;

	// Move the lines of the box
	line_move(&box->line[0], x1, y1, *x2, y1);
	line_move(&box->line[1], x1, *y2, *x2, *y2);
	line_move(&box->line[2], x1, y1, x1, *y2);
	line_move(&box->line[3], *x2, y1, *x2, *y2);

	// Clean up
	free3(current_width);
	free3(current_height);
	free3(x2); 
	free3(y2);
}


void draw_line(Line *line) {
	DrawPrim(&line->line);
}

void draw_box(Box *box) {
	int i;
	for(i = 0; i < 4; i++) {
		DrawPrim(&box->line[i].line);
	}
}

void draw_sprite(Sprite *sprite) {
	currentBuffer = GsGetActiveBuff();
	GsSortSprite(sprite, &orderingTable[currentBuffer], 0);
}

//Set the screen mode to either SCREEN_MODE_PAL or SCREEN_MODE_NTSC
void set_screen_mode(int mode) {
	if (mode == SCREEN_MODE_PAL) { // SCEE string address
    	// PAL MODE
    	SCREEN_WIDTH = 320;
    	SCREEN_HEIGHT = 256;
    	if (DEBUG) printf("Setting the PlayStation Video Mode to (PAL %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT);
    	SetVideoMode(1);
    	if (DEBUG) printf("Video Mode is (%ld)\n",GetVideoMode());
   	} else {
     	// NTSC MODE
     	SCREEN_WIDTH = 320;
     	SCREEN_HEIGHT = 240;
     	if (DEBUG) printf("Setting the PlayStation Video Mode to (NTSC %dx%d)\n",SCREEN_WIDTH,SCREEN_HEIGHT);
     	SetVideoMode(0);
     	if (DEBUG) printf("Video Mode is (%ld)\n",GetVideoMode());
   }
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);
}

void set_background_color(Color *color) {
	systemBackgroundColor = color;
}

void initialize_heap() {
	printf("\nReserving 1024KB (1,048,576 Bytes) RAM... \n");
    InitHeap3((void*)0x800F8000, 0x00100000);
    printf("Success!\n");
}

void initialize_ordering_table(){
    GsClearOt(0,0,&orderingTable[GsGetActiveBuff()]);

    // initialise the ordering tables
    orderingTable[0].length = OT_LENGTH;
    orderingTable[1].length = OT_LENGTH;
    orderingTable[0].org = minorOrderingTable[0];
    orderingTable[1].org = minorOrderingTable[1];

    GsClearOt(0,0,&orderingTable[0]);
    GsClearOt(0,0,&orderingTable[1]);
}

void clear_vram() {
    RECT rectTL;
    setRECT(&rectTL, 0, 0, 1024, 512);
    ClearImage2(&rectTL, 0, 0, 0);
    DrawSync(0);
    return;
}

void initialize_screen() {

	// Automatically adjust screen to PAL or NTCS from license
	if (*(char *)0xbfc7ff52=='E') set_screen_mode(SCREEN_MODE_PAL);
   	else set_screen_mode(SCREEN_MODE_NTSC);

	SetDispMask(1);
	ResetGraph(0);
	clear_vram();
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); //Set up interlation..
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);	//..and double buffering.
	color_create(0, 0, 255, &systemBackgroundColor);
	initialize_ordering_table();
}

void initialize_debug_font() {
	FntLoad(960, 256);
	SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); //Sets the dumped font for use with FntPrint();
}

void clear_display() {
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
	GsSortClear(systemBackgroundColor->r, systemBackgroundColor->g, systemBackgroundColor->b, &orderingTable[currentBuffer]);
	GsDrawOt(&orderingTable[currentBuffer]);
}

#endif