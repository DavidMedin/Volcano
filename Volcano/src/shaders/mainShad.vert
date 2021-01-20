#version 460

layout(location=0) in vec2 pos;
// layout(location=1) in vec2 offset;
layout(location=1) in vec3 color;


layout(location=0) out vec3 vertColor;

void main(){

    gl_Position = vec4(pos,0,1);
    vertColor = color; 
    // vertColor = vec3(1,0,0);
}