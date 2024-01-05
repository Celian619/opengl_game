#version 420 core
out vec4 FragColor;
precision mediump float;
in float visibility;
in vec3 v_frag_coord; 
in vec2 v_text_coord; 
in vec3 v_normal; 
in vec3 v_tangent;

struct Light{ 
	vec3 light_pos; 
	float ambient_strength; 
	float diffuse_strength; 
	float specular_strength; 
			//attenuation factors
	float constant;
	float linear;
	float quadratic;
};

layout (binding = 0) uniform sampler2D ourTexture;
layout (binding = 1) uniform sampler2D ourSpecularMap;
layout (binding = 2) uniform sampler2D ourNormalMap;

uniform bool hasTexture = false;
uniform bool hasSpecularMap = false;
uniform bool hasNormalMap = false;

uniform vec3 u_view_pos; 
uniform Light light;

const vec4 fogColor = vec4(0.28f, 0.19f, 0.12f, 1.0f);
const vec4 lavaColor = vec4(0.7f, 0.1f, 0.1f, 1.0f);
const float lavaTop = 10.0f;
const float lavaLightHight = 20.0f;

float specularCalculation(vec3 N, vec3 L, vec3 V ){ 
	vec3 R = reflect (-L,N); //reflect (-L,N) is  equivalent to //max (2 * dot(N,L) * N - L , 0.0) ;
	float cosTheta = dot(R , V); 
	float strength = light.specular_strength;

	float specularExponent = 32.0f;
	if(hasSpecularMap){
 		specularExponent = texture(ourSpecularMap, v_text_coord).r *255.0f;
	}else{//reduce specular strength for other objects
		strength = strength/2.0f;
	}

	float spec = pow(max(cosTheta,0.0), specularExponent); 
	return strength * spec;
}

float getLavaFogFactor(){

    if(v_frag_coord.y < lavaTop){//pixel inside the lava
		return exp(-(pow((lavaTop -v_frag_coord.y)*0.5,0.6)));
	}
	return 1.0f;
}

float getLavaAmbientLight(){
	float delta = v_frag_coord.y - lavaTop;

    if(delta < lavaLightHight){
		float attenuation = 1 / (1 + 0.08f * delta + 0.1f * delta * delta);
		return 0.7f * attenuation;
	}
	return 0.0f;
}

void main() { 
	vec3 N = normalize(v_normal);
	if(hasNormalMap){
		vec3 T = normalize(v_tangent);
		T = normalize(T - dot(T,N) * N);
		vec3 B = cross(T,N);
		vec3 bumpMapNormal = texture(ourNormalMap, v_text_coord).xyz;
		bumpMapNormal = 2.0*bumpMapNormal - vec3(1.0, 1.0, 1.0);
		mat3 TBN = mat3(T, B, N);
		vec3 newNormal = TBN* bumpMapNormal;
		N = normalize(newNormal);
	}
	vec3 L = normalize(light.light_pos - v_frag_coord); 
	vec3 V = normalize(u_view_pos - v_frag_coord); 
	float specular = specularCalculation( N, L, V); 
	float diffuse = light.diffuse_strength * max(dot(N,L),0.0);
	float distance = length(light.light_pos - v_frag_coord);
	float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

	vec4 materialColor = vec4(0.5f,0.5f,0.5f,1.0f);//default to grey
	if(hasTexture){
		materialColor = texture(ourTexture, v_text_coord);
	}

	vec3 light = (light.ambient_strength + attenuation * (diffuse + specular)) * vec3(1.0f)  + getLavaAmbientLight() * lavaColor.xyz;
	vec4 finalColor = vec4(materialColor.xyz * light, materialColor.w);
	finalColor = mix(fogColor, finalColor, visibility);
	FragColor = mix(lavaColor, finalColor, getLavaFogFactor());
}