/*
 * scene.h
 *
 * Travis Banken
 * 4/27/21
 */

#ifndef _SCENE_H
#define _SCENE_H

#include <stddef.h>

#include "geometry.h"

#define MAX_OBJS 100
#define MAX_LIGHTS 4

struct SceneGlobals {
    size_t img_h;
    size_t img_w;
    int max_depth;
    Camera camera;
    Sphere objects[MAX_OBJS];
    size_t num_objs;
    Light lights[MAX_LIGHTS];
    size_t num_lights;
};
extern struct SceneGlobals g;

// functions
void GenScene(bool random_objs, bool random_lights);



#endif
