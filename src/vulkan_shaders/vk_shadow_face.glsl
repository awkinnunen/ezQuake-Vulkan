// Same convention for depth rasterization and receiver lookup.
vec3 shadowFace(vec3 d,int face) {
 if(face==0) return vec3(-d.z,-d.y,d.x);
 if(face==1) return vec3(d.z,-d.y,-d.x);
 if(face==2) return vec3(d.x,d.z,d.y);
 if(face==3) return vec3(d.x,-d.z,-d.y);
 if(face==4) return vec3(d.x,-d.y,d.z);
 return vec3(-d.x,-d.y,-d.z);
}
int shadowFaceIndex(vec3 d) {
 vec3 a=abs(d);
 if(a.x>=a.y && a.x>=a.z) return d.x>=0?0:1;
 if(a.y>=a.z) return d.y>=0?2:3;
 return d.z>=0?4:5;
}
float shadowDepth(float z,float farPlane) {return farPlane/(farPlane-1.0)*(1.0-1.0/z);}

// Spot basis and projection shared by rasterization and receiver lookup.
vec3 shadowSpot(vec3 d,vec4 direction) {
 vec3 f=normalize(direction.xyz),up=abs(f.z)>.99?vec3(0,1,0):vec3(0,0,1);
 vec3 right=normalize(cross(f,up));up=cross(right,f);
 float scale=direction.w/sqrt(max(0.0001,1.0-direction.w*direction.w));
 return vec3(dot(d,right)*scale,-dot(d,up)*scale,dot(d,f));
}
