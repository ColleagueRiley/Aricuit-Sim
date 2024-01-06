#define RSGLDEF 
#include "RSGL.h"

#define COMPONENT_IMPLEMENTATION
#include "component.h"

#include <stdlib.h>

int main() {
    comp_init();

    bool running = true;

    FILE* romFile = fopen("rom.rom", "r");

    fseek(romFile, 0U, SEEK_END);
    size_t size = ftell(romFile);
    fseek(romFile, 0U, SEEK_SET);

    char* rom = (char*)malloc(sizeof(char) * size);
    u8* comp_rom = malloc(sizeof(u8) * 255 * 8);

    fread(rom, size, 1, romFile);

    size_t i, index;
    for (i = 0, index = 0; i < size; i++)
        if (rom[i] == '1' || rom[i] == '0') {
            comp_rom[index] = (rom[i] == '1') ? 1 : 0;
            index++;
        }
    
    fclose(romFile);
    free(rom);

    RSGL_audio buzzer_audio;
    RSGL_audio_loadFile(&buzzer_audio, "beep.mp3");

    RSGL_window* window = RSGL_createWindow("Aricuit Sim", RSGL_RECT(0, 0, 0, 0), RSGL_FULLSCREEN | RSGL_NO_BORDER);

    RSGL_setFont(RSGL_loadFont("SansPosterBold.ttf"));

    comp_loadSave("untitled.save");

    while (running) {
        while (RGFW_window_checkEvent(window)) {
            if (window->event.type == RGFW_quit) {
                running = false;
                break;
            }

            if (window->event.type == RGFW_mouseButtonPressed) {
               comp_pressed(window->event.x, window->event.y, RGFW_isPressedI(window, RGFW_ShiftL) || RGFW_isPressedI(window, RGFW_ShiftR));
            }
            else if (window->event.type == RGFW_mousePosChanged)
                comp_move(window->event.x, window->event.y);
            else if (window->event.type == RGFW_mouseButtonReleased)
                comp_click(window->event.x, window->event.y);
        }

        if (RGFW_isPressedI(window, RGFW_Escape) || ((RGFW_isPressedI(window, RGFW_ControlL) || RGFW_isPressedI(window, RGFW_ControlR)) && RGFW_isPressedI(window, RGFW_q))) {
            running = false;
            break;
        }

        if (window->event.type == RGFW_keyPressed && 
            (RGFW_isPressedI(window, RGFW_ControlL) || RGFW_isPressedI(window, RGFW_ControlR)) && RGFW_isPressedI(window, RGFW_s))
                comp_save();

        comp_draw(window->r.w, window->r.h, (void*)&buzzer_audio, comp_rom);

        RSGL_window_clear(window, RSGL_RGB(60, 60, 60));
    }

    free(comp_rom);
    RSGL_audio_free(buzzer_audio);

    comp_free();
    RSGL_window_close(window);
}