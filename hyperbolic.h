#ifndef hyperbolic_h
#define hyperbolic_h

struct vec3 {
    double x, y, z;
};

extern void compute_triangle(int p,int q,int r,vec3 &a, vec3 &b, vec3 &c);

#endif