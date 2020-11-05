#version 460

layout(location=0) in vec2 pos;
layout(location=1) in vec3 color;
// vec2 positions[3]={
//     vec2(0.0,-0.5),
//     vec2(0.5,0.5),
//     vec2(-0.5,0.5)
// };
// vec3 colors[3]={
//     vec3(1,0,0),
//     vec3(0,1,0),
//     vec3(0,0,1)
// };

layout(location=0)out vec3 vertColor;

void main(){
    // gl_Position= vec4(positions[gl_VertexIndex],0.0,1.0);
    // vertColor = colors[gl_VertexIndex];
    gl_Position = vec4(pos,0,1);
    vertColor = color; 
}