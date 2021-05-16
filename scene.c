/*
 * scene.c
 *
 * Travis Banken
 * 5/1/2021
 *
 * Functions for generating a scene.
 */

#include "scene.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

struct SceneGlobals g = {
    /* .img_h = 400, */
    /* .img_w = 600, */
    .img_h = 1080,
    .img_w = 1920,
    .max_depth = 2,
    .camera = {
        .eye    = {.x = 0, .y = 0, .z = 5},
        .center = {.x = 0, .y = 0, .z = 0},
        .up     = {.x = 0, .y = 1, .z = 0},
        .fovy   = 60.0
    },
    .objects = {
        {
            .center = {.x = 0, .y = 0, .z = 0}, 
            .radius = 1,
            .ambient  = {0.0, 0.0, 0.0},
            .emission = {0.0, 0.0, 0.0},
            .diffuse  = {0.9, 0.9, 0.9},
            .specular = {1.0, 1.0, 1.0},
            .shininess = 100.0,
            .reflectivity = 0.80
        },
        {
            .center = {.x = 1, .y = 1, .z = 1}, 
            .radius = .5,
            .ambient  = {0.0, 0.0, 0.0},
            .emission = {0.0, 0.0, 0.0},
            .diffuse  = {0.5, 0.5, 0.5},
            .specular = {1.0, 1.0, 1.0},
            .shininess = 100.0,
            .reflectivity = 0.00
        },
        {
            .center = {.x = -1, .y = 0, .z = 2}, 
            .radius = .3,
            .ambient  = {0.6, 0.2, 0.2},
            .emission = {0.0, 0.0, 0.0},
            .diffuse  = {0.5, 0.5, 0.5},
            .specular = {0.3, 0.3, 0.3},
            .shininess = 5.0,
            .reflectivity = 0.0
         },
    },
    .num_objs = 2,
    .lights = {
        {.dir = {.x = -1, .y = -1, .z = -5}, .col = {.r = 0.5, .g = 0.5, .b = 0}},
        /* {.dir = {.x = 0, .y = 0, .z = -2}, .col = {.r = 0.0, .g = 0.5, .b = 0.5}} */
    },
    .num_lights = 1,
};

static Vec3f genVec3f(Vec3f lo, Vec3f hi, bool is_signed)
{
    // x
    int i_hi = hi.x * 1000.0;
    float x = (float) (rand() % i_hi) / 1000.0;
    x = fmax(x, lo.x);
    x = is_signed && rand() % 2 ? -x : x;

    // y
    i_hi = hi.y * 1000.0;
    float y = (float) (rand() % i_hi) / 1000.0;
    y = fmax(y, lo.y);
    y = is_signed && rand() % 2 ? -y : y;

    // z
    i_hi = hi.y * 1000.0;
    float z = (float) (rand() % i_hi) / 1000.0;
    z = fmax(z, lo.z);
    z = is_signed && rand() % 2 ? -z : z;

    Vec3f res = {x, y, z};
    return res;
}

void GenScene(bool random_objs, bool random_lights)
{
    // seed the random number generator
    time_t t;
    srand((unsigned) time(&t));

    if (random_objs) {
        int num_objs = 1 + (rand() % MAX_OBJS);
        for (int i = 0; i < num_objs; i++) {
            Sphere s;
            Vec3f lo;
            Vec3f hi;

            // generate the center
            lo.x = 0; lo.y = 0; lo.z = 0;
            hi.x = 4; hi.y = 4; hi.z = 2;
            s.center = genVec3f(lo, hi, true);
            // gen radius [0.15, 0.50]
            s.radius = (float)(rand() % 4000) / 4000.0;
            s.radius = fmax(s.radius, 0.15);
            /* fprintf(stderr, "[%d] Center = %s, Radius = %.2f\n", i, toString3f(s.center), s.radius); */

            s.shininess = (rand() % (100*1000)) / (1000.0);
            s.shininess = fmax(s.shininess, 10.0);
            lo.x = 0.0; lo.y = 0.0; lo.z = 0.0;
            hi.x = 0.5; hi.y = 0.5; hi.z = 0.5;
            s.ambient = genVec3f(lo, hi, false);
            s.emission.x = 0;s.emission.y = 0;s.emission.z = 0;
            // diffuse and specular
            lo.x = 0.8; lo.y = 0.8; lo.z = 0.8;
            hi.x = 1.0; hi.y = 1.0; hi.z = 1.0;
            s.diffuse = genVec3f(lo, hi, false);
            lo.x = 0.5; lo.y = 0.5; lo.z = 0.5;
            hi.x = 1.0; hi.y = 1.0; hi.z = 1.0;
            s.specular = genVec3f(lo, hi, false);
            // 1/6 chance of being reflective
            if (rand() % 6 == 0) {
                s.reflectivity = (rand() % 1000) / (1000.0);
                s.shininess = 100;
            } else {
                s.reflectivity = 0;
            }

            /* fprintf(stderr, "[%d]\n", i); */
            /* printObject(s); */
            g.objects[i] = s;
        }
        g.num_objs = num_objs;
    }

    if (random_lights) {
        int num_lights = (rand() % MAX_LIGHTS) + 1;
        for (int i = 0; i < num_lights; i++) {
            Vec3f lo, hi;
            Light l;

            lo.x = 0; lo.y = 0; lo.z = 0;
            hi.x = 5.0; hi.y = 5.0; hi.z = 5.0;
            l.dir = genVec3f(lo, hi, true);

            lo.x = 0.0; lo.y = 0.0; lo.z = 0.0;
            hi.x = 0.7; hi.y = 0.7; hi.z = 0.7;
            Vec3f c = genVec3f(lo, hi, false);
            l.col.r = c.x; l.col.g = c.y; l.col.b = c.z;
            g.lights[i] = l;
        }
        g.num_lights = num_lights;
    }
}

