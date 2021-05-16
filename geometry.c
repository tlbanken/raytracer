/*
 * geometry.c
 *
 * Travis Banken
 * 4/27/21
 *
 * Header for the geometry utility functions and structs.
 */

#include "geometry.h"

#include <stdio.h>

//=========================================
// Vec3f
//=========================================
float dot3f(Vec3f v1, Vec3f v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

Vec3f cross3f(Vec3f v1, Vec3f v2)
{
    Vec3f c = {
        .x = (v1.y * v2.z) - (v1.z * v2.y),
        .y = (v1.z * v2.x) - (v1.x * v2.z),
        .z = (v1.x * v2.y) - (v1.y * v2.x)
    };
    return c;
}

Vec3f normalize(Vec3f v)
{
    float len = sqrt(dot3f(v, v));
    Vec3f n = {.x = v.x / len, .y = v.y / len, .z = v.z / len};
    return n;
}

Vec3f add3f(Vec3f v1, Vec3f v2)
{
    Vec3f sum = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return sum;
}

Vec3f sub3f(Vec3f v1, Vec3f v2)
{
    Vec3f sum = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return sum;
}

Vec3f scale3f(Vec3f v, float s)
{
    Vec3f res = {v.x * s, v.y * s, v.z * s};
    return res;
}

char* toString3f(Vec3f v)
{
    static char buf[64];
    sprintf(buf, "(%.3f, %.3f, %.3f)", v.x, v.y, v.z);
    return buf;
}


//=========================================
// Objects
//=========================================
bool Intersect(const Sphere *sphere, const Ray *ray, IntersectInfo *info)
{
    // setup quad eq for solving
    Vec3f oc = sub3f(ray->origin, sphere->center);
    float a = dot3f(ray->dir, ray->dir);
    float b = 2.0 * dot3f(ray->dir, oc);
    float c = dot3f(oc, oc) - (sphere->radius * sphere->radius);

    // find solutions
    float t0, t1;
    bool found = solveQuadratic(a, b, c, &t0, &t1);
    if (!found) {
        return false;
    }

    // fill out the intersection info
    info->t = fmin(t0, t1);
    // check if object is behind us
    if (info->t < 0.0) info->t = fmax(t0, t1);
    if (info->t < 0.0) return false;

    info->phit = add3f(ray->origin, scale3f(ray->dir, info->t));
    /* info->nhit = sub3f(sphere->center, info->phit); */
    info->nhit = sub3f(info->phit, sphere->center);
    info->nhit = normalize(info->nhit);
    return true;
}

//=========================================
// Helpers
//=========================================
bool solveQuadratic(float a, float b, float c, float *t0, float *t1)
{
    bool real_sol;
    float discr = (b * b) - (4.0 * a * c);
    if (discr < 0.0) {
        real_sol = false;
    } else if (discr == 0.0) {
        // one solution
        *t0 = *t1 = -0.5 * b / a;
        real_sol = true;
    } else {
        // two solutions
        float sqd = sqrt(discr);
        // need to use a rearranged quad form to avoid floating point errs
        float q = (b > 0) ? -0.5 * (b + sqd) : -0.5 * (b - sqd);
        *t0 = q / a;
        *t1 = c / q;
        real_sol = true;
    }
    return real_sol;
}

void printObject(Sphere o)
{
    fprintf(stderr, "--------------------------------------\n");
    fprintf(stderr, "Center  : %s\n", toString3f(o.center));
    fprintf(stderr, "Radius  : %.2f\n", o.radius);
    fprintf(stderr, "Ambient : %s\n", toString3f(o.ambient));
    fprintf(stderr, "Emission: %s\n", toString3f(o.emission));
    fprintf(stderr, "Diffuse : %s\n", toString3f(o.diffuse));
    fprintf(stderr, "Specular: %s\n", toString3f(o.specular));
    fprintf(stderr, "Shininess: %.2f\n", o.shininess);
    fprintf(stderr, "Reflectivity: %.2f\n", o.reflectivity);
    fprintf(stderr, "--------------------------------------\n");
}
