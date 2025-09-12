/*

Copyright 2023 by Christophe Favergeon.
All rights reserved.

https://www.favergeon.info/arts/2023/02/11/hyperbolic.html

*/

#include "hyperbolic.h"
#include <cmath>

vec3 hcross(const vec3 &u, const vec3 &v)
{
    return (vec3{v.z * u.y - v.y * u.z,
                 -v.z * u.x + v.x * u.z,
                 -(v.y * u.x - v.x * u.y)});
}

double hdot(const vec3 &u, const vec3 &v)
{
    return (u.x * v.x + u.y * v.y - u.z * v.z);
}

void get_angles(int p, int q, int r, double &alpha, double &beta, double &gamma)
{
    alpha = M_PI / p;
    beta = M_PI / q;
    gamma = M_PI / r;
}

vec3 operator-(const vec3 &v)
{
    return (vec3{-v.x, -v.y, -v.z});
}

void compute_triangle(int p, int q, int r, vec3 &n1, vec3 &n2, vec3 &n3)
{
    double alpha, beta, gamma;
    if (1.0 / p + 1.0 / q + 1.0 / r >= 1.0)
    {
        p = 4;
        q = 4;
        r = 4;
    }
    get_angles(p, q, r, alpha, beta, gamma);

    double a = acosh((cos(gamma) * cos(beta) + cos(alpha))/(sin(gamma) * sin(beta)));
    double b = acosh((cos(gamma) * cos(alpha) + cos(beta))/(sin(gamma) * sin(alpha)));
    double c = acosh((cos(alpha) * cos(beta) + cos(gamma))/(sin(alpha) * sin(beta)));
        
    vec3 p0{0.0,0.0,1.0};
    vec3 p1{0.0,sinh(a),cosh(a)};

    double u=cosh(c) / tanh(a) - cosh(b) / sinh(a);
    double v=cosh(c);
    vec3 p2{sqrt(v*v-u*u-1),u,v};

    n1 = -hcross(p0, p1);
    n2 = -hcross(p1, p2);
    n3 = -hcross(p2, p0);

}
