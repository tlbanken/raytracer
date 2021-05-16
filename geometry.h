/*
 * geometry.h
 *
 * Travis Banken
 * 4/27/21
 *
 * Header for the geometry utility functions and structs.
 */

#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#include <stdbool.h>

// Vectors and matrices
typedef struct Vec3f {
    float x;
    float y;
    float z;
} Vec3f;
// functions
float dot3f(Vec3f v1, Vec3f v2);
Vec3f cross3f(Vec3f v1, Vec3f v2);
Vec3f normalize(Vec3f v);
Vec3f add3f(Vec3f v1, Vec3f v2);
Vec3f sub3f(Vec3f v1, Vec3f v2);
Vec3f scale3f(Vec3f v, float s);
char* toString3f(Vec3f v);

// ray
typedef struct Ray {
    Vec3f origin;
    Vec3f dir;
} Ray;

// Color
typedef struct Color {
    float r;
    float g;
    float b;
} Color;

typedef struct Camera {
    Vec3f eye;
    Vec3f center;
    Vec3f up;
    float fovy;
} Camera;

// objects
typedef struct Sphere {
    Vec3f center;
    float radius;

    // light properties
    Vec3f ambient;
    Vec3f emission;
    Vec3f diffuse;
    Vec3f specular;
    float shininess;
    float reflectivity;
} Sphere;

// lights
typedef struct Light {
    Vec3f dir;
    Color col;
} Light;

typedef struct IntersectInfo {
    float t;
    Vec3f nhit;
    Vec3f phit;
} IntersectInfo;
// functions
bool Intersect(const Sphere *sphere, const Ray *ray, IntersectInfo *info);

// other helpers
inline float radians(float deg)
{
    return deg * M_PI / 180.0;
}
// functions
bool solveQuadratic(float a, float b, float c, float *t0, float *t1);
void printObject(Sphere o);

#endif

