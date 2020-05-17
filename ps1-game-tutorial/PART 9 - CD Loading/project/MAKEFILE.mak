# ----------------------------
# PlayStation 1 Psy-Q MAKEFILE
# ----------------------------

all:
	#pragma warning( disable : 4507 34 )
	ccpsx -Xo0x80010000 -Wall -O2 main.c -o main.cpe
	cpe2x /ce main.cpe
