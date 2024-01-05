#ifndef TEXTURE_H
#define TEXTURE_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "stb_image.h"

class Texture {

public:
    Texture(const std::string& fileName){
        this->fileName = fileName;
    }

    bool load(GLenum textureUnit){

        glGenTextures(1, &textureObj);
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureObj);

        stbi_set_flip_vertically_on_load(true);
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(fileName.c_str(), &imWidth, &imHeight, &imNrChannels, 0);

        if (data){
            switch (imNrChannels) {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imWidth, imHeight, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case 2:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, imWidth, imHeight, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imWidth, imHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            default:
                std::cout << "Texture image number of channel not implemented" << std::endl;
            }
            
        }else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
            return false;
        }
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);

        //unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }

    void bind(GLenum textureUnit){
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureObj);
    }

private:
    std::string fileName;
    GLuint textureObj;

};
#endif