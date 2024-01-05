#version 330 core
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal; 
layout(location = 2) in vec2 textureCoord; 
layout(location = 3) in vec3 tangent; 

out vec3 v_frag_coord; 
out vec2 v_text_coord; 
out vec3 v_normal; 
out vec3 v_tangent;
out float visibility;

uniform mat4 M; 
uniform mat4 itM; 
uniform mat4 V; 
uniform mat4 P; 

const float fogDensity = 0.0012f;
const float gradient = 1.0f;

void main(){ 
    vec4 frag_coord = M*vec4(position, 1.0); 
    v_frag_coord = frag_coord.xyz; 
    v_text_coord = textureCoord;
    v_normal = vec3(itM * vec4(normal, 1.0)); 
    v_tangent = tangent;
    gl_Position = P*V*frag_coord; 

    float distance = gl_Position.z;
    visibility = exp(-pow(distance * fogDensity, gradient));
};