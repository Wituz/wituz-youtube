#include "constants.h"

void initialize();
void update();
void draw();

Color* color;
u_long* data[3];
Sprite* sprites[2];

int main() {
    initialize();
    while(1) {
        update();
        clear_display();
        draw();
        display();
    }
}

void initialize() {
    initialize_heap();
    initialize_screen();
    initialize_pad();
    color_create(0, 255, 0, &color);
    set_background_color(color);

    cd_open();
    cd_read_file("HPUP.VAG", &data[0]);
    cd_read_file("CRASH.TIM", &data[1]);
    cd_read_file("PS1.TIM", &data[2]);
    cd_close();

    audio_init();
    audio_transfer_vag_to_spu((u_char *)data[0], SECTOR * 21, SPU_00CH);
    audio_play(SPU_00CH);

    sprite_create((u_char *)data[1], 32, 32, &sprites[0]);
    sprite_create((u_char *)data[2], 100, 100, &sprites[1]);

    free3(data[0]);



}

void update() {

    pad_update();
    if(pad_check(Pad1Right)) {
        sprites[0]->rotate += ROT_ONE * 1;
    }

}

void draw() {
    draw_sprite(sprites[0]);
    draw_sprite(sprites[1]);
}