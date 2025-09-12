/*

Copyright 2023 by Christophe Favergeon.
All rights reserved.

Don't share. 
*/
//const float3 np1 = -float3(-0.661297, 0., 0.);
//const float3 np2 = -float3(0.515985, 0.297904, 0.164323);
//const float3 np3 = -float3(0.143463, -0.248485, 0.);

const int NUM_ITERATIONS = 40;
const int transformID = 1;
const float M_PI = 3.141592653589793238462643383;
const int nbPlanes = 3;
const int COLOR_MODULO=2;
const float padding = 30.0;

float hdot(const float3 a,const float3 b)
{
    return(dot(a.xy,b.xy) - a.z * b.z);
}

float3 hnorm(const float3 a)
{
    return(a / (sqrt(-hdot(a,a))));
}

float3 hcross(float3 u,float3 v)
{
    return(float3(v.z * u.y - v.y * u.z,
                  -v.z * u.x + v.x * u.z,
                  -(v.y * u.x - v.x * u.y)));
}

float3 refl(const float3 p,const float3 n)
{
    return(p - 2.0*hdot(p,n)/hdot(n,n)*n);
}

float3 toHyperboloid(const float2 p)
{
    float n = p.x * p.x + p.y * p.y;
    return(float3(2.0*p/(1.0-n),(1.0+n)/(1.0-n)));
}

float2 toPoincare(const float3 p)
{
    return(float2(p.xy/(1 + p.z)));
}

float2 conj(const float2 a)
{
    return(float2(a.x,-a.y));
}

float2 mult(const float2 a,const float2 b)
{
    return(float2(a.x * b.x - a.y * b.y,a.y*b.x + a.x*b.y ));
}

float2 div(const float2 a,const float2 b)
{
    return(mult(a,conj(b))/(b.x*b.x+b.y*b.y));
}

float2 expi(const float angle)
{
    return(float2(cos(angle),sin(angle)));
}

float2 cexp(float2 c)
{
    return(exp(c.x)*expi(c.y));
}

float _sinh(const float x)
{
    return((exp(x)-exp(-x))/2.0);
}

float _cosh(const float x)
{
    return((exp(x)+exp(-x))/2.0);
}

float2 csin(float2 c)
{
    float x,y;
    
    x = _cosh(c.y)*sin(c.x);
    y = cos(c.x)*_sinh(c.y);
    
    return (float2(x,y));
}

float2 mobius(const float2 z,const float2 b,const float angle)
{
    
    float2 r;
    
    r = div((z-b) ,(float2(1,0)-mult(conj(b),z)));
    r = mult(expi(angle),r);
    return(r);
}

float2 thePlane(float2 w)
{
    //const float beta = 0;
    //(1+zi)/(z+1)
    w.y = w.y+1;
    return(div(float2(w.x,w.y-1),float2(w.x,w.y+1)));
}

/*float2 clog(float2 w)
{
    float r = sqrt(w.x * w.x + w.y * w.y);
    float a = atan(w);
    return(float2(log(r),a));
}
*/


float _acosh(float x)
{
    return(log(x+sqrt(x*x-1.0)));
}

float2 strip(float2 w)
{
    //const float beta = 0;
    //(1+zi)/(z+1)
    float2 c = cexp(M_PI/2.0*w);
    float2 n=c-float2(1,0);
    float2 d=c+float2(1,0);

    return(div(n,d));
}

float4 iter(float3 np1,float3 np2,float3 np3,const float2 org,const float2 moebiusTranslation, const float moebiusAngle,int transform)
    {
        int nb=0;
        float2 p=org;
        float2 latest;
        
        if (transform==1)
        {
            p = strip(p);
        }
       
        
        if (length(p)>1)
        {
            return(float4(-1,10000.0,0.0,0.0));
        }
        
        p = mobius(p,moebiusTranslation,radians(moebiusAngle));

        float3 ph = toHyperboloid(p);
        float3 n = float3(0.0);
        
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
        res = _acosh(1.0+pow(hdot(ph,np1),2.0)/hdot(np1,np1));
        
        float d = _acosh(1.0+pow(hdot(ph,np2),2.0)/hdot(np2,np2));
        res = min(res,d);
        
        d = _acosh(1.0+pow(hdot(ph,np3),2.0)/hdot(np3,np3));
        res = min(res,d);
        
        
        
        latest = toPoincare(ph);
        
        
        return(float4(float(nb),res,latest));
    }
    
vec2 getSample(float rs,float rd,float2 p,float2 size,float2 refpos,int m,float scaleA,float angleA,float aspect)
{
    float2 pos = refpos*rd/scaleA/float2(1,aspect)+p;
    float2 tmp;
    float a =radians(angleA);
    tmp=float2(cos(a)*pos.x +sin(a)*pos.y,-sin(a)*pos.x + cos(a)*pos.y);
    pos = tmp +size/2.0;
    if (m==1)
    {
       pos = mod(pos,size);
    }
    return(pos);
}



kernel vec4 myFilter(float3 np1, float3 np2, float3 np3,float2 moebiusTranslation, float moebiusAngle,float2 iResolution,int highRes,__color backgroundColor,float edgeWidth, int hasEdge,__color edgeColor,int dualTile,__color tilea,__color tileb,sampler imagea,sampler imageb,int useTexture,int hasBackground,int repeatTextureA,int repeatTextureB,float2 posa,float2 posb,float scaleA,float scaleB,float angleA,float angleB,float aspectA,float aspectB,int transform) {
    
    __color fragColor = vec4(0,0,0,1);
    float2 exta = samplerSize(imagea);
    float2 extb = samplerSize(imageb);
    
    
    float r = 0.5*min(iResolution.x-padding,iResolution.y-padding);
    float ra = 0.5*min(exta.x,exta.y);
    float rb = 0.5*min(extb.x,extb.y);
    float2 center = iResolution / 2.0;
    float2 refpos = (destCoord()-center)/r;
    
    int nbAA = 1;
    if (highRes==1)
    {
        nbAA = 4;
    }
    float4 total=float4(0.0,0.0,0.0,0.0);

    /*
    if (sqrt(dot(refpos, refpos))<1.0f)
    {
        return (float4(1.0,0.0,0.0,1.0));
    }
    else 
    {
        return (float4(0.0,0.0,0.0,1.0));
    }
    
      */    
    
    for(int m=0;m<nbAA;m++)
    {
        for(int n=0;n<nbAA;n++)
        {
            float2 cpos = float2(0.0,0.0);
            if (nbAA > 1)
            {
                cpos = float2(float(m),float(n));
                cpos = cpos - float2(nbAA)/2.0;
                cpos = cpos / float(nbAA);
                cpos = cpos / r;
                
                
            }
            
            float2 pos = refpos + cpos;
            float2 latest;
            
            float4 d = iter(np1,np2,np3,pos,moebiusTranslation,moebiusAngle,transform);
            latest = d.zw;
            
            
            
            if (d.x < 0.0)
            {
                if (hasBackground==1)
                {
                    total += backgroundColor;
                }
                
            }
                
            else if ((hasEdge==1) && (abs(d.y) < edgeWidth))
            {
                    total += edgeColor;
            }
            else
            { 
                        float4 col;
                        if (dualTile==1)
                        {
                            int tileId = (int(d.x)%COLOR_MODULO);
                            if (tileId==0)
                            {
                                if (useTexture==1)
                                {
                                    float2 samplerPos;
                                                                    samplerPos=getSample(r,ra,posa,exta,latest,repeatTextureA,scaleA,angleA,aspectA);
                                    col = sample(imagea, samplerPos);
                                }
                                else
                                {
                                   col = tilea;
                                }
                            }
                            else 
                            {
                                if (useTexture==1)
                                {
                                   float2 samplerPos;
                                    samplerPos=getSample(r,rb,posb,extb,latest, repeatTextureB,scaleB,angleB,aspectB);
                                    col = sample(imageb, samplerPos);
                                }
                                else
                                {
                                   col = tileb;
                                }
                            }
                        }
                        else
                        {
                            if (useTexture==1)
                            {
                                float2 samplerPos;
samplerPos=getSample(r,ra,posa,exta,latest, repeatTextureA,scaleA,angleA,aspectA);
                                col = sample(imagea, samplerPos);
                            }
                            else
                            {
                                col = tilea;
                            }
                        }
                        
                        total += col;
                    
                    
             }
            
        }
    }
    total /= float(nbAA * nbAA);
    //fragColor = float4(total.xyz,1.0);
    fragColor = total;
    
    
    
    return fragColor;
    
}