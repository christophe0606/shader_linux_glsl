#version 330 core

out vec4 FragColor;  // Declare an output variable
uniform vec3 uColor;  // our uniform
uniform sampler2D uTex;
uniform vec2 uResolution;

void main()
{
   vec2 p = 2.0*(gl_FragCoord.xy-uResolution/2.0) / uResolution.y;
   ivec2 size = textureSize(uTex, 0);
   float aspect = float(size.y)/float(size.x);

   vec3 rgb = texture(uTex, vec2(-p.x*aspect,-p.y)+vec2(0.5)).rgb;
   float r = length(p);
   if (r<1.0)
   {
         FragColor = vec4(rgb,1.0);
   }
   else
   {
      FragColor = vec4(0,0,0,1.0);
   }
}