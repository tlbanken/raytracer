/*
 * raytracer.c
 *
 * Travis Banken
 * 4/27/21
 *
 * Simple raytracer for spheres.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>


#include "geometry.h"
#include "scene.h"

// declare the inline function here to make compiler happy
float radians(float deg);

static void render(Color *vbuf, size_t buf_size);
static void computePrimRay(float x, float y, const Camera *cam, Ray *ray);
static Color trace(const Ray *ray, int depth);
static void writePPM(Color *vbuf, size_t vbsize, FILE *fout);
static Color computeLight(const Light *light, const Sphere *obj, const Ray *ray,
        const IntersectInfo *info);
static void printHelp();
static void updateProgress(float percentage);

int main(int argc, char **argv)
{
    bool randomize_objs = false;
    bool randomize_lights = false;

    // parse arguments
    int option;
    while ((option = getopt(argc, argv, "olh")) != -1) {
        switch (option) {
        case 'o':
            randomize_objs = true;
            break;
        case 'l':
            randomize_lights = true;
            break;
        case 'h':
            printHelp();
            return 0;
        default:
            printHelp();
            return 1;
        }
    }

    GenScene(randomize_objs, randomize_lights);

    const size_t vbuf_size = g.img_h * g.img_w;

    Color *vbuf = malloc(vbuf_size * sizeof(Color));
    assert(vbuf != NULL);
    render(vbuf, vbuf_size);

    writePPM(vbuf, vbuf_size, stdout);

    return 0;
}

/*
 * For each pixel, trace a ray.
 */
static void render(Color *vbuf, size_t buf_size)
{
    fprintf(stderr, "Rendering scene with %zu Objects and %zu Lights:\n", g.num_objs, g.num_lights);
    for (size_t y = 0; y < g.img_h; y++) {
        // print out the percentage complete in scanlines
        updateProgress((float)y / (float)g.img_h);
        for (size_t x = 0; x < g.img_w; x++) {
            // generate a primitive ray passing through middle of pixel
            Ray prim_ray;
            computePrimRay(x + 0.5, y + 0.5, &g.camera, &prim_ray);

            // trace ray
            Color col = trace(&prim_ray, 0);

            // color pixel
            int index = (y * g.img_w) + x;
            assert((size_t)index < buf_size);
            vbuf[index] = col;
        }
    }
    updateProgress(1.00);
    fprintf(stderr, "\n");
}

/*
 * Compute a ray which passes through point x and y on the screen.
 */
static void computePrimRay(float x, float y, const Camera *cam, Ray *ray)
{
    // first construct coordinate frame
    Vec3f u, v, w;
    w = normalize(sub3f(cam->eye, cam->center));
    u = normalize(cross3f(cam->up, w));
    v = cross3f(w, u);

    // calc fovx from fovy and aspect
    float aspect = (float)g.img_w / (float)g.img_h;
    float fovy = radians(cam->fovy);
    float fovx = 2.0 * atan(tan(fovy / 2.0) * aspect);

    // next calc alpha and beta
    float half_w = (float)g.img_w / 2.0;
    float half_h = (float)g.img_h / 2.0;
    float alpha = tan(fovx / 2.0) * ((x - half_w) / half_w);
    float beta  = tan(fovy / 2.0) * ((half_h - y) / half_h);

    // build ray
    Vec3f alpha_u = scale3f(u, alpha);
    Vec3f beta_v = scale3f(v, beta);
    Vec3f dir = sub3f(add3f(alpha_u, beta_v), w);
    ray->dir = normalize(dir);
    ray->origin = cam->eye;
}

static Color trace(const Ray *ray, int depth)
{
    Color col;
    Sphere *objhit = NULL;
    IntersectInfo info, info_min;
    info_min.t = 99999999.0; // something big, shouldn't matter
    for (size_t i = 0; i < g.num_objs; i++) {
        if (Intersect(&g.objects[i], ray, &info)) {
            if (objhit == NULL || info.t < info_min.t) {
                objhit = &g.objects[i];
                info_min = info;
            }
        }
    }
    // no hit, exit early
    if (objhit == NULL) {
        // return default miss color (gray)
        col.r = .1; col.g = .1; col.b = .1;
        return col;
    }

    // limit the recursive depth
    if (depth > g.max_depth) {
        Color black = {0, 0, 0};
        return black;
    }

    // trace to lights
    const float bias = 0.001; // bias so that we don't intersect with our starting point
    Color sumcol = {0, 0, 0};
    for (size_t i = 0; i < g.num_lights; i++) {
        Ray lray;
        lray.dir = scale3f(g.lights[i].dir, -1.0); // reverse light dir
        lray.dir = normalize(lray.dir);
        // origin with bias = phit + nhit * bias
        lray.origin = add3f(info_min.phit, scale3f(info_min.nhit, bias));
        bool shadow = false;
        // search for shadows
        Color c;
        for (size_t j = 0; j < g.num_objs; j++) {
            if (Intersect(&g.objects[j], &lray, &info)) {
                // in shadow (color black)
                c.r = 0; c.g = 0; c.b = 0;
                shadow = true;
                break;
            }
        }
        // not in shadow
        if (!shadow) {
            // add light to sum
            c = computeLight(&g.lights[i], objhit, ray, &info_min);
        }
        sumcol.r += c.r; sumcol.g += c.g; sumcol.b += c.b;
    }

    // if object is reflective, send out more rays!
    float r = objhit->reflectivity;
    if (r > 0.0) {
        // build reflected ray
        Ray refl_ray;
        Vec3f ndir = normalize(ray->dir);// maybe don't need this
        float n_dot_I = dot3f(info_min.nhit, ndir);
        refl_ray.dir = sub3f(ndir, scale3f(info_min.nhit, 2.0*n_dot_I));
        refl_ray.origin = add3f(info_min.phit, scale3f(info_min.nhit, bias));
        Color c = trace(&refl_ray, depth + 1);
        sumcol.r += r*c.r; sumcol.g += r*c.g; sumcol.b += r*c.b;
    }

    // simple color shading
    Vec3f col3f = add3f(objhit->ambient, objhit->emission);
    col.r = col3f.x + sumcol.r;
    col.g = col3f.y + sumcol.g;
    col.b = col3f.z + sumcol.b;
    return col;
}

static Color computeLight(const Light *light, const Sphere *obj, const Ray *ray,
        const IntersectInfo *info)
{
    // compute halfvector
    Vec3f nldir = scale3f(normalize(light->dir), -1.0);
    /* Vec3f neye = normalize(sub3f(g.camera.eye, info->phit)); */
    /* Vec3f neye = normalize(g.camera.eye); */
    Vec3f nrdir = normalize(ray->dir);
    Vec3f halfvec = normalize(sub3f(nldir, nrdir));

    // compute lambert
    float n_dot_L = dot3f(info->nhit, nldir);
    // component-wise mult
    Vec3f dc = {
        light->col.r * obj->diffuse.x,
        light->col.g * obj->diffuse.y,
        light->col.b * obj->diffuse.z
    };
    Vec3f lambert = scale3f(dc, fmax(n_dot_L, 0.0));

    // compute phong
    float n_dot_H = dot3f(info->nhit, halfvec);
    // component-wise mult
    Vec3f sc = {
        light->col.r * obj->specular.x,
        light->col.g * obj->specular.y,
        light->col.b * obj->specular.z
    };
    Vec3f phong = scale3f(sc, pow(fmax(n_dot_H, 0.0), obj->shininess));

    // return lambert + phong
    Vec3f res3f = add3f(lambert, phong);
    Color res = {res3f.x, res3f.y, res3f.z};
    return res;
}

static void writePPM(Color *vbuf, size_t vbsize, FILE *fout)
{
    assert(fout != NULL);

    // header
    fprintf(fout, "P3\n"); // magic num
    fprintf(fout, "%lu %lu\n", g.img_w, g.img_h); // width height
    fprintf(fout, "255\n"); // max color val

    // colors
    for (size_t i = 0; i < vbsize; i++) {
        if ((i % g.img_w) == 0) {
            fprintf(fout, "\n");
        }
        int r = (int)fmin(vbuf[i].r * 255.0, 255.0);
        int g = (int)fmin(vbuf[i].g * 255.0, 255.0);
        int b = (int)fmin(vbuf[i].b * 255.0, 255.0);
        fprintf(fout, "%d %d %d   ", r, g, b);
    }
    fprintf(fout, "\n");
}

static void printHelp()
{
    fprintf(stderr, "usage: raytracer [OPTIONS]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "DESCRIPTION\n");
    fprintf(stderr, "A simple raytracer for spheres. Modify the struct inside scene.c\n");
    fprintf(stderr, "to edit the scene. Use the below options to randomize a scene.\n");
    fprintf(stderr, "Output is a ppm file writen to stdout.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "OPTIONS\n");
    fprintf(stderr, " -o  --  Randomize the objects within the scene\n");
    fprintf(stderr, " -l  --  Randomize the lighting within the scene\n");
    fprintf(stderr, " -h  --  Print this help message\n");
}

/*
 * Print out the current progress of rendering. This uses the
 * '\b' escape char so that the progress bar will update on
 * only one line.
 */
static void updateProgress(float percentage)
{
    const int bar_length = 40;
    if (percentage > 1.0) {
        percentage = 1.0;
    } else if (percentage < 0.0) {
        percentage = 0.0;
    }

    const char prefix[] = "Progress: ";

    const int del_size = bar_length + 3 + 2 + 4 + (int)strlen(prefix);
    for (int i = 0; i < del_size; i++) {
        fprintf(stderr, "\b");
    }
    fprintf(stderr, "%s[", prefix);
    const int filled = percentage * bar_length;
    for (int i = 0; i < bar_length; i++) {
        if (i < filled) {
            fprintf(stderr, "#");
        } else {
            fprintf(stderr, " ");
        }
    }
    fprintf(stderr, "] %4.2f%%", percentage * 100.0);
}
