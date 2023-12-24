#define RSGLDEF 
#include "RSGL.h"

#define COMPONENT_IMPLEMENTATION
#include "component.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    comp_init();

    bool running = true;

    RSGL_audio buzzer_audio;

    RSGL_audio_loadFile(&buzzer_audio, "beep.mp3");

    RSGL_window* window = RSGL_createWindow("Aircuit", RSGL_RECT(0, 0, 1000, 600), RSGL_CENTER);

    RSGL_setFont(RSGL_loadFont("SansPosterBold.ttf"));

    while (running) {
        while (RGFW_window_checkEvent(window)) {
            if (window->event.type == RGFW_quit) {
                running = false;
                break;
            }

            if (window->event.type == RGFW_mouseButtonPressed)
               comp_pressed(window->event.x, window->event.y, RGFW_isPressedI(window, RGFW_ShiftL) || RGFW_isPressedI(window, RGFW_ShiftR));
            
            else if (window->event.type == RGFW_mousePosChanged)
                comp_move(window->event.x, window->event.y);

            else if (window->event.type == RGFW_mouseButtonReleased)
                comp_click(window->event.x, window->event.y);
        }

        comp_draw(window->r.w, window->r.h, (void*)&buzzer_audio);

        RSGL_window_clear(window, RSGL_RGB(60, 60, 60));
    }

    RSGL_audio_free(buzzer_audio);

    comp_free();
    RSGL_window_close(window);
}