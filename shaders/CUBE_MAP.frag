#version 330 core
out vec4 FragColor;
precision mediump float; 
		//Get the cube map
uniform samplerCube cubemapSampler; 
in vec3 texCoord_v; 
uniform vec3 light_pos; 
uniform float timeOfDay;

const vec4 lavaColor = vec4(0.7f, 0.1f, 0.1f, 1.0f);

//sky color between day and night
vec3 neutralColor = vec3(0.08f, 0.0f,0.06f);

void main() { 
    if(texCoord_v.y < 0){//ground
        FragColor = lavaColor;
    }else{
        if(timeOfDay >= 0.0f){
            vec3 L = normalize(light_pos);
            vec3 D = normalize(texCoord_v);
            float angle = dot(L , D);
            angle = max(angle,0);
            float spec = pow(angle, 32); 
            vec4 color = texture(cubemapSampler, texCoord_v);
            vec3 color_sun = spec * (vec3(1,1,1) - color.xyz) + color.xyz  * min(timeOfDay*2, 1); 
            color_sun = mix(color_sun.xyz, neutralColor, 1-timeOfDay); 
            FragColor = vec4(color_sun.xyz,color.w); 
        }else{
            vec4 color = texture(cubemapSampler, texCoord_v);
            vec3 color_night = mix(color.xyz, neutralColor, 1+timeOfDay); 
            FragColor = vec4(color_night.xyz,color.w); 
        }
    }
}