#include "constants.h"
#include "sounds/hpup.h"
#include "sounds/crate.h"

void initialize();
void update();
void draw();

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
    setBackgroundColor(createColor(0, 0, 255));

    audioInit();
    audioTransferVagToSPU(&hpup, hpup_size, SPU_0CH);
    audioTransferVagToSPU(&crate, crate_size, SPU_1CH);
    audioPlay(SPU_0CH);
    audioPlay(SPU_1CH);

}
void update() {}
void draw() {}