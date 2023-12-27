#version 330 core
in vec3 position; 		
		//only P and V are necessary
uniform mat4 V; 
uniform mat4 P; 
out vec3 texCoord_v; 

 void main(){ 
		//What are we going to use to determine what texture to put on the cube map
texCoord_v = position ;
		//remove translation info from view matrix to only keep rotation
mat4 V_rot = mat4(mat3(V)) ;
		//Compute the position of the cube map
vec4 pos = P*V_rot*vec4(position,1.0); 
		// the positions xyz are divided by w after the vertex shader
		// the z component is equal to the depth value
		// we want a z always equal to 1.0 here, so we set z = w!
		// Remember: z=1.0 is the MAXIMUM depth value 
gl_Position = pos.xyww;
 
}