#ifndef OBJECT_H
#define OBJECT_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
	glm::vec3 Position;
	glm::vec2 Texture;
	glm::vec3 Normal;
};

class Object
{
public:
    std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Vertex> vertices;
	int numVertices;

	GLuint VBO, VAO;

	glm::mat4 model = glm::mat4(1.0);

	Object(const char* path) {

		// Read the file defined by the path argument 
		// open the .obj file into a text editor and see how the data are organized
		// you will see line starting by v, vt, vn and f --> what are these ?
		// Then parse it to extract the data you need
		// keep track of the number of vertices you need       
       std::ifstream file (path);
       if(file.is_open()){
            std::string line;
            while(getline(file, line)){
                std::istringstream iss(line);
                std::string indice;
                iss >> indice;
                //std::cout << "indice : " << indice << std::endl;
                if (indice == "v") {
                    float x, y, z;
                    iss >> x >> y >> z;
                    positions.push_back(glm::vec3(x, y, z));

                }else if (indice == "vn") {
                    float x, y, z;
                    iss >> x >> y >> z;
                    normals.push_back(glm::vec3(x, y, z));

                }else if (indice == "vt") {
                    float u, v;
                    iss >> u >> v;
                    textures.push_back(glm::vec2(u, v));

                }else if (indice == "f") {
                    std::string faces = line.substr(line.find(" "));

                    for(int i =0; i < 3; i++){
                        std::string p, t, n;
                        //for face 1
                        Vertex ver;

                        p = faces.substr(0, faces.find("/"));
                        faces.erase(0, faces.find("/") + 1);

                        t = faces.substr(0, faces.find("/"));
                        faces.erase(0, faces.find("/") + 1);

                        if(i == 2){
                            n = faces.substr(0, faces.find("\n"));
                        }else{
                            n = faces.substr(0, faces.find(" "));
                            faces.erase(0, faces.find(" ") + 1);
                        }

                        ver.Position = positions.at(std::stof(p) - 1);
                        ver.Normal = normals.at(std::stof(n) - 1);
                        ver.Texture = textures.at(std::stof(t) - 1);
                        vertices.push_back(ver);
                    }
                    
                }
            }
            std::cout << "Load model with " << vertices.size() << " vertices" << std::endl;
            file.close();
            numVertices = vertices.size();

       }else std::cout << "Unable to open file";
	}



	void makeObject(Shader shader, bool texture = true) {
		/* This is a working but not perfect solution, you can improve it if you need/want
		* What happens if you call this function twice on an Model ?
		* What happens when a shader doesn't have a position, tex_coord or normal attribute ?
		*/

		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW);

		auto att_pos = glGetAttribLocation(shader.ID, "position");
        if(att_pos != -1){//used in shader
            glEnableVertexAttribArray(att_pos);
            glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);
        }
		
		if (texture) {
			auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
            if(att_tex != -1){//used in shader
                glEnableVertexAttribArray(att_tex);
                glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            }
		}
		
		auto att_col = glGetAttribLocation(shader.ID, "normal");
        if(att_col != -1){//used in shader
            glEnableVertexAttribArray(att_col);
            glVertexAttribPointer(att_col, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        }
		//deactivate the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data;

	}

	void draw() {

		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

	
	}
};
#endif