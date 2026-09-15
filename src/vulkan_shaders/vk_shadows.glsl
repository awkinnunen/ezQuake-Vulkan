// SHADOW-001. Only selected lights move out of legacy CPU lighting.
#include "vk_shadow_face.glsl"
layout(set=CV_SET,binding=1) uniform sampler2D shadowAtlas;
layout(set=CV_SET,binding=2,std140) uniform ShadowLights {
 mat4 inverseViewProjection;
 vec4 viewport;
 vec4 settings; // count, face size, world bias, PCF radius
 vec4 positions[8]; // world xyz, radius
 vec4 colors[8]; // rgb, shadow strength (zero = unshadowed budget fallback)
 vec4 camera;
 vec4 directions[8];
 vec4 modes;
} sl;
layout(constant_id=33) const bool shadowFeature=true;
vec3 shadowLighting(vec3 baked) {
 if(!shadowFeature) return baked;
 if(sl.settings.x<0.5) return sl.modes.x>1.5?vec3(sl.modes.y):baked;
 vec4 h=sl.inverseViewProjection*vec4((gl_FragCoord.x-sl.modes.z)/sl.viewport.x*2.0-1.0,
  1.0-(gl_FragCoord.y-sl.modes.w)/sl.viewport.y*2.0,gl_FragCoord.z*2.0-1.0,1.0);
 vec3 p=h.xyz/h.w;
 vec3 n=normalize(cross(dFdx(p),dFdy(p)));
 if(dot(n,p-sl.camera.xyz)>0) n=-n;
 vec3 light=vec3(0);float mapTotal=0,mapVisible=0;
 for(int i=0;i<8;++i) {
  if(i>=int(sl.settings.x)) break;
  vec3 d=p-sl.positions[i].xyz;
  float distanceToLight=length(d),radius=sl.positions[i].w;
  if(distanceToLight>=radius || distanceToLight<1.0) continue;
  float diffuse=max(0.0,dot(n,-d/distanceToLight));
  bool mapLight=sl.directions[i].w!=0.0;
  float cone=1.0;
  if(sl.directions[i].w>0)cone=smoothstep(sl.directions[i].w,mix(sl.directions[i].w,1.0,.2),dot(d/distanceToLight,sl.directions[i].xyz));
  if(cone<=0)continue;
  float visible=1.0;
  if(sl.colors[i].w>0.0) {
   int face=sl.directions[i].w>0?0:shadowFaceIndex(d),tile=i*6+face;
   vec3 q=sl.directions[i].w>0?shadowSpot(d,sl.directions[i]):shadowFace(d,face);
   vec2 base=vec2(tile%6,tile/6)*sl.settings.y;
   vec2 pixel=base+(q.xy/q.z*.5+.5)*sl.settings.y;
   float reference=shadowDepth(max(1.0,q.z-sl.settings.z*(1.0+2.0*(1.0-diffuse))),radius);
   float sum=0;
   for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x) {
    vec2 tap=clamp(pixel+vec2(x,y)*sl.settings.w,base+.5,base+sl.settings.y-.5);
    sum+=step(reference,texture(shadowAtlas,tap*sl.viewport.zw).r);
   }
   visible=mix(1.0,sum/9.0,sl.colors[i].w);
  }
  vec3 energy=sl.colors[i].rgb*(radius-distanceToLight)/128.0*diffuse*cone;
  if(mapLight && sl.modes.x<1.5){float weight=dot(energy,vec3(.2126,.7152,.0722));mapTotal+=weight;mapVisible+=weight*visible;}
  else light+=energy*visible;
 }
 if(sl.modes.x>1.5)return vec3(sl.modes.y)+light;
 return baked*(mapTotal>0?mix(1.0,mapVisible/mapTotal,min(1.0,mapTotal)):1.0)+light;
}

vec3 shadowIrradiance(){return shadowLighting(vec3(0));}

