#version 440

layout(location=0) in vec3 vertColor;
layout(location=0) out vec4 outColor;

void main(){
    outColor = vec4(vertColor,1);
}