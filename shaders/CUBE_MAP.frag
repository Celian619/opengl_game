#version 330 core
out vec4 FragColor;
precision mediump float; 
		//Get the cube map
uniform samplerCube cubemapSampler; 
in vec3 texCoord_v; 
uniform vec3 light_pos; 
uniform float timeOfDay;
void main() { 

    if(timeOfDay >= 0.0f){
        vec3 L = normalize(light_pos);
        vec3 D = normalize(texCoord_v);
        float angle = dot(L , D);
        angle = max(angle,0);
        float spec = pow(angle, 32); 
        vec4 color = texture(cubemapSampler, texCoord_v);
        vec3 color_sun = spec * (vec3(1,1,1) - color.xyz) + color.xyz  * min(timeOfDay*2, 1); 
        FragColor = vec4(color_sun.xyz,color.w); 
    }else{
        vec4 color = texture(cubemapSampler, texCoord_v);
        vec3 color_night = color.xyz * (-timeOfDay); 
        FragColor = vec4(color_night.xyz,color.w); 
    }
}