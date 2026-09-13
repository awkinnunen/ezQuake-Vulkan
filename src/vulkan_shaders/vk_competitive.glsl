// OpenAI Codex, 2026-09-13. GPL-2.0-or-later. std140 mirrors cv_params_t.
#ifndef CV_TEST_PARAMS
layout(std140,set=CV_SET,binding=0) uniform CompetitiveVisuals {
 vec4 world, floorTint, wallTint, light, levels, shape, rim, rimColor, post, post2;
 mat4 modelView;
} cv;
#endif

vec3 cvLighting(vec3 original, bool model) {
 if(cv.light.x<0.5||cv.light.y<=0.0||(model?cv.light.w:cv.light.z)<0.5) return original;
 float luma=dot(original,vec3(.2126,.7152,.0722));
 float x=clamp(luma,0.0,1.0), t=x;
 if(cv.light.x>1.5) {
  float bands=max(2.0,round(cv.levels.w))-1.0;
  float q=x*bands;
  t=(floor(q)+smoothstep(.5-.5*cv.shape.x,.5+.5*cv.shape.x,fract(q)))/bands;
 }
 float low=min(cv.levels.x,cv.levels.y), high=max(cv.levels.y,cv.levels.z);
 float value=t<.5?mix(low,cv.levels.y,smoothstep(0.0,.5,t)):mix(cv.levels.y,high,smoothstep(.5,1.0,t));
 vec3 styled=(luma>.0001?original/luma:vec3(1.0))*value;
 styled*=vec3(1.0+cv.shape.y*(2.0*x-1.0),1.0,1.0-cv.shape.y*(2.0*x-1.0));
 return mix(original,styled,cv.light.y);
}

#ifdef CV_WORLD_FRAGMENT
vec3 cvMaterial(sampler2D tex,vec2 uv,vec3 original,bool floorSurface) {
 vec3 colour=original;
 if(cv.world.x<.999||abs(cv.world.z-1.0)>.001) {
  float lod=max(0.0,textureQueryLod(tex,uv).x);
  float distanceBias=cv.wallTint.w*smoothstep(0.0,4.0,lod);
  vec3 broad=textureLod(tex,uv,lod+cv.world.y+distanceBias).rgb;
  colour=broad+(original-broad)*cv.world.x*cv.world.z;
 }
 float lum=dot(colour,vec3(.2126,.7152,.0722));
 colour=mix(vec3(lum),colour,cv.world.w);
 vec3 tint=floorSurface?cv.floorTint.rgb:cv.wallTint.rgb;
 return max(vec3(0.0),mix(colour,colour*tint,cv.floorTint.w));
}
#endif

// Continuous surface rim: no displaced shell geometry or animated texture.
vec3 cvRim(vec3 normal, vec3 position, float up) {
 if(cv.rim.x<=0.0 || length(normal)<=.001 || length(position)<=.001) return vec3(0);
 float facing=abs(dot(normalize(normal),normalize(-position)));
 float amount=pow(1.0-clamp(facing,0.0,1.0),cv.rim.y);
 amount*=mix(1.0,smoothstep(-.4,.8,up),cv.rim.z);
 return cv.rimColor.rgb*cv.rim.x*amount;
}
