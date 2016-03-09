#include "Tree.h"
#include <iostream>

using namespace std;

float tree::dh = 0.01;
float tree::dw = 0.005;
unsigned tree::time = 0;
unsigned tree::nleaf = 0;
unsigned tree::nbranch = 0;

tree::tree(obj t, float w, float h, tree *par) {
    season = SPRING;
    type = t;
    prev = par;
    free_slot = slots - 2;
    born_time = time;

    for (int i = 0; i < slots; i++)
        child[i] = NULL;

    if (!par) {
        free_slot += 2;
        place = 0;
        xz_angle = 0;
        xy_angle = 0;
        heigh = h;
        width = w;
        h0 = h;
        w0 = w;

        mod = glm::mat4();
        mod = glm::translate(mod, glm::vec3(0, 0, 0));
        mod = glm::scale(mod, glm::vec3(w, h, w));
            
        return;
    }

    int n = 1 + rand() % par->free_slot;
    int i = 0, k = 0;
    while (k != n)
        if (!par->child[i++])
            k++;
    i--;
    par->child[i] = this;

    float step = 0.7 / slots;
    float p = 0.3 + i * step + (rand() % (int)(step * 1000)) / 10000;

    float xz = rand() % 360;
    float xy;
    if (t == BRANCH)
        xy = 30 + rand() % 30;
    else 
        xy = rand() % 180;
    xy *= (rand() % 2 ? -1 : 1);

    glm::vec4 ang(0, 1, 0, 0);
    ang = prev->mod * ang;
    float absa;
    absa = atan(ang[1] / ang[0]) * 180 / PI;
    if (t == BRANCH && fabs(absa + xy) > 100)
        xy *= -1;

    xy += absa;
        
    place = p;
    xz_angle = xz;
    xy_angle = xy;

    glm::vec4 pp(0.0, p, 0.0, 1.0);
    pp = par->mod * pp;

    mod = glm::mat4();
    mod = glm::translate(mod, glm::vec3(pp));
    mod = glm::rotate(mod, xz, glm::vec3(0.0f, 1.0f, 0.0f));
    mod = glm::rotate(mod, xy, glm::vec3(0.0f, 0.0f, 1.0f));
    
    float c = w / (par->width * (1 - p));
    if (c > 1) {
        h /= c;
        w /= c;
    }
    /*if (w < 1e-3) {
        w *= 10;
    }
    if (h < 1e-3) {
        h *= 10;
    }*/
    
    if (type == LEAF) {
        h = 0.03;
        w = 0.03;
    }
    mod = glm::scale(mod, glm::vec3(w, h, w));
    
    heigh = h;
    width = w;
    h0 = h;
    w0 = w;
}

void tree::grow(bool f) {
    if (type == LEAF) {
        if (time - born_time < 50) {
            heigh += 0.001;//h0 + dh * (time - born_time - 1);
            width += 0.001;//w0 + dw * (time - born_time - 1);
        }

        glm::vec4 pp(0.0, place, 0.0, 1.0);
        pp = prev->mod * pp;

        mod = glm::mat4();
        mod = glm::translate(mod, glm::vec3(pp));
        mod = glm::rotate(mod, xz_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        mod = glm::rotate(mod, xy_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        mod = glm::scale(mod, glm::vec3(width, heigh, width));

        return;
    }
    if (!f) { 
        if (!prev) {
            heigh += pheigh;
            width += pwidth;
        } else {
            heigh += dheigh;
            width += dwidth;
        }

   /* heigh += dheigh;//*= 1.005;//h0 + dh * (time - born_time - 1);
    width += dwidth; //*= 1.005;//w0 + dw * (time - born_time - 1);
    } else {
        heigh *= 1.0025;//h0 + dh * (time - born_time - 1);
        width *= 1.0025;//w0 + dw * (time - born_time - 1);
    }*/

        glm::vec4 pp(0.0, place, 0.0, 1.0);
        if (!prev)
            pp = glm::mat4() * pp;
        else
            pp = prev->mod * pp;

        mod = glm::mat4();
        mod = glm::translate(mod, glm::vec3(pp));
        mod = glm::rotate(mod, xz_angle, glm::vec3(0.0f, 1.0f, 0.0f));
        mod = glm::rotate(mod, xy_angle, glm::vec3(0.0f, 0.0f, 1.0f));
        mod = glm::scale(mod, glm::vec3(width, heigh, width));
    }

    for (int i = 0; i < slots; i++)
        if (child[i])// && child[i]->type == BRANCH) 
            child[i]->grow(f);
    
    if (!((time - born_time) % newtime) && free_slot) {
        obj t = prev ? (rand() % 2 ? LEAF : BRANCH) : BRANCH;
        if (f)
            t = LEAF;

        if (t == BRANCH)
            nbranch++;
        else
            nleaf++;
        
        tree *newchild = new tree(t, gwidth, gheigh, this);
        free_slot--;
    }

    if (!((time - born_time) % newtime) && free_slot) {
        obj t = prev ? (rand() % 2 ? LEAF : BRANCH) : BRANCH;
        if (f)
            t = LEAF;

        if (t == BRANCH)
            nbranch++;
        else
            nleaf++;
        
        tree *newchild = new tree(t, gwidth, gheigh, this);
        free_slot--;
    }
}

void tree::roll(float angle) {
    glm::vec4 pp(0.0, place, 0.0, 1.0);
    if (!prev)
        pp = glm::mat4() * pp;
    else
        pp = prev->mod * pp;

    mod = glm::mat4();
    mod = glm::translate(mod, glm::vec3(pp));
    mod = glm::rotate(mod, xz_angle, glm::vec3(0.0f, 1.0f, 0.0f));

    if (prev)
        mod = glm::rotate(mod, xy_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    else
        mod = glm::rotate(mod, xy_angle + angle, glm::vec3(0.0f, 0.0f, 1.0f));

    mod = glm::scale(mod, glm::vec3(width, heigh, width));

    if (type == BRANCH)
        for (int i = 0; i < slots; i++)
            if (child[i])
                child[i]->roll(angle);
}