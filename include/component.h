/*
* Copyright (c) 2023 ColleagueRiley ColleagueRiley@gmail.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following r estrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/

#ifndef COMPDEF
#define COMPDEF inline static
#endif

#ifndef COMPONENT_CHILDREN_BASE
#define COMPONENT_CHILDREN_BASE 16
#endif

#ifndef COMPONENT_BASE
#define COMPONENT_BASE 34
#endif

#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

typedef enum componentType {
    INPUT = 0,
    LED,
    NOT,
    AND,
    OR,
    BUTTON,
    CONST_ONE,
    CONST_ZERO,
    BUZZER,
    CLOCK,
    ROM,
    BITS,
} componentType;

typedef struct component component;

struct component {
    size_t x, y;
    componentType type;

    bool active;

    size_t parents_len;
    size_t parents_cap;
    component** parents;

    size_t children_len;
    size_t children_cap;
    component** children;

    time_t startTime;
};

#define COMPONENT(x, y, type) (component){x, y, type}

COMPDEF void comp_init(void);
COMPDEF void comp_save(void);
COMPDEF component* add_component(component c);
COMPDEF void delete_component(component* comp);
COMPDEF void comp_free(void);

COMPDEF size_t component_addChild(component* comp, component* child);
COMPDEF void component_deleteChild(component* comp, component* child);
COMPDEF void component_deleteParent(component* comp, component* parent);

COMPDEF void comp_pressed(size_t x, size_t y, uint8_t moving);
COMPDEF void comp_click(size_t x, size_t y);

COMPDEF void comp_draw(size_t w, size_t h, void* buzzer_audio, unsigned char* rom);
#endif

#ifdef COMPONENT_IMPLEMENTATION

#include "silistr.h"

component* comp_components;
size_t comp_components_len;
size_t comp_components_cap;

size_t comp_scrollX = 0;

COMPDEF void comp_init(void) {
    comp_components_cap = COMPONENT_BASE;
    comp_components_len = 0;
    comp_scrollX = 0;

    comp_components = (component*)malloc(sizeof(component) * comp_components_cap);
}

char* comp_defaultName = "untitled.save";

COMPDEF void comp_saveNode(component comp, siString* str) {
    static char dataStr[1024];
    sprintf(dataStr, "%li,%li,%i", comp.x, comp.y, comp.type);

    si_string_append(str, dataStr);

    if (comp.children_len)
        si_string_append_len(str, "{", 1);
    si_string_append_len(str, "\n", 1);

    size_t i;
    for (i = 0; i < comp.children_len; i++) {
        comp_saveNode(*comp.children[i], str);
    }
    
    if (comp.children_len) {
        si_string_append_len(str, "}\n", 2);
    }
}

COMPDEF void comp_save(void) {
    FILE* saveFile = fopen(comp_defaultName, "w+");

    siString str = si_string_make_reserve(1025);

    size_t i;
    for (i = 0; i < comp_components_len; i++) {
        component comp = comp_components[i];
        if (comp.parents_len)
            continue;
        
        comp_saveNode(comp, &str);

        si_string_append_len(&str, "\n", 1);
    }
    
    fwrite(str, si_string_len(str), 1, saveFile);

    si_string_free(str);
    fclose(saveFile);
}

COMPDEF component* comp_loadNode(const char* file, size_t index, size_t size) {
    static char num[25];
    
    component comp;

    size_t i = index, j;
    for (j = 0; j < 3; j++) {
        for (; i < size && (file[i] >= '0' && file[i] <= '9'); i++)
            num[i - index] = file[i]; 
        
        if (file[i] == ',') {
            if (j == 0) 
                comp.x = si_cstr_to_u64(num); 
            else 
                comp.y = si_cstr_to_u64(num); 
            
//            printf("%li %li\n", comp.x, comp.y);
            continue;
        }

        if (file[i] == '\n' || file[i] == '{')
            comp.type = si_cstr_to_u64(num);

        if (file[i] == '\n')
            break;
        
        if (file[i] == '{') {
            i += 2;
            
            //component_addChild(&comp, comp_loadNode(file, i, size));
            break;
        }

        break;
    }

    return add_component(comp);
}

COMPDEF void comp_loadSave(const char* file) {
    FILE* saveFile = fopen(file, "r");

    fseek(saveFile, 0U, SEEK_END);
    size_t size = ftell(saveFile);
    fseek(saveFile, 0U, SEEK_SET);

    char* fileData = malloc(sizeof(char) * size);
    fread(fileData, size, 1, saveFile);

    size_t i;
    for (i = 0; i < size; i++)
        if (fileData[i] >= '0' && fileData[i] <= '9')
            comp_loadNode(fileData, i, size);

    fclose(saveFile);
    free(fileData);
}

COMPDEF component* add_component(component c) {
    c.active = false;
    c.children_len = 0;
    c.children_cap = COMPONENT_CHILDREN_BASE;
    c.children = (component**)malloc(sizeof(component*) * c.children_cap);
    c.parents = (component**)malloc(sizeof(component*) * c.children_cap);

    c.parents_cap = 9;
    c.parents_len = 0;

    if (comp_components_len >= comp_components_cap) {
        comp_components_cap += (COMPONENT_BASE / 4);
        comp_components = (component*)realloc(comp_components, sizeof(component) * comp_components_cap);
    }

    comp_components[comp_components_len] = c;
    comp_components_len++;

    if (c.type == CONST_ONE)
        c.active = true;

    c.startTime = 0;

    return &comp_components[comp_components_len - 1];
}

COMPDEF void delete_component(component* comp) {
    size_t i, j;

    for (i = 0; i < comp_components_len; i++) {
        if (&comp_components[i] != comp) 
            continue;

        for (j = 0; j < comp->parents_len; j++)
            component_deleteParent(comp, comp->parents[j]);

        for (j = 0; j < comp->children_len; j++)
            component_deleteChild(comp, comp->children[j]);

        free(comp_components[i].children);
        free(comp_components[i].parents);

        for (j = i + 1; j < comp_components_len; j++)
            comp_components[i + (j - i - 1)] = comp_components[j];
        
        comp_components_len--;

        return;
    }
}

COMPDEF void comp_free(void) {
    size_t i;
    for (i = 0; i < comp_components_len; i++)
        delete_component(&comp_components[i]);

    free(comp_components);
}

COMPDEF size_t component_addChild(component* comp, component* child) {
    size_t i;
    for (i = 0; i < comp->children_len; i++)
        if (child == comp->children[i]) {
            component_deleteChild(comp, child);
        }

    if (comp->children_len >= comp->children_cap) {
        comp->children_cap += (COMPONENT_CHILDREN_BASE / 4);
        comp->children = (component**)realloc(comp->children, sizeof(component*) * comp->children_cap);
    }

    if (child->parents_len >= child->parents_cap) {
        child->parents_cap += (COMPONENT_BASE / 4);
        child->parents = (component**)realloc(child->parents, sizeof(component*) * child->parents_cap);
    }

    child->parents[child->parents_len] = comp;
    child->parents_len++;

    comp->children[comp->children_len] = child;
    comp->children_len++;

    return i;
}

COMPDEF void component_deleteChild(component* comp, component* child) {
    size_t i, j;

    for (i = 0; i < comp->children_len; i++) {
        if (comp->children[i] != child) 
            continue;
        
        for (j = i + 1; j < comp->children_len; j++)
            comp->children[i + (j - i - 1)] = comp->children[j];

        comp->children_len--;

        component_deleteParent(child, comp);
        return;
    }
}

COMPDEF void component_deleteParent(component* comp, component* parent) {
    size_t i, j;

    for (i = 0; i < comp->parents_len; i++) {
        if (comp->parents[i] != parent) 
            continue;
        
        for (j = i + 1; j < comp->parents_len; j++)
            comp->parents[i + (j - i - 1)] = comp->parents[j];

        comp->parents_len--;

        component_deleteChild(parent, comp);
        return;
    }
}

component* compPressed = NULL;
uint8_t compMoving = 0;

#define comp_collide(x, y, w, h, x1, y1) ((x1) >= (x) && (x1) <= (x) + (w) && (y1) >= (y) && (y1) <= (y) + (h))
/* #define comp_collideComp(x, y, w, h, x1, y1, w1, h1) ((x) + (w) >= (x1) && (x) <= (x1) + (w1) && (y) + (h) >= (y1) && (y) <= (y1) + (h1)) */

size_t compX, compY, compH;

COMPDEF void comp_pressed(size_t x, size_t y, uint8_t moving) {
    compX = x;
    compY = y;
    
    compPressed = NULL;
    compMoving = moving;

    size_t index;
    for (index = 0; index < comp_components_len; index++) {
        component* comp = &comp_components[index];

        if (comp_collide(comp->x, comp->y, 45, 45, x, y) == 0)
            continue;
        
        compPressed = comp;

        if (compPressed->type == BUTTON && moving == false)
            comp->active = true;    
    
        return;
    }

    if (comp_collide(20, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, INPUT));
        compMoving = true;
        return;
    }

    if (comp_collide(110, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, LED));
        compMoving = true;
        return;
    }

    if (comp_collide(200, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, NOT));
        compMoving = true;
        return;
    }

    if (comp_collide(290, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, AND));
        compMoving = true;
        return;
    }

    if (comp_collide(380, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, OR));
        compMoving = true;
        return;
    }

    if (comp_collide(470, (compH - 40), 45, 45, x, y))  {
        compPressed = add_component(COMPONENT(x, y, BUTTON));
        compMoving = true;
        return;
    }

    if (comp_collide(570, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, CONST_ONE));
        compPressed->active = true;
        compMoving = true;
        return;
    }
    
    if (comp_collide(650, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, CONST_ZERO));
        compMoving = true;
        return;
    }


    if (comp_collide(740, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, BUZZER));
        compMoving = true;
        return;
    }

    if (comp_collide(850, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, CLOCK));
        compMoving = true;
        return;
    }

    /*if (comp_collide(940, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, ROM));
        compMoving = true;
        return;
    }

    if (comp_collide(1030, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, BITS));
        compMoving = true;
        return;
    }*/
}

COMPDEF void comp_move(size_t x, size_t y) {
    if (compPressed == NULL)
        return;

    if (compMoving == 0) {
        if (compPressed->type == BUTTON && comp_collide(compPressed->x, compPressed->y, 45, 45, x, y) == 0)
            compPressed->active = false;  

        compX = x;
        compY = y;

        return;
    }

    compPressed->x = x;
    compPressed->y = y;
}

COMPDEF void comp_click(size_t x, size_t y) {
    if (compMoving || compPressed == NULL) {
        if (y > compH - 70)
            delete_component(compPressed);

        compMoving = false;
        compPressed = NULL;
        return;
    }
    
    size_t index;
    component* compClicked = NULL;

    for (index = 0; index < comp_components_len; index++) {
        component* comp = &comp_components[index];

        if (comp_collide(comp->x, comp->y, 45, 45, x, y) == 0)
            continue;
        
        compClicked = comp;
        break;
    }
    
    if (compClicked == compPressed && (compPressed->type != CONST_ZERO && compPressed->type != CONST_ONE))
        compPressed->active = !compPressed->active;
    else if (compClicked != NULL) 
        component_addChild(compPressed, compClicked);

    compPressed = NULL;
}

#ifdef RSGL_H
COMPDEF void comp_draw(size_t w, size_t h, void* buzzer_audio, u8* rom) {
    compH = h;

    size_t index, i;
    for (index = 0; index < comp_components_len; index++) {
        component comp = comp_components[index];
        
        switch (comp.type) {
            case INPUT:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(0, 0, comp.active ? 120 : 100));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(0, 0, comp.active ? 200 : 120));
                break;
            case LED:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(0, 100, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(0, 120, 0));
                if (comp.active)
                    RSGL_drawCircle(RSGL_CIRCLE(comp.x + (45 / 4), comp.y + (45 / 4), 45 / 2), RSGL_RGBA(0, 255, 0, 255));
                break;
            case NOT:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 120 : 100, 0, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 200 : 120, 0, 0));
                break;
            case AND:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 120 : 100, 0, comp.active ? 120 : 100));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 200 : 120, 0, comp.active ? 200 : 120));
                break;
            case OR:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 120 : 100, comp.active ? 120 : 100, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 200 : 145, comp.active ? 200 : 145, 0));
                break;
            case BUTTON:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 120 : 100, comp.active ? 120 : 100, comp.active ? 120 : 100));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 200 : 145, comp.active ? 200 : 145, comp.active ? 200 : 145));
                break;
            case CONST_ONE:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(235, 235, 235));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(255, 255, 255));
                break;
            case CONST_ZERO:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(0, 0, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(35, 35, 35));
                break;     
            case BUZZER:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(0, 0, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(76, 39, 45));

                if (comp.active == 0)
                    break;
                
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + (45 / 4), comp.y + (45 / 4), 45 / 2), RSGL_RGB(0, 0, 0));
                RSGL_audio_play(*(RSGL_audio*)buzzer_audio);

                break;
            case CLOCK:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 220 : 200, comp.active ? 85 : 65, 0));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 255 : 235, comp.active ? 120 : 100, 0));
                break;
            case ROM:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(comp.active ? 144 : 124, comp.active ? 233 : 213, comp.active ? 202 : 182));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(comp.active ? 174: 154, comp.active ? 263 : 243, comp.active ?  232 : 212));
                break;
            case BITS:
                RSGL_drawCircle(RSGL_CIRCLE(comp.x, comp.y, 45), RSGL_RGB(122, 85, 66));
                RSGL_drawCircle(RSGL_CIRCLE(comp.x + 3, comp.y + 3, 39), RSGL_RGB(142, 105, 86));
            default:
                break;
        }

        for (i = 0; i < comp.children_len; i++) {
            component* child = comp.children[i];

            if (comp.type == BITS && comp.active) {
                component* comp = &comp_components[index];


                size_t index = component_addChild(comp, child);
            }
            else if (comp.type == ROM && comp.active) {
                component* comp = &comp_components[index];

                if (comp->parents_len < 2 || comp->parents[comp->parents_len - 1] == 0) {
                    comp->active = false; 
                    RSGL_drawLine(RSGL_POINT((comp->x + 45), comp->y + (45 / 2)), RSGL_POINT(child->x, child->y + (45 / 2)), 3, RSGL_RGB(comp->active ? 255 : 0, 0, 0));
                    continue;
                }

                size_t j;
                u8 rom_index = 0;
                u8 cmp = 0x00000001;

                for (j = 0; j < (comp->parents_len - 1) && j < 8; j++) {
                    if (comp->parents[j])
                        rom_index |= cmp;
                    
                    cmp <<= 1;
                }

                cmp = 0x00000001;
                for (j = 0; j < comp->children_len && j < 8; j++) {
                    comp->children[j]->active = rom[(8 * rom_index) + j];
                    cmp <<= 1;
                }
            }
            else if (comp.type == CLOCK && comp.active) {
                component* comp = &comp_components[index];

                if (comp->startTime == 0)
                    comp->startTime = RGFW_getTime();
                
                u8 curTime = (RGFW_getTime() - comp->startTime);

                size_t j;
                u8 cmp = 0x00000001;

                for (j = 0; j < comp->children_len && j < 8; j++) {
                    comp->children[j]->active = curTime & cmp;
                    cmp <<= 1;
                }

                if (curTime >= 255 || curTime >= pow(2, comp->children_len))
                    comp->startTime = RGFW_getTime();
            }
            else if (child->type == CONST_ZERO) child->active = false;
            else if (child->type == CONST_ONE) child->active = true;
            else if (child->type == NOT)
                child->active = !comp.active; 
            else if (child->type == AND) {
                bool active = comp.active;

                size_t j; 
                for (j = 0; j < child->parents_len && active; j++)
                    if (child->parents[j]->active == false)
                        active = false;
                
                child->active = active;
            }
            else if (child->type == OR) {
                bool active = comp.active;

                size_t j; 
                for (j = 0; j < child->parents_len && active == false; j++)
                    if (child->parents[j]->active == true)
                        active = true;
                
                child->active = active;
            }
            else child->active = comp.active;

            RSGL_drawLine(RSGL_POINT((comp.x + 45), comp.y + (45 / 2)), RSGL_POINT(child->x, child->y + (45 / 2)), 3, RSGL_RGB(comp.active ? 255 : 0, 0, 0));
        }
    }

    /* draw drag line (for connections) */
    if (
        compPressed != NULL && compMoving == false && 
        comp_collide(compPressed->x, compPressed->y, 45, 45, compX, compY) == 0
    )
        RSGL_drawLine(RSGL_POINT(compPressed->x + 45, compPressed->y + (45 / 2)), 
                      RSGL_POINT(compX, compY), 3, RSGL_RGB(0, 0, 0)
                      );
    

    RSGL_drawRect(RSGL_RECT(0, h - 40, w, 40), compMoving ? RSGL_RGB(235, 0, 0) : RSGL_RGB(40, 40, 40));
    RSGL_drawRectOutline(RSGL_RECT(0, h - 40, w, 40), 3, RSGL_RGB(0, 0, 0));

    if (compMoving)
        return;

    /* draw shop */
    RSGL_drawCircle(RSGL_CIRCLE(20, (h - 40), 45), RSGL_RGB(0, 0, 100));
    RSGL_drawCircle(RSGL_CIRCLE(20 + 3, (h - 40) + 3, 39), RSGL_RGB(0, 0, 120));
    RSGL_drawText("INPUT", RSGL_CIRCLE(5, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(110, (h - 40), 45), RSGL_RGB(0, 100, 0));
    RSGL_drawCircle(RSGL_CIRCLE(110 + 3, (h - 40) + 3, 39), RSGL_RGB(0, 120, 0));
    RSGL_drawText("LED", RSGL_CIRCLE(110, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(200, (h - 40), 45), RSGL_RGB(100, 0, 0));
    RSGL_drawCircle(RSGL_CIRCLE(200 + 3, (h - 40) + 3, 39), RSGL_RGB(120, 0, 0));
    RSGL_drawText("NOT", RSGL_CIRCLE(190, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(290, (h - 40), 45), RSGL_RGB(100, 0, 100));
    RSGL_drawCircle(RSGL_CIRCLE(290 + 3, (h - 40) + 3, 39), RSGL_RGB(120, 0, 120));
    RSGL_drawText("AND", RSGL_CIRCLE(290, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(380, (h - 40), 45), RSGL_RGB(100, 100, 0));
    RSGL_drawCircle(RSGL_CIRCLE(380 + 3, (h - 40) + 3, 39), RSGL_RGB(145, 145, 0));
    RSGL_drawText("OR", RSGL_CIRCLE(380, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(470, (h - 40), 45), RSGL_RGB(100, 100, 100));
    RSGL_drawCircle(RSGL_CIRCLE(470 + 3, (h - 40) + 3, 39), RSGL_RGB(145, 145, 145));
    RSGL_drawText("BUTTON", RSGL_CIRCLE(440, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(560, (h - 40), 45), RSGL_RGB(235, 235, 235));
    RSGL_drawCircle(RSGL_CIRCLE(560 + 3, (h - 40) + 3, 39), RSGL_RGB(255, 255, 255));
    RSGL_drawText("1", RSGL_CIRCLE(570, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(650, (h - 40), 45), RSGL_RGB(0, 0, 0));
    RSGL_drawCircle(RSGL_CIRCLE(650 + 3, (h - 40) + 3, 39), RSGL_RGB(35, 35, 35));
    RSGL_drawText("0", RSGL_CIRCLE(660, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(740, (h - 40), 45), RSGL_RGB(0, 0, 0));
    RSGL_drawCircle(RSGL_CIRCLE(740 + 3, (h - 40) + 3, 39), RSGL_RGB(76, 39, 45));
    RSGL_drawText("Buzzer", RSGL_CIRCLE(720, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(850, (h - 40), 45), RSGL_RGB(200, 65, 0));
    RSGL_drawCircle(RSGL_CIRCLE(850 + 3, (h - 40) + 3, 39), RSGL_RGB(235, 100, 0));
    RSGL_drawText("Clock", RSGL_CIRCLE(835, h - 40, 25), RSGL_RGB(120, 120, 120));

    /*RSGL_drawCircle(RSGL_CIRCLE(940, (h - 40), 45), RSGL_RGB(124, 213, 182));
    RSGL_drawCircle(RSGL_CIRCLE(940 + 3, (h - 40) + 3, 39), RSGL_RGB(154, 243, 212));
    RSGL_drawText("ROM", RSGL_CIRCLE(935, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(1030, (h - 40), 45), RSGL_RGB(122, 85, 66));
    RSGL_drawCircle(RSGL_CIRCLE(1030 + 3, (h - 40) + 3, 39), RSGL_RGB(142, 105, 86));
    RSGL_drawText("BITS", RSGL_CIRCLE(1025, h - 40, 25), RSGL_RGB(120, 120, 120));*/
}
#endif
#endif