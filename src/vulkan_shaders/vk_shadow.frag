#version 450
layout(binding=0) uniform sampler2D material[2];
layout(location=0) in vec2 texCoord;
void main(){if(texture(material[0],texCoord).a<0.5) discard;}
