#version 330 core

#define M_PI 3.1415926535897932384626433832795
/*

Copyright 2023 by Christophe Favergeon.
All rights reserved.

https://www.favergeon.info/arts/2023/02/11/hyperbolic.html

*/
const int NUM_ITERATIONS = 40;
const int COLOR_MODULO=2;

const vec2 moebiusTranslation = vec2(0,0);
const float moebiusAngle = 0.0;
const vec4 backgroundColor = vec4(0.0,0.0,0.0,1.0);
const vec4 edgeColor = vec4(1.0,0,0,1.0);
const vec4 tileAColor = vec4(1.0,0.0,0.0,1.0);
const vec4 tileBColor = vec4(0.0,0.0,1.0,1.0);
const float edgeWidth = 0.01;
const int dualTile = 1;
const int animationOn = 1;
const int transform = 0; // 0: none, 1: strip,


const int tileATexture = 1;
const int tileBTexture = 0;

out vec4 FragColor;  // Declare an output variable
uniform float uTime;  // our uniform
uniform sampler2D uTex;
uniform vec2 uResolution;

uniform vec3 uN1;
uniform vec3 uN2;
uniform vec3 uN3;
uniform int uAA; // anti aliasing
uniform float uTextureZoom;

float hdot(const vec3 a,const vec3 b)
{
    return(dot(a.xy,b.xy) - a.z * b.z);
}

vec3 hnorm(const vec3 a)
{
    return(a / (sqrt(-hdot(a,a))));
}

vec3 hcross(vec3 u,vec3 v)
{
    return(vec3(v.z * u.y - v.y * u.z,
                  -v.z * u.x + v.x * u.z,
                  -(v.y * u.x - v.x * u.y)));
}

vec3 refl(const vec3 p,const vec3 n)
{
    return(p - 2.0*hdot(p,n)/hdot(n,n)*n);
}

vec3 toHyperboloid(const vec2 p)
{
    float n = p.x * p.x + p.y * p.y;
    return(vec3(2.0*p/(1.0-n),(1.0+n)/(1.0-n)));
}

vec2 toPoincare(const vec3 p)
{
    return(vec2(p.xy/(1 + p.z)));
}

vec2 conj(const vec2 a)
{
    return(vec2(a.x,-a.y));
}

vec2 mult(const vec2 a,const vec2 b)
{
    return(vec2(a.x * b.x - a.y * b.y,a.y*b.x + a.x*b.y ));
}

vec2 div(const vec2 a,const vec2 b)
{
    return(mult(a,conj(b))/(b.x*b.x+b.y*b.y));
}

vec2 expi(const float angle)
{
    return(vec2(cos(angle),sin(angle)));
}

vec2 cexp(vec2 c)
{
    return(exp(c.x)*expi(c.y));
}

vec2 csin(vec2 c)
{
    float x,y;
    
    x = cosh(c.y)*sin(c.x);
    y = cos(c.x)*sinh(c.y);
    
    return (vec2(x,y));
}

vec2 strip(vec2 w)
{
    //const float beta = 0;
    //(1+zi)/(z+1)
    vec2 c = cexp(M_PI/2.0*w);
    vec2 n=c-vec2(1,0);
    vec2 d=c+vec2(1,0);

    return(div(n,d));
}

vec2 mobius(const vec2 z,const vec2 b,const float angle)
{
    
    vec2 r;
    
    r = div((z-b) ,(vec2(1,0)-mult(conj(b),z)));
    r = mult(expi(angle),r);
    return(r);
}

vec2 thePlane(vec2 w)
{
    //const float beta = 0;
    //(1+zi)/(z+1)
    w.y = w.y+1;
    return(div(vec2(w.x,w.y-1),vec2(w.x,w.y+1)));
}

vec4 iter(vec3 np1,vec3 np2,vec3 np3,
          const vec2 org,
          int transform)
    {
        int nb=0;
        vec2 p=org;
        vec2 latest;
        
        if (transform==1)
        {
            p = strip(p);
        }
       
        
        if (length(p)>1)
        {
            return(vec4(-1,10000.0,0.0,0.0));
        }
        
        float moebiusAngle = uTime * 5;  
        if (moebiusAngle > 360)
            moebiusAngle -= 360;
        float dx = cos(2*M_PI*uTime*0.1) * 0.5;
        if (animationOn==1)
        {
            p = mobius(p,vec2(dx,0),radians(moebiusAngle));
        }

        vec3 ph = toHyperboloid(p);
        vec3 n = vec3(0.0);
        
        for(int i=0;i<NUM_ITERATIONS;i++)
        {
                if (hdot(ph,np1)<0)
                {
                    nb++;
                    ph -= 2*min(0.,hdot(ph,np1))/hdot(np1,np1)*np1;
                    
                }
                
                if (hdot(ph,np2)<0)
                {
                    nb++;
                    ph -= 2*min(0.,hdot(ph,np2))/hdot(np2,np2)*np2;
                    
                }
                
                if (hdot(ph,np3)<0)
                {
                    nb++;
                    ph -= 2*min(0.,hdot(ph,np3))/hdot(np3,np3)*np3;
                    
                }
            
            
        }
        
        float res;
        float scale = 1.0;
        
        

        // Distance from point to each plane of the poly
        // encodee by its normal.
        // Since each plane is going through the origin we don't need a point on those
        // planes to compute the projection.
        // We are computing distance between ph and projection on the plane p
        // Which is given by Cosh d = - <ph,p>
        res = acosh(1.0+pow(hdot(ph,np1),2.0)/hdot(np1,np1));
        
        float d = acosh(1.0+pow(hdot(ph,np2),2.0)/hdot(np2,np2));
        res = min(res,d);
        
        d = acosh(1.0+pow(hdot(ph,np3),2.0)/hdot(np3,np3));
        res = min(res,d);
        
        
        
        latest = toPoincare(ph);
        
        
        return(vec4(float(nb),res,latest));
    }
    
vec4 getTexture(in vec2 refp)
{
   ivec2 size = textureSize(uTex, 0);
   float aspect = float(size.y)/float(size.x);
   float dx = 1.0 / min(float(size.x),float(size.y));
   vec2 texPos = vec2(-refp.x*aspect,-refp.y)+vec2(0.5);
   // texPos by default will map texture at center of screen
   // Let's modify a little so that it is in center of tiles
   texPos = uTextureZoom*4*texPos + vec2(0.6,0.5);

   vec3 rgb = texture(uTex, texPos).rgb;
   return(vec4(rgb,1.0));
}

void main()
{
   vec2 refp = 2.0*(gl_FragCoord.xy-uResolution/2.0) / uResolution.y;
   float r = 0.5*min(uResolution.x,uResolution.y);

   int nbAA = uAA;

   //vec3 rgb = texture(uTex, vec2(-refp.x*aspect,-refp.y)+vec2(0.5)).rgb;
   
   vec4 total=vec4(0.0,0.0,0.0,0.0);
   for(int m=0;m<nbAA;m++)
   {
        for(int n=0;n<nbAA;n++)
        {
            vec2 cpos = vec2(0.0,0.0);
            if (nbAA > 1)
            {
                cpos = vec2(float(m),float(n));
                cpos = cpos - vec2(nbAA)/2.0;
                cpos = cpos / float(nbAA);
                cpos = cpos / r;
            }

            vec2 pos = refp + cpos;
            vec2 latest;
            
            vec4 d = iter(uN1,uN2,uN3,pos,transform);
            latest = d.zw;

            if (d.x < 0.0)
            {
                total += backgroundColor;
            }
            else if ((abs(d.y) <= edgeWidth))
            {
                total += edgeColor;
            }
            else
            { 
               vec4 col;
               if (dualTile==1)
               {
                  int tileId = (int(d.x)%COLOR_MODULO);
                  if (tileId==0)
                  {
                     if (tileATexture==1)
                     {
                         col = getTexture(latest);
                     }
                     else 
                     {
                         col = tileAColor;
                     }

                  }
                  else
                  {
                     if (tileBTexture==1)
                     {
                        col = getTexture(latest);
                     } 
                     else 
                     {
                        col = tileBColor;
                     }
                  }
               }
               else 
               {
                   col = tileAColor;
               }
               total += col;
           }
        }
   }
   
   /*float r = length(p);
   if (r<1.0)
   {
         FragColor = vec4(rgb,1.0);
   }
   else
   {
      FragColor = vec4(0,0,0,1.0);
   }*/

   total /= float(nbAA * nbAA);
   FragColor = vec4(total.xyz,1.0);
}