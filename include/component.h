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
#define COMPONENT_CHILDREN_BASE 4
#endif

#ifndef COMPONENT_BASE
#define COMPONENT_BASE 34
#endif

#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>

typedef enum componentType {
    INPUT,
    LED,
    NOT,
    AND,
    OR
} componentType;

typedef struct component component;

struct component {
    size_t x, y;
    componentType type;

    bool active;

    size_t parents_len;
    size_t parents_cap;
    component* parents[9];

    size_t children_len;
    size_t children_cap;
    component** children;
};

#define COMPONENT(x, y, type) (component){x, y, type}

COMPDEF void comp_init(void);
COMPDEF component* add_component(component c);
COMPDEF void delete_component(component* comp);
COMPDEF void comp_free(void);

COMPDEF void component_addChild(component* comp, component* child);
COMPDEF void component_deleteChild(component* comp, component* child);
COMPDEF void component_deleteParent(component* comp, component* parent);

COMPDEF void comp_pressed(size_t x, size_t y, uint8_t moving);
COMPDEF void comp_click(size_t x, size_t y);

COMPDEF void comp_draw(size_t w, size_t h);
#endif

#ifdef COMPONENT_IMPLEMENTATION
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

COMPDEF component* add_component(component c) {
    c.active = false;
    c.children_len = 0;
    c.children_cap = COMPONENT_CHILDREN_BASE;
    c.children = (component**)malloc(sizeof(component*) * c.children_cap);

    c.parents_cap = 9;
    c.parents_len = 0;

    comp_components[comp_components_len] = c;
    comp_components_len++;

    return &comp_components[comp_components_len - 1];
}

COMPDEF void delete_component(component* comp) {
    size_t i, j;

    for (i = 0; i < comp_components_len; i++) {
        if (&comp_components[i] != comp) 
            continue;

        for (j = 0; j < comp->parents_len; j++)
            component_deleteChild(comp->parents[j], comp);

        for (j = 0; j < comp->children_len; j++)
            component_deleteChild(comp, comp->children[j]);

        free(comp_components[i].children);

        for (j = i + 1; j < comp_components_len; j++)
            comp_components[i + (j - i - 1)] = comp_components[j];
        
        comp_components_len--;

        return;
    }
}

COMPDEF void comp_free(void) {
    size_t i;
    for (i = 0; i < comp_components_len; i++)
        free(comp_components[i].children);

    free(comp_components);
}

COMPDEF void component_addChild(component* comp, component* child) {
    size_t i;
    for (i = 0; i < comp->children_len; i++)
        if (child == comp->children[i]) {
            component_deleteChild(comp, child);
            return;
        }

    child->parents[child->parents_len] = comp;
    child->parents_len++;

    comp->children[comp->children_len] = child;
    comp->children_len++;
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
        break;
    }

    if (comp_collide(65, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, INPUT));
        compMoving = true;
        return;
    }

    if (comp_collide(195, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, LED));
        compMoving = true;
        return;
    }

    if (comp_collide(285, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, NOT));
        compMoving = true;
        return;
    }

    if (comp_collide(385, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, AND));
        compMoving = true;
        return;
    }

    if (comp_collide(485, (compH - 40), 45, 45, x, y)) {
        compPressed = add_component(COMPONENT(x, y, OR));
        compMoving = true;
        return;
    }
}

COMPDEF void comp_move(size_t x, size_t y) {
    if (compPressed == NULL)
        return;

    if (compMoving == 0) {
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
    
    if (compClicked == compPressed)
        compPressed->active = !compPressed->active;
    else if (compClicked != NULL) 
        component_addChild(compPressed, compClicked);

    compPressed = NULL;
}

#ifdef RSGL_H
COMPDEF void comp_draw(size_t w, size_t h) {
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
                    RSGL_drawCircle(RSGL_CIRCLE(comp.x + (45 / 4), comp.y + (45 / 4), 45 / 2), RSGL_RGBA(0, 255, 0, 150));
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
            default: 
                break;
        }

        for (i = 0; i < comp.children_len; i++) {
            component* child = comp.children[i];

            if (child->type == NOT)
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
    RSGL_drawCircle(RSGL_CIRCLE(65, (h - 40), 45), RSGL_RGB(0, 0, 100));
    RSGL_drawCircle(RSGL_CIRCLE(65 + 3, (h - 40) + 3, 39), RSGL_RGB(0, 0, 120));
    RSGL_drawText("I N P U T", RSGL_CIRCLE(45, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(195, (h - 40), 45), RSGL_RGB(0, 100, 0));
    RSGL_drawCircle(RSGL_CIRCLE(195 + 3, (h - 40) + 3, 39), RSGL_RGB(0, 120, 0));
    RSGL_drawText("L E D", RSGL_CIRCLE(190, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(285, (h - 40), 45), RSGL_RGB(100, 0, 0));
    RSGL_drawCircle(RSGL_CIRCLE(285 + 3, (h - 40) + 3, 39), RSGL_RGB(120, 0, 0));
    RSGL_drawText("N O T", RSGL_CIRCLE(280, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(385, (h - 40), 45), RSGL_RGB(100, 0, 100));
    RSGL_drawCircle(RSGL_CIRCLE(385 + 3, (h - 40) + 3, 39), RSGL_RGB(120, 0, 120));
    RSGL_drawText("A N D", RSGL_CIRCLE(380, h - 40, 25), RSGL_RGB(120, 120, 120));

    RSGL_drawCircle(RSGL_CIRCLE(480, (h - 40), 45), RSGL_RGB(100, 100, 0));
    RSGL_drawCircle(RSGL_CIRCLE(480 + 3, (h - 40) + 3, 39), RSGL_RGB(145, 145, 0));
    RSGL_drawText("O R", RSGL_CIRCLE(480, h - 40, 25), RSGL_RGB(120, 120, 120));
}
#endif
#endif